//
// dispatch.c
//

#include "orbenum.h"

// Completion routine
NTSTATUS
OrbDispatchComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PPDO_EXTENSION pdevExt)
{
	// Mark Irp pending if needed
	if (Irp->PendingReturned) {
		IoMarkIrpPending(Irp);
	}
	// Release remove lock
	IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
	// Complete request
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

// Common dispatch code
NTSTATUS
OrbEnumDispatch(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PPDO_EXTENSION pdevExt;
	NTSTATUS status = STATUS_NOT_SUPPORTED;
	PIO_STACK_LOCATION irpSp;
	UCHAR major;
#if	DBG
	ULONG ioctl;
#endif

	// Get device extension, stack location and stuff
	pdevExt = (PPDO_EXTENSION) devObj->DeviceExtension;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	major = irpSp->MajorFunction;
	// We forward only requests coming to PDO
	if (pdevExt->Flags) {
		DbgOut(ORB_DBG_DISPATCH, ("OrbEnumDispatch(): PDO enter %s\n", DbgMajorToStr(major)));
#if	DBG
		if (major == IRP_MJ_DEVICE_CONTROL) {
			ioctl = irpSp->Parameters.DeviceIoControl.IoControlCode;
			DbgOut(ORB_DBG_DISPATCH, ("OrbEnumDispatch(): ioctl %s\n", DbgSerIoctlToStr(ioctl)));
		}
#endif
		// Get remove lock
		status = IoAcquireRemoveLock(&pdevExt->RemoveLock, Irp);
		// Fail request if can't get lock
		if (!NT_SUCCESS(status)) {
			DbgOut(ORB_DBG_DISPATCH, ("OrbEnumDispatch(): no remove lock\n"));

			return CompleteIrp(Irp, status, 0);
		}
		// Old cruft, to be removed soon
#if OLD_CRUFT
		IoSkipCurrentIrpStackLocation(Irp);
		// Call lower driver (serenum/serial)
		status = IoCallDriver(pdevExt->nextDevObj, Irp);
		DbgOut(ORB_DBG_DISPATCH, ("OrbEnumDispatch(): PDO call %d, status %x\n", major, status));
		// Release remove lock
		// NOTE: should we install completion routine and release lock there?
		IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
#endif
		// New way
		// Set completion routine and release remove lock there
		IoCopyCurrentIrpStackLocationToNext(Irp);
		IoSetCompletionRoutine(Irp, OrbDispatchComplete, pdevExt, TRUE, TRUE, TRUE);
		// Call lower driver (serenum/serial)
		status = IoCallDriver(pdevExt->nextDevObj, Irp);
		DbgOut(ORB_DBG_DISPATCH, ("OrbEnumDispatch(): PDO exit %s, status %x\n", DbgMajorToStr(major), status));
	} else {
		// This request is coming to FDO
		// Complete IRP with STATUS_NOT_SUPPORTED
		CompleteIrp(Irp, status, 0);
	}

	return status;
}
