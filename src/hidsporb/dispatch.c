//
// dispatch.c
//

#include "hidsporb.h"

// Common dispatch code
// Not used for now
NTSTATUS
OrbDispatch(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status = STATUS_NOT_SUPPORTED;
	PIO_STACK_LOCATION irpSp;
	ULONG major;

	DbgOut(ORB_DBG_DISPATCH, ("OrbDispatch(): enter\n"));
	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	major = irpSp->MajorFunction;
	// Get remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// Fail request if can't get lock
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_DISPATCH, ("OrbDispatch(): no remove lock\n"));

		return CompleteIrp(Irp, status, 0);
	}
	IoSkipCurrentIrpStackLocation(Irp);
	// Call lower driver (serenum)
	status = IoCallDriver(devExt->nextDevObj, Irp);
	DbgOut(ORB_DBG_DISPATCH, ("OrbDispatch(): call %d, status %x\n", major, status));
	// Release remove lock
	IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
	DbgOut(ORB_DBG_DISPATCH, ("OrbDispatch(): exit, status %x\n", status));

	return status;
}
