//
// PDO handles device specific (functional device objects)
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

	ObDereferenceObject(fdo);
	irpSp = IoGetCurrentIrpStackLocation(fdoIrp);
	Irp = (PIRP) irpSp->Parameters.Others.Argument1;
	DbgOut(("OrbPdoCallComplete(): status %x\n", fdoIrp->IoStatus.Status));
	Irp->IoStatus.Status = fdoIrp->IoStatus.Status;
	Irp->IoStatus.Information = fdoIrp->IoStatus.Information;
	IoFreeIrp(fdoIrp);
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
OrbPdoCallFdo(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PDEVICE_OBJECT fdo;
	PPDO_EXTENSION pdevExt;
	PIO_STACK_LOCATION irpSp, fdoSp;
	PIRP fdoIrp;

	DbgOut(("OrbPdoCallFdo(): enter\n"));
	pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	fdo = pdevExt->fdo;
	fdo = IoGetAttachedDeviceReference(fdo);
	fdoIrp = IoAllocateIrp(fdo->StackSize + 1, FALSE);
	if (fdoIrp == NULL) {
		DbgOut(("OrbPdoCallFdo(): no IRP\n"));

		ObDereferenceObject(fdo);
		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, Irp->IoStatus.Information);
	}
	fdoSp = IoGetNextIrpStackLocation(fdoIrp);
	fdoSp->DeviceObject = fdo;
	fdoSp->Parameters.Others.Argument1 = (PVOID) Irp;
	// Set next IRP stack location
	IoSetNextIrpStackLocation(fdoIrp);
	fdoSp = IoGetNextIrpStackLocation(fdoIrp);
	RtlCopyMemory(fdoSp, irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine));
	fdoSp->Control = 0;
	IoSetCompletionRoutine(fdoIrp, (PIO_COMPLETION_ROUTINE) OrbPdoCallComplete, NULL, TRUE, TRUE, TRUE);
	fdoIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoMarkIrpPending(Irp);
	IoCallDriver(fdo, fdoIrp);
	DbgOut(("OrbPdoCallFdo(): exit"));

	return STATUS_PENDING;
}

// Handle start/stop device like functional driver does
NTSTATUS
OrbPdoPnp(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PPDO_EXTENSION pdevExt;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	UCHAR func;
	PAGED_CODE();

	pdevExt = (PDEVICE_EXTENSION) pdo->DeviceExtension;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	func = irpSp->MinorFunction;
	DbgOut(("OrbPdoPnp(): enter %s\n", PnpToString(func)));
	status = IoAcquireRemoveLock(&pdevExt->RemoveLock, Irp);
	// Fail if we don't get remove lock
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPdoPnp(): remove lock failed %x\n", status));
		return CompleteIrp(Irp, status, 0);
	}
	switch (func) {
	// These requests always succeed
	case IRP_MN_REMOVE_DEVICE:
		// dbg dbg special handling
		status = OrbPdoRemove(pdo, Irp);
		break;
	// Note, maybe we should pass these
	// Requests to lower driver?
	case IRP_MN_START_DEVICE:
	case IRP_MN_QUERY_REMOVE_DEVICE:
	case IRP_MN_CANCEL_REMOVE_DEVICE:
	case IRP_MN_STOP_DEVICE:
	case IRP_MN_QUERY_STOP_DEVICE:
	case IRP_MN_CANCEL_STOP_DEVICE:
	case IRP_MN_SURPRISE_REMOVAL:
	case IRP_MN_QUERY_RESOURCES:
	case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
		status = CompleteIrp(Irp, STATUS_SUCCESS, Irp->IoStatus.Information);
		break;
	//
	case IRP_MN_QUERY_DEVICE_RELATIONS:
		status = OrbPdoQueryRelations(pdo, Irp);
		break;
	// These always go to orbenum bus FDO
	case IRP_MN_QUERY_CAPABILITIES:
	case IRP_MN_READ_CONFIG:
	case IRP_MN_WRITE_CONFIG:
	case IRP_MN_EJECT:
	case IRP_MN_SET_LOCK:
	case IRP_MN_QUERY_PNP_DEVICE_STATE:
	case IRP_MN_QUERY_BUS_INFORMATION:
	case IRP_MN_DEVICE_USAGE_NOTIFICATION:
	case IRP_MN_SURPRISE_REMOVAL+1:
		status = OrbPdoCallFdo(pdo, Irp);
		break;
	// PDOs handle QueryId calls
	case IRP_MN_QUERY_ID:
		status = OrbPdoQueryId(pdo, Irp);
		break;
	// This is used to get device description text
	case IRP_MN_QUERY_DEVICE_TEXT:
		status = OrbPdoQueryDeviceText(pdo, Irp);
		break;
	default:
		status = CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
		//status = CallNextDriver(pdevExt->nextDevObj, Irp);
	}
	// In case of IRP_MN_REMOVE_DEVICE, device is already gone,
	// so no device extension!
	if (func != IRP_MN_REMOVE_DEVICE) {
		IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
	}
	DbgOut(("OrbPdoPnp(): exit %s, status %x\n", PnpToString(func), status));

	return status;
}

