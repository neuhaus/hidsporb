//
// pnp.c
//

#include "hidsporb.h"

// Everything is nonpaged for now
#if 0
#pragma alloc_text(PAGE, OrbAddDevice)
#endif

// Note
// AddDevice() for HID minidrivers is a little strange beast
// HID calls us with our FDO already created and attached to PDO
// so beware
NTSTATUS
OrbAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT fdo)
{
	PDEVICE_EXTENSION devExt;
	PORB_DATA orbData;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG i;

	PAGED_CODE();

	DbgOut(ORB_DBG_PNP, ("OrbAddDevice(): enter\n"));

	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(fdo);

	// Set up serial devObj pointer
	devExt->nextDevObj = GET_PDO_FROM_EXT(fdo);
	// We didn't attach to lower serenum device
	// can't happen!!!
	if (devExt->nextDevObj == NULL) {
		return STATUS_NO_SUCH_DEVICE;
	}

	// Initialize remove lock
	IoInitializeRemoveLock(&devExt->RemoveLock, HIDSPORB_TAG,
				1,
				5);

	fdo->Flags |= (DO_POWER_PAGABLE | DO_BUFFERED_IO);

	// Initialize device extension
	// Pointer to self
	devExt->devObj = fdo;
	devExt->Removed = FALSE;
	devExt->Started = FALSE;

	// Initialize events
	KeInitializeEvent(&devExt->threadTerminated, NotificationEvent, FALSE);
	KeInitializeEvent(&devExt->threadTermEvent, NotificationEvent, FALSE);
	KeClearEvent(&devExt->threadTerminated);
	KeClearEvent(&devExt->threadTermEvent);
	devExt->threadStarted = FALSE;

	// Init queue
	OrbInitReadQueue(devExt);

	// Allocate read Irp used for I/O
	devExt->readIrp = IoAllocateIrp(devExt->nextDevObj->StackSize + 1, FALSE);
	// Fail if we couldn't
	if (devExt->readIrp == NULL) {
		DbgOut(ORB_DBG_PNP, ("OrbAddDevice(): cant alloc Irp\n"));

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Get Orb data
	orbData = GET_ORB_DATA(devExt);
	// Init ORB data & fast mutex
	OrbInitData(orbData);
	// Initialize device mapping fields
	for (i = 0; i < ORB_NUM_AXES; i++) {
		orbData->AxisMap[i] = i;
		orbData->sensitivities[i] = 0;
		orbData->polarities[i] = HIDSPORB_POLARITY_POSITIVE;
		orbData->gains[i] = 50;
	}
	orbData->use_chording = TRUE;
	orbData->new_null_region_pending = FALSE;
	orbData->null_region = 0;
	// precision settings
	orbData->precision_sensitivity = 0;
	orbData->precision_gain = 50;
	orbData->precision_button_type = HIDSPORB_BUTTON_TYPE_NONE;
	orbData->precision_button_index = 0;
	// OK, we're ready to go!
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	devExt->nextDevObj->Flags &= ~DO_DEVICE_INITIALIZING;
	DbgOut(ORB_DBG_PNP, ("OrbAddDevice: %p to %p->%p \n", fdo, 
			devExt->nextDevObj,
			fdo));
	DbgOut(ORB_DBG_PNP, ("OrbAddDevice(): exit\n"));

	return STATUS_SUCCESS;
}

// IRP_MJ_PNP handler
NTSTATUS
OrbPnp(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;
	PIO_STACK_LOCATION irpSp;
	CHAR func;

	PAGED_CODE();

	// Get stack location & function code
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	func = irpSp->MinorFunction;
	DbgOut(ORB_DBG_PNP, ("OrbPnp(): enter %s\n", PnpToString(func)));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	// Acquire remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// If we can't get lock device is removed
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_PNP, ("OrbPnp(): cant get removelock %x\n", status));
		goto failed;
	}
	switch (func) {
	case IRP_MN_START_DEVICE:
		status = OrbStartDevice(devObj, Irp);
		break;
	case IRP_MN_REMOVE_DEVICE:
		status = OrbRemoveDevice(devObj, Irp);
		break;
	case IRP_MN_SURPRISE_REMOVAL:
		// Note, we must handle this for upcoming USB/Serial ORB support
		//devExt->fSurpriseRemoved = TRUE;
		//or devExt->Removed = TRUE;
		Irp->IoStatus.Status = STATUS_SUCCESS;
		status = CallNextDriver(devExt->nextDevObj, Irp);
		break;
	case IRP_MN_QUERY_REMOVE_DEVICE:
		// Query remove is sent when PNP is about to remove our device
		// We simply send it to lower driver
		// Set removed to TRUE until I know what causes PNP stuff to stop
		devExt->Removed = TRUE;
		// Flush queue twice
		OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
		OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
		Irp->IoStatus.Status = STATUS_SUCCESS;
		status = CallNextDriver(devExt->nextDevObj, Irp);
		break;
	default:
		status = CallNextDriver(devExt->nextDevObj, Irp);
	}
	// Release remove lock
	if (func != IRP_MN_REMOVE_DEVICE) {
		IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
	}
