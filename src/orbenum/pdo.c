//
// pdo.c
// PDO PNP handlers
//

#include "orbenum.h"
#include "stdio.h"

//
// This file contains handlers for PDO objects (devices themself) that
// our BUS creates
//

NTSTATUS
OrbPdoCallComplete(IN PDEVICE_OBJECT fdo, IN PIRP fdoIrp, PVOID nothing)
{
	PIO_STACK_LOCATION irpSp;
	PIRP Irp;

	// Dereference bus FDO
	ObDereferenceObject(fdo);
	// Get current I/O stack location and original Irp
	irpSp = IoGetCurrentIrpStackLocation(fdoIrp);
	Irp = (PIRP) irpSp->Parameters.Others.Argument1;
	DbgOut(ORB_DBG_PDO, ("OrbPdoCallComplete(): status %x\n", fdoIrp->IoStatus.Status));
	// Copy status and information
	Irp->IoStatus.Status = fdoIrp->IoStatus.Status;
	Irp->IoStatus.Information = fdoIrp->IoStatus.Information;
	// Free bus FDO Irp
	IoFreeIrp(fdoIrp);
	// Complete original Irp
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

// We might better be handling all IRPs that are routed to FDO.
NTSTATUS
OrbPdoCallFdo(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PDEVICE_OBJECT fdo;
	PIO_STACK_LOCATION irpSp, fdoSp;
	PIRP fdoIrp;

	DbgOut(ORB_DBG_PDO, ("OrbPdoCallFdo(): enter\n"));
	// Get current I/O stack location and FDO reference
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	fdo = IoGetAttachedDeviceReference(pdevExt->fdo);
	// Allocate Irp for calling bus FDO
	fdoIrp = IoAllocateIrp(fdo->StackSize + 1, FALSE);
	// Fail if we couldn't alloc Irp
	if (fdoIrp == NULL) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoCallFdo(): no IRP\n"));
		// Derefence bus FDO
		ObDereferenceObject(fdo);

		// Complete Irp with error
		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, Irp->IoStatus.Information);
	}
	// Set up Irp stack for calling bus FDO
	fdoSp = IoGetNextIrpStackLocation(fdoIrp);
	fdoSp->DeviceObject = fdo;
	// Save original Irp for later
	fdoSp->Parameters.Others.Argument1 = (PVOID) Irp;
	// Set next IRP stack location
	IoSetNextIrpStackLocation(fdoIrp);
	fdoSp = IoGetNextIrpStackLocation(fdoIrp);
	// Prepare Irp stack for bus FDO call
	RtlCopyMemory(fdoSp, irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine));
	fdoSp->Control = 0;
	// Set completion func
	IoSetCompletionRoutine(fdoIrp, (PIO_COMPLETION_ROUTINE) OrbPdoCallComplete, NULL, TRUE, TRUE, TRUE);
	// Set default error code
	fdoIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	// Pend original Irp
	IoMarkIrpPending(Irp);
	// Call bus FDO
	IoCallDriver(fdo, fdoIrp);
	DbgOut(ORB_DBG_PDO, ("OrbPdoCallFdo(): exit"));

	return STATUS_PENDING;
}