// Note, we can just simply complete this call
// w/o calling serial
NTSTATUS
OrbPdoStart(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PPDO_EXTENSION pdevExt;
	NTSTATUS status;

	DbgOut(("OrbPdoStart(): enter\n"));
	status = CallNextDriverWait(pdo, Irp);
	if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
		// Start and do whatever we want
		if (NT_SUCCESS(status)) {
			//
			// As we are successfully now back from our start device
			// we can do work.
			//
			pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
			pdevExt->Started = TRUE;
			pdevExt->Removed = FALSE;
		}
	}
	//
	// We must now complete the IRP, since we stopped it in the
	// completion routine with STATUS_MORE_PROCESSING_REQUIRED.
	//
	DbgOut(("OrbPdoStart(): exit %x\n", status));

	return CompleteIrp(Irp, status, 0);
}

NTSTATUS
OrbPdoRemove(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PPDO_EXTENSION pdevExt;
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;
	PIRP Irp1;

	DbgOut(("OrbPdoRemove(): enter\n"));
	//
	pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
	devExt = (PDEVICE_EXTENSION) pdevExt->fdo->DeviceExtension;
	OrbLockPdos(devExt);
	// Remove PDO from array
	devExt->devArray[0] = NULL;
	devExt->numDevices--;
	OrbUnlockPdos(devExt);
	// Wait for pending I/O to complete
	IoReleaseRemoveLockAndWait(&pdevExt->RemoveLock, Irp);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoSkipCurrentIrpStackLocation(Irp);
	// Call serial driver
	status = IoCallDriver(pdevExt->nextDevObj, Irp);
	// Detach and delete device object
#if 0
	Irp1 = IoAllocateIrp(pdevExt->nextDevObj->StackSize + 1, FALSE);
	// Send IRP_MJ_CLOSE to serial driver
	if (Irp1 == NULL) {
		DbgOut(("OrbPdoRemove(): no IRP??? bad!\n"));
	}
	if (Irp1) {
		IoSetNextIrpStackLocation(Irp1);
		irpSp = IoGetNextIrpStackLocation(Irp1);
		irpSp->MajorFunction = IRP_MJ_CLOSE;
		status = CallNextDriverWait(pdevExt->nextDevObj, Irp1);
		DbgOut(("OrbPdoRemove(): sent CLOSE, status %x\n"));
		// Send IRP_MJ_CLEANUP request
		IoSetNextIrpStackLocation(Irp1);
		irpSp = IoGetNextIrpStackLocation(Irp1);
		irpSp->MajorFunction = IRP_MJ_CLEANUP;
		status = CallNextDriverWait(pdevExt->nextDevObj, Irp1);
		DbgOut(("OrbPdoRemove(): sent CLEANUP, status %x\n"));
		IoFreeIrp(Irp1);
	}
#endif
	IoDetachDevice(pdevExt->nextDevObj);
	ObDereferenceObject(pdevExt->file);
	IoDeleteDevice(pdo);
	DbgOut(("OrbPdoRemove(): exit %x\n", status));

	return status;
}