failed:
	DbgOut(ORB_DBG_PNP, ("OrbPnp(): exit %s, status %x\n", PnpToString(func), status));

	return status;
}

// Start device
NTSTATUS
OrbStartDevice(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;
	HANDLE keyHandle;

	DbgOut(ORB_DBG_PNP, ("OrbStartDevice(): enter\n"));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	// Get nextDevObj again
	devExt->nextDevObj = GET_PDO_FROM_EXT(devObj);
	// Set up stack for call
	Irp->IoStatus.Status = STATUS_SUCCESS;
	// Call lower driver
	status = CallNextDriverWait(devExt->nextDevObj, Irp);
	if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
		// Open reg key
		status = IoOpenDeviceRegistryKey(devExt->nextDevObj,
						PLUGPLAY_REGKEY_DEVICE,
						STANDARD_RIGHTS_READ,
						&keyHandle);
		// Fail if we couldn't open registry
		if (!NT_SUCCESS(status)) {
			DbgOut(ORB_DBG_PNP, ("OrbStartDevice(): failed to open registry, status %x\n", status));
		} else {
			// Restore ORB settings from registry
			OrbGetSettingsFromRegistry(devExt, keyHandle);
			// Don't forget to close key handle
			ZwClose(keyHandle);
		}
		// Start I/O
		status = OrbStartIo(devExt);
		if (NT_SUCCESS(status)) {
			//
			// As we are successfully now back from our start device
			// we can do work.
			//
			devExt->Started = TRUE;
			devExt->Removed = FALSE;
		}
	}
	DbgOut(ORB_DBG_PNP, ("OrbStartDevice(): exit, status %x\n", status));

	//
	// We must now complete the IRP, since we stopped it in the
	// completion routine with STATUS_MORE_PROCESSING_REQUIRED.
	//

	return CompleteIrp(Irp, status, 0);
}

// Remove ORB device
NTSTATUS
OrbRemoveDevice(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;

	DbgOut(ORB_DBG_PNP, ("OrbRemoveDevice(): enter\n"));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	// Bug fix for IRP_MN_*_REMOVE stuff
	devExt->Removed = TRUE;
	// Terminate thread
	OrbStopIo(devExt);
	// Wait for pending I/O to complete
	IoReleaseRemoveLockAndWait(&devExt->RemoveLock, Irp);
	// Free Irp
	IoFreeIrp(devExt->readIrp);
	devExt->readIrp = NULL;
	// Send to lower driver
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoSkipCurrentIrpStackLocation(Irp);
	// Call lower driver
	status = IoCallDriver(devExt->nextDevObj, Irp);
	DbgOut(ORB_DBG_PNP, ("OrbRemoveDevice(): exit %x\n", status));

	return status;
}