// PDO PNP handler
NTSTATUS
OrbPdoPnp(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	UCHAR func;

	PAGED_CODE();
	// Get current I/O stack location and stuff 
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	func = irpSp->MinorFunction;
	DbgOut(ORB_DBG_PDO, ("OrbPdoPnp(): enter %s\n", PnpToString(func)));
	status = IoAcquireRemoveLock(&pdevExt->RemoveLock, Irp);
	// Fail if we can't get remove lock
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoPnp(): remove lock failed %x\n", status));

		return CompleteIrp(Irp, status, 0);
	}
	// Determine what to do
	switch (func) {
	// These requests always succeed
	case IRP_MN_REMOVE_DEVICE:
	status = OrbPdoRemove(pdevExt, Irp);
	break;
	case IRP_MN_START_DEVICE:
	status = OrbPdoStart(pdevExt, Irp);
	break;
	case IRP_MN_QUERY_REMOVE_DEVICE:
	case IRP_MN_CANCEL_REMOVE_DEVICE:
	case IRP_MN_STOP_DEVICE:
	case IRP_MN_QUERY_STOP_DEVICE:
	case IRP_MN_CANCEL_STOP_DEVICE:
	case IRP_MN_SURPRISE_REMOVAL:
	// We might want to handle these IRPs!
	status = CompleteIrp(Irp, STATUS_SUCCESS, Irp->IoStatus.Information);
	break;
	// PDO must handle these requests
	case IRP_MN_QUERY_DEVICE_RELATIONS:
	status = OrbPdoQueryRelations(pdevExt, Irp);
	break;
	case IRP_MN_QUERY_CAPABILITIES:
	status = OrbPdoQueryCaps(pdevExt, Irp);
	break;
	case IRP_MN_QUERY_PNP_DEVICE_STATE:
	status = OrbPdoQueryPnpState(pdevExt, Irp);
	break;
	case IRP_MN_QUERY_BUS_INFORMATION:
	status = OrbPdoQueryBusInfo(pdevExt, Irp);
	break;
	// This code is disabled!!!
#if 0
	// These always go to orbenum bus FDO
	// NOTE: we might want to handle these too
	case IRP_MN_READ_CONFIG:
	case IRP_MN_WRITE_CONFIG:
	case IRP_MN_EJECT:
	case IRP_MN_SET_LOCK:
	case IRP_MN_DEVICE_USAGE_NOTIFICATION:
	case IRP_MN_SURPRISE_REMOVAL+1:
	case IRP_MN_QUERY_RESOURCES:
	case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
	status = OrbPdoCallFdo(pdevExt, Irp);
	break;
#endif
	// PDOs handle QueryId calls
	case IRP_MN_QUERY_ID:
	status = OrbPdoQueryId(pdevExt, Irp);
	break;
	// This is used to get device description text
	case IRP_MN_QUERY_DEVICE_TEXT:
	status = OrbPdoQueryDeviceText(pdevExt, Irp);
	break;
	default:
	// We must fail all unsupported IRPs
	status = CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	// DON'T!
	//status = CallNextDriver(pdevExt->nextDevObj, Irp);
	}
	// In case of IRP_MN_REMOVE_DEVICE, device is already gone,
	// so no device extension!
	if (func != IRP_MN_REMOVE_DEVICE) {
		IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
	}
	DbgOut(ORB_DBG_PDO, ("OrbPdoPnp(): exit %s, status %x\n", PnpToString(func), status));

	return status;
}

// Start handler
NTSTATUS
OrbPdoStart(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status;

	DbgOut(ORB_DBG_PDO, ("OrbPdoStart(): enter\n"));
	status = STATUS_SUCCESS;
	// Set new PNP state
	pdevExt->Started = TRUE;
	pdevExt->Removed = FALSE;
	DbgOut(ORB_DBG_PDO, ("OrbPdoStart(): exit %x\n", status));

	return CompleteIrp(Irp, status, 0);
}

// Remove handler
NTSTATUS
OrbPdoRemove(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status;
	PDEVICE_OBJECT serObj;

	DbgOut(ORB_DBG_PDO, ("OrbPdoRemove(): enter\n"));
	// Perform cleanup only if PDO is really removed
	if (pdevExt->Removed) {
		// Delete PDO
		OrbDeletePdo(pdevExt);
	}
	// Complete Irp
	status = CompleteIrp(Irp, STATUS_SUCCESS, 0);
	DbgOut(ORB_DBG_PDO, ("OrbPdoRemove(): exit, status %x\n", status));

	return status;
}