//
// This is where we tell PNP about all our devices
//
NTSTATUS
OrbPdoQueryRelations(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PDEVICE_RELATIONS rel, oldrel;
	ULONG count, size, i;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	devExt = (PDEVICE_EXTENSION) pdo->DeviceExtension;
	DbgOut(("OrbPdoQueryRelations(): enter\n"));
	if (irpSp->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation) {
		// Simply complete request
		DbgOut(("OrbPdoQueryRelations(): not TargDevRel, completing\n"));
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	oldrel = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
	rel = ExAllocatePoolWithTag(PagedPool, sizeof(DEVICE_RELATIONS), 'ZbrO');
	status = STATUS_INSUFFICIENT_RESOURCES;
	if (rel) {
		rel->Count = 1;
		rel->Objects[0] = pdo;
		ObReferenceObject(pdo);
		status = STATUS_SUCCESS;
		if (oldrel)
			ExFreePool(oldrel);
	}
	DbgOut(("OrbPdoQueryRelations(): exit\n"));

	return CompleteIrp(Irp, status, (ULONG_PTR) rel);
}

// Handle query ID for device
NTSTATUS
OrbPdoQueryId(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PPDO_EXTENSION pdevExt;
	NTSTATUS status;
	PIO_STACK_LOCATION irpSp;
	ULONG idType;
	PWCHAR idstring;
	PWCHAR id;
	ULONG len;
	ULONG size;

	DbgOut(("OrbPdoQueryId(): enter\n"));
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	idType = irpSp->Parameters.QueryId.IdType;
	pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
	DbgOut(("OrbPdoQueryId(): query %d\n", idType));

	switch (idType) {
	case BusQueryInstanceID:
	status = OrbPdoQueryInstanceId(pdo, Irp);
		break;
	case BusQueryDeviceID:
	status = OrbPdoQueryDeviceId(pdo, Irp);
		break;
	case BusQueryHardwareIDs:
	status = OrbPdoQueryHardwareId(pdo, Irp);
		break;
	case BusQueryCompatibleIDs:
	default:
	status = CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
		goto ex;
	}
ex:
	DbgOut(("OrbPdoQueryId(): exit %x\n", status));

	return status;
}

NTSTATUS
OrbPdoQueryInstanceId(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PPDO_EXTENSION pdevExt;
	PWCHAR id;
	ULONG len, size;

	DbgOut(("OrbPdoQueryId(): enter\n"));
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
	len = 4;
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	if (id == NULL) {
		DbgOut(("OrbPdoQueInstId(): no memory\n"));

		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
	}
	swprintf(id, L"%02d", pdevExt->numDevice);
	id[len+1] = 0;
	DbgOut(("OrbPdoQueryId(): exit\n"));

	return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceId(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PWCHAR id;
	ULONG len, size;

	DbgOut(("OrbPdoQueryDevId(): enter\n"));
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	len = wcslen(L"ORBENUM\\*FOOBAR");
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	if (id == NULL) {
		DbgOut(("OrbPdoQueDevId(): no memory\n"));

		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
	}
	wcscpy(id, L"ORBENUM\\*FOOBAR");
	id[len+1] = 0;
	DbgOut(("OrbPdoQueryDevId(): exit\n"));

	return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryHardwareId(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PWCHAR id;
	ULONG len, size;

	DbgOut(("OrbPdoQueryHardId(): enter\n"));
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	len = wcslen(L"*FOOBAR");
	size = (len + 2) * sizeof(WCHAR);
	id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	if (id == NULL) {
		DbgOut(("OrbPdoQueHardId(): no memory\n"));

		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
	}
	wcscpy(id, L"*FOOBAR");
	// Add null terminator
	id[len+1] = 0;
	DbgOut(("OrbPdoQueryHardId(): exit\n"));

	return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceText(IN PDEVICE_OBJECT pdo, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PWCHAR text;
	ULONG len, size;

	DbgOut(("OrbPdoQueryDeviceText(): enter\n"));
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	if (irpSp->Parameters.QueryDeviceText.DeviceTextType != DeviceTextDescription) {
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	if ((text = (PWCHAR) Irp->IoStatus.Information) != NULL) {
		// Somebody provided this information? strange
		status = STATUS_SUCCESS;
		goto complete;
	}
#define	DUMMY_VENDOR	L"Foobar electonics "
#define	DUMMY_MODEL	L"Foobar dumb device"
	len = wcslen(DUMMY_MODEL) + 8;
	size = (len + 2) * sizeof(WCHAR);
	text = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
	if (text == NULL) {
		DbgOut(("OrbPdoQueryDeviceText(): no memory\n"));

		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
	}
	swprintf(text, L"%ws", DUMMY_MODEL);
	DbgOut(("OrbPdoQueryDeviceText(): exit, text %ws\n", text));

complete:
	return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) text);
}
