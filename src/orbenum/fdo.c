//
// FDO functions
//

#include "orbenum.h"

//
// This file contains handlers for our bus FDO
//

// Handle start/stop device like functional driver does
NTSTATUS
OrbFdoPnp(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	UCHAR func;

	PAGED_CODE();
	// Get current I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	func = irpSp->MinorFunction;
	DbgOut(ORB_DBG_FDO, ("OrbFdoPnp(): enter %s\n", PnpToString(func)));
	// Try to acquire remove lock
	// it will fail if device is being removed
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// Fail if we don't get remove lock
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_FDO, ("OrbFdoPnp(): remove lock failed %x\n", status));

		return CompleteIrp(Irp, status, 0);
	}
	// Determine what to do
	switch (func) {
	case IRP_MN_QUERY_DEVICE_RELATIONS:
	// Device relations
	// PNP manager sends this IRP to enumerate our bus
	// We return list of active PDOs
		status = OrbFdoQueryRelations(devExt, Irp);
	break;
	case IRP_MN_START_DEVICE:
	// Start ORB minibus FDO
		status = OrbFdoStart(devExt, Irp);
	break;
	case IRP_MN_REMOVE_DEVICE:
	// Remove ORB minibus FDO
		status = OrbFdoRemove(devExt, Irp);
	break;
	case IRP_MN_SURPRISE_REMOVAL:
	// I doubt that will ever happen, but we must handle it anyway
		Irp->IoStatus.Status = STATUS_SUCCESS;
		status = CallNextDriver(devExt->nextDevObj, Irp);
	break;
	case IRP_MN_QUERY_REMOVE_DEVICE:
	// Query remove is sent when PNP is about to remove our device
	// We simply send it to lower bus driver
		Irp->IoStatus.Status = STATUS_SUCCESS;
		status = CallNextDriver(devExt->nextDevObj, Irp);
	break;
	case IRP_MN_QUERY_CAPABILITIES:
	// We don't handle it
		status = CallNextDriverWait(devExt->nextDevObj, Irp);
		CompleteIrp(Irp, status, Irp->IoStatus.Information);
	break;
	// Handle this!
	case IRP_MN_QUERY_PNP_DEVICE_STATE:
		status = OrbFdoQueryPnpState(devExt, Irp);
	break;
	default:
	// Call lower bus driver for all other cases
		status = CallNextDriver(devExt->nextDevObj, Irp);
	}
	// In case of IRP_MN_REMOVE_DEVICE, device is already gone,
	// so there is no device extension!!!
	if (func != IRP_MN_REMOVE_DEVICE) {
		IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
	}
	DbgOut(ORB_DBG_FDO, ("OrbFdoPnp(): exit %s, status %x\n", PnpToString(func), status));

	return status;
}

// Start FDO device
// Nothing special happens here
NTSTATUS
OrbFdoStart(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	KEVENT event;
	NTSTATUS status;

	DbgOut(ORB_DBG_FDO, ("OrbFdoStart(): enter\n"));
	// Call root bus driver
	Irp->IoStatus.Status = STATUS_SUCCESS;
	status = CallNextDriverWait(devExt->nextDevObj, Irp);
	if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
		// Indicate we're started
		devExt->Started = TRUE;
		devExt->Removed = FALSE;
		status = STATUS_SUCCESS;
	}
	DbgOut(ORB_DBG_FDO, ("OrbFdoStart(): exit %x\n", status));

	// We must now complete this Irp
	return CompleteIrp(Irp, status, 0);
}

// Remove ORB minibus FDO device
NTSTATUS
OrbFdoRemove(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	UNICODE_STRING symName;
	ULONG i;
	PPDO_EXTENSION pdevExt;
	NTSTATUS status;
	PDEVICE_OBJECT fdo;

	DbgOut(ORB_DBG_FDO, ("OrbFdoRemove(): enter\n"));
	// Wait for pending I/O to complete
	IoReleaseRemoveLockAndWait(&devExt->RemoveLock, Irp);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoSkipCurrentIrpStackLocation(Irp);
	// Call root bus
	status = IoCallDriver(devExt->nextDevObj, Irp);
	// Delete all child devices
	// Note, I believe we get here after all our PDOs are removed
	// PNP deletes PDOs first
	// Lock
	OrbLockPdos(devExt);
	DbgOut(ORB_DBG_FDO, ("OrbFdoRemove(): deleting %d PDOs\n", devExt->numDevices));
	for (i = 0; i < devExt->numDevices; i++) {
		DbgOut(ORB_DBG_FDO, ("OrbFdoRemove(): deleting %d PDO %p\n", i, devExt->devArray[i]));
		pdevExt = (PPDO_EXTENSION) devExt->devArray[i]->DeviceExtension;
		// Mark PDO as removed and delete it
		OrbMarkPdoAsRemoved(pdevExt);
		OrbDeletePdo(pdevExt);
	}
	// Unregister notification callback
	IoUnregisterPlugPlayNotification(devExt->notifyEntry);
	devExt->notifyEntry = NULL;
	OrbUnlockPdos(devExt);
#if 0
	// Delete symbolic link
	RtlInitUnicodeString(&symName, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&symName);
#endif
	// Detach and delete device object
	IoDetachDevice(devExt->nextDevObj);
	fdo = devExt->devObj;
	IoDeleteDevice(fdo);
	DbgOut(ORB_DBG_FDO, ("OrbFdoRemove(): exit %x\n", status));

	return status;
}

