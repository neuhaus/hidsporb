//
// pnp.c
//

#include "hidsporb.h"

// Everything is nonpaged for now
#if 0
#pragma alloc_text (PAGE, OrbAddDevice)
#pragma alloc_text (PAGE, OrbDispatchPnp)
#pragma alloc_text (PAGE, OrbDispatchSystemControl)
#endif

// Note
// AddDevice() for HID minidrivers is a little strange beast
// HID calls us with our FDO already created and attached to PDO
// so beware
NTSTATUS
OrbAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT fdo)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG i;

	PAGED_CODE();

	DbgOut(("OrbAddDevice(): enter\n"));

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
		DbgOut(("OrbAddDevice(): cant alloc Irp\n"));

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Initialize device mapping fields
	for (i = 0; i < ORB_NUM_AXES; i++) {
		devExt->AxisMap[i] = i;
		devExt->sensitivities[i] = 0;
		devExt->polarities[i] = HIDSPORB_POLARITY_POSITIVE;
		devExt->gains[i] = 50;
	}

	devExt->use_chording = FALSE;
	devExt->new_null_region_pending = FALSE;
	devExt->null_region = 0;

	// precision settings
	devExt->precision_sensitivity = 0;
	devExt->precision_gain = 50;
	devExt->precision_button_type = HIDSPORB_BUTTON_TYPE_NONE;
	devExt->precision_button_index = 0;

	// OK, we're ready to go!
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	devExt->nextDevObj->Flags &= ~DO_DEVICE_INITIALIZING;
	DbgOut(("OrbAddDevice: %p to %p->%p \n", fdo, 
			devExt->nextDevObj,
			fdo));
	DbgOut(("OrbAddDevice(): exit\n"));

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
	DbgOut(("OrbPnp(): enter %s\n", PnpToString(func)));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	// Acquire remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// If we can't get lock device is removed
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPnp(): cant get removelock\n"));
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
		Irp->IoStatus.Status = STATUS_SUCCESS;
		status = CallNextDriver(devExt->nextDevObj, Irp);
		break;
	case IRP_MN_QUERY_REMOVE_DEVICE:
		// Query remove is sent when PNP is about to remove our device
		// We simply send it to lower driver
		// Set removed to TRUE until I know what causes PNP stuff to stop
		devExt->Removed = TRUE;
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
	DbgOut(("OrbPnp(): exit %s, status %x\n", PnpToString(func), status));

	return status;
}

// Start device
NTSTATUS
OrbStartDevice(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;

	DbgOut(("OrbStartDevice(): enter\n"));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	// Set up stack for call
	Irp->IoStatus.Status = STATUS_SUCCESS;
	// Call lower driver
	status = CallNextDriverWait(devExt->nextDevObj, Irp);
	if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
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
	DbgOut(("OrbStartDevice(): exit, status %x\n", status));

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

	DbgOut(("OrbRemoveDevice(): enter\n"));
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
	DbgOut(("OrbRemoveDevice(): exit %x\n", status));

	return status;
}
