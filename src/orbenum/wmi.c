//
// wmi.c
//
// IRP_MJ_SYSTEM_CONTROL aka WMI functions
//

#include "orbenum.h"

NTSTATUS 
OrbEnumDispatchSystemControl(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
  NTSTATUS status;

  PAGED_CODE();

  devExt = (PDEVICE_EXTENSION) devObj->DeviceExtension;
  DbgOut(("OrbDispatchSysControl(): enter"));
  IoSkipCurrentIrpStackLocation(Irp);
  // Call root bus driver
  status = IoCallDriver(devExt->nextDevObj, Irp);
  DbgOut(("OrbDispatchSysControl(): exit, status %x\n", status));

  return status;
}
