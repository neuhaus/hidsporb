//
// wmi.c
//
// IRP_MJ_SYSTEM_CONTROL aka WMI functions
//

#include "hidsporb.h"

NTSTATUS 
OrbSystemControl(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;

	PAGED_CODE();

	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	DbgOut(ORB_DBG_ALL, ("OrbDispatchSysControl(): enter"));
	IoSkipCurrentIrpStackLocation(Irp);
	// Call root bus driver
	status = IoCallDriver(devExt->nextDevObj, Irp);
	DbgOut(ORB_DBG_ALL, ("OrbDispatchSysControl(): exit, status %x\n", status));

	return status;
}