//
// This is where we tell PNP about all our devices
//
NTSTATUS
OrbFdoQueryRelations(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PDEVICE_RELATIONS rel, oldrel;
	ULONG count, size, nCopy, i, relType;

	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryRelations(): enter, old rel %p\n", Irp->IoStatus.Information));
	// Get current I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	relType = irpSp->Parameters.QueryDeviceRelations.Type;
#if DBG
	switch (relType) {
		case TargetDeviceRelation:
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): target relations\n"));
		break;
		case EjectionRelations:
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): ejection relations\n"));
		break;
		case RemovalRelations:
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): removal relations\n"));
		break;
	}
#endif
	if (relType != BusRelations) {
		// Simply pass request to root bus device
		DbgOut(ORB_DBG_FDO, ("OrbFdoQueryRelations(): not BusRelations\n"));
      		return CallNextDriver(devExt->nextDevObj, Irp);
		goto failed;
	}
	// Lock FDO array so detect code won't do bad things behind our back
	OrbLockPdos(devExt);
	oldrel = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
	nCopy = 0;
	size = sizeof(DEVICE_RELATIONS);
	// Expand array
	// TBD: add code to handle this!
	if (oldrel != NULL) {
		OrbUnlockPdos(devExt);
		rel = oldrel;
		status = STATUS_SUCCESS;
		goto failed;
	}
#if 0
	if (oldrel) {
		size += (oldrel->Count + 1) * sizeof(PDEVICE_OBJECT);
		nCopy = size;
	} else {
		count = devExt->numDevices;
		if (count > 1) {
			size += ((count - 1) * sizeof(PDEVICE_OBJECT));
		}
	}
	// Calculate new size
	rel = ExAllocatePoolWithTag(PagedPool, size, 'rBRO');
	if (rel == NULL) {
		if (oldrel) {
			ExFreePool(oldrel);
		}
		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
	}
	if (oldrel == 0) {
		rel->Count = 0;
	} else {
		// Copy oldrel
		RtlCopyMemory(newrel, oldrel, nCopy);
	}
#endif
	count = devExt->numDevices;
	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryRelations(): count %d\n", count));
	// Calculate needed size
	if (count > 1) {
		size += ((count - 1) * sizeof(PDEVICE_OBJECT));
	}
	// Allocate memory for DEVREL stuff
	rel = ExAllocatePoolWithTag(PagedPool, size, 'rBRO');
	// Fail if no memory
	if (rel == NULL) {
		OrbUnlockPdos(devExt);
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto failed;
	}
	rel->Count = count;
	// Copy & reference devices only if there are any
	if (count > 0) {
		// Copy PDO pointers
		RtlCopyMemory(&rel->Objects[0], &devExt->devArray[0], count * sizeof(PDEVICE_OBJECT));
		// Reference all device objects
		for (i = 0; i < count; i++) {
			ObReferenceObject(devExt->devArray[i]);
		}
	}
	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryRelations(): new rel %p\n", rel));
	// Unlock FDO
	OrbUnlockPdos(devExt);
	// I'm not sure if we should call lower PDO (provided by root enum???)
	// Complete query request
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryRelations(): status %x\n", status));

	// Complete Irp
	return CompleteIrp(Irp, status, (ULONG_PTR) rel);
	// We shouldn't call any other driver!
	//return CallNextDriver(devExt->nextDevObj, Irp);
}

NTSTATUS
OrbFdoQueryPnpState(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	NTSTATUS status;
	PNP_DEVICE_STATE state = 0;

	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryPnpState(): enter\n"));
	// Set correct PNP state
	if (devExt->Removed) {
		state |= PNP_DEVICE_REMOVED;
	}
	if (!devExt->Started) {
		state |= PNP_DEVICE_FAILED;
	}
	// Complete IRP
	status = CompleteIrp(Irp, STATUS_SUCCESS, state);
	DbgOut(ORB_DBG_FDO, ("OrbFdoQueryPnpState(): exit, status %x\n", status));

	return status;
}