// This is where we tell PNP about all our devices
//
NTSTATUS
OrbPdoQueryRelations(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PDEVICE_RELATIONS rel, oldrel;
	ULONG count, size, i, relType;

	// Get I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): enter\n"));
	relType = irpSp->Parameters.QueryDeviceRelations.Type;
	// Determine what to do
	switch (relType) {
	// We always handle that
	case TargetDeviceRelation:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): target relations\n"));
	break;
	// NOTE: we might not want to handle this?
	case EjectionRelations:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): ejection relations\n"));
	// Complete request if device isn't removed
	if (!pdevExt->Removed) {
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	break;
	// Docs say it's optional, but PNP goes nuts if we don't handle it
	case RemovalRelations:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): removal relations\n"));
	// Complete request if device isn't removed
	if (!pdevExt->Removed) {
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	break;
	default:
		// Fail all unknown relations
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	// Return PDO for all relations
	oldrel = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
	// Allocate place for DEVICE_RELATIONS
	rel = ExAllocatePoolWithTag(PagedPool, sizeof(DEVICE_RELATIONS), 'ZbrO');
	status = STATUS_INSUFFICIENT_RESOURCES;
	// Fill in DEVICE_RELATIONS if any
	if (rel) {
		rel->Count = 1;
		rel->Objects[0] = pdevExt->devObj;
		ObReferenceObject(pdevExt->devObj);
		status = STATUS_SUCCESS;
		// This isn't supposed to happen!
		// At least docs say that
		if (oldrel) {
			DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): someone plays games with us!\n"));
			ExFreePool(oldrel);
		}
	}
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): exit\n"));

	// Complete Irp
	return CompleteIrp(Irp, status, (ULONG_PTR) rel);
}

// Handle query ID for device
NTSTATUS
OrbPdoQueryId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpSp;
	ULONG idType;
	PWCHAR idstring;
	PWCHAR id;
	ULONG len;
	ULONG size;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryId(): enter\n"));
	// Get current I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	idType = irpSp->Parameters.QueryId.IdType;
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryId(): query %d\n", idType));
	// See what to do
	switch (idType) {
	// We must handle these IDs!
	case BusQueryInstanceID:
		status = OrbPdoQueryInstanceId(pdevExt, Irp);
	break;
	case BusQueryDeviceID:
		status = OrbPdoQueryDeviceId(pdevExt, Irp);
	break;
	case BusQueryHardwareIDs:
		status = OrbPdoQueryHardwareId(pdevExt, Irp);
	break;
	// Fail unsupported idTypes
	case BusQueryCompatibleIDs:
	default:
		status = CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
	}
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryId(): exit %x\n", status));

	return status;
}

NTSTATUS
OrbPdoQueryInstanceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PWCHAR id;
	ULONG len, size;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryId(): enter\n"));
	// Allocate ID buffer
	len = 4;
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	// Fail if no memory
	if (id == NULL) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueInstId(): no memory\n"));
		goto failed;
	}
	// Copy ID to buffer
 	swprintf(id, L"%02d", pdevExt->instanceId);
	id[len+1] = 0;
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryId(): exit\n"));

	return CompleteIrp(Irp, status, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PWCHAR id;
	ULONG len, size;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryDevId(): enter\n"));
	// Get ID length and allocate buffer for it
	len = wcslen(pdevExt->deviceId);
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	// Fail if no memory
	if (id == NULL) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueDevId(): no memory\n"));
		goto failed;
	}
	// Copy ID to buffer
	wcscpy(id, pdevExt->deviceId);
	id[len+1] = 0;
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryDevId(): exit\n"));

	return CompleteIrp(Irp, status, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryHardwareId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PWCHAR id;
	ULONG len, size;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryHardId(): enter\n"));
	// Get hardware ID length and allocate buffer for it
	len = wcslen(pdevExt->hardwareId);
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	// Fail if no memory
	if (id == NULL) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueHardId(): no memory\n"));
		goto failed;
	}
	// Copy ID to buffer
	wcscpy(id, pdevExt->hardwareId);
	id[len+1] = 0;
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryHardId(): exit\n"));

	return CompleteIrp(Irp, status, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceText(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PWCHAR text;
	ULONG len, size;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): enter\n"));
	// Get current I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	// Check if we support that text type
	if (irpSp->Parameters.QueryDeviceText.DeviceTextType != DeviceTextDescription) {
		// Fail it
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	// This isn't supposed to happen, but who knows?
	if ((text = (PWCHAR) Irp->IoStatus.Information) != NULL) {
		// Somebody provided this information? strange
		status = STATUS_SUCCESS;
		goto failed;
	}
	// Get model name length and allocate buffer for it
	len = wcslen(pdevExt->model);
	size = (len + 2) * sizeof(WCHAR);
	text = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	// Fail if no memory
	if (text == NULL) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): no memory\n"));
		goto failed;
	}
	// Copy model name
	swprintf(text, L"%ws", pdevExt->model);
	status = STATUS_SUCCESS;
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): exit, text %ws\n", text));
failed:

	return CompleteIrp(Irp, status, (ULONG_PTR) text);
}

