//
// power.c
//
// IRP_MJ_POWER functions
//

#include "orbenum.h"

NTSTATUS
OrbEnumDispatchPower(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
    
  devExt = (PDEVICE_EXTENSION) devObj->DeviceExtension;

  //
  // If the device has been removed, the driver should not pass 
  // the IRP down to the next lower driver.
  //
  DbgOut(("OrbDispatchPower(): enter"));
    
  if (devExt->Removed) 
    {
      PoStartNextPowerIrp(Irp);
      return CompleteIrp(Irp, STATUS_DELETE_PENDING, 0);
    }

  PoStartNextPowerIrp(Irp);
  IoSkipCurrentIrpStackLocation(Irp);

  // Call root bus driver
  return PoCallDriver(devExt->nextDevObj, Irp);
}