// Query PNP device state handler
NTSTATUS
OrbPdoQueryPnpState(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status;
	PNP_DEVICE_STATE state = 0;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryPnpState(): enter\n"));
	// Check and set PNP state
	if (pdevExt->Removed) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryPnpState(): removed\n"));
		state |= PNP_DEVICE_REMOVED;
	} else
	if (pdevExt->Started == FALSE) {
		DbgOut(ORB_DBG_PDO, ("OrbPdoQueryPnpState(): failed\n"));
		state |= PNP_DEVICE_FAILED;
	}
	// Complete Irp
	status = CompleteIrp(Irp, STATUS_SUCCESS, state);
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryPnpState(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbPdoQueryCaps(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	NTSTATUS status;
	PDEVICE_CAPABILITIES devCaps;
	PIO_STACK_LOCATION irpSp;

	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryCaps(): enter\n"));
	// Get current I/O stack location
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	// Get PNP devcaps pointer
	devCaps = irpSp->Parameters.DeviceCapabilities.Capabilities;
	// Set PNP devcaps
	if (devCaps) {
		// Fill in required fields
		devCaps->LockSupported = 0;
		devCaps->EjectSupported = 0;
		devCaps->Removable = 1;
		devCaps->DockDevice = 0;
		devCaps->UniqueID = 0;
		devCaps->SilentInstall = 0;
		devCaps->RawDeviceOK = 0;
		devCaps->SurpriseRemovalOK = 1;
		devCaps->HardwareDisabled = 0;
		devCaps->WakeFromD0 = 0;
		devCaps->WakeFromD1 = 0;		
		devCaps->WakeFromD2 = 0;
		devCaps->WakeFromD3 = 0;
		devCaps->Address = pdevExt->instanceId;
		devCaps->UINumber = pdevExt->instanceId;
	}
	// Complete Irp
	status = CompleteIrp(Irp, STATUS_SUCCESS, Irp->IoStatus.Information);
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryCaps(): exit, status %x\n", status));

	return status;
}

// Query BUS information handler
NTSTATUS
OrbPdoQueryBusInfo(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PPNP_BUS_INFORMATION busInfo;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	DbgOut(ORB_DBG_PDO, ("OrbQueryBusInfo(): enter\n"));
	// Allocate busInfo structure
	busInfo = ExAllocatePoolWithTag(PagedPool, sizeof(PNP_BUS_INFORMATION), 'BjsO');
	if (busInfo) {
		// Indicate success
		status = STATUS_SUCCESS;
		// Fill in BUS_INFO structure
		busInfo->BusTypeGuid = GUID_ORBENUM_BUS_TYPE;
		busInfo->LegacyBusType = PNPBus;
		busInfo->BusNumber = 0;
	}
	DbgOut(ORB_DBG_PDO, ("OrbQueryBusInfo(): exit, status %x\n", status));
	// Complete Irp
	status = CompleteIrp(Irp, status, (ULONG_PTR) busInfo);

	return status;
}
