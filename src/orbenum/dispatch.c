//
// dispatch.c
//

#include "orbenum.h"

// Common dispatch code
NTSTATUS
OrbEnumDispatch(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
  PPDO_EXTENSION pdevExt;
  NTSTATUS status = STATUS_NOT_SUPPORTED;
  PIO_STACK_LOCATION irpSp;
  ULONG major;

  DbgOut( ORB_DBG_DISPATCH, ("OrbEnumDispatch(): enter\n"));
  pdevExt = (PPDO_EXTENSION) devObj->DeviceExtension;
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  major = irpSp->MajorFunction;
  // We forward only requests coming to PDO
  if (pdevExt->Flags) 
    {
      // Get remove lock
      status = IoAcquireRemoveLock(&pdevExt->RemoveLock, Irp);
      // Fail request if can't get lock
      if (!NT_SUCCESS(status)) 
	{
	  DbgOut( ORB_DBG_DISPATCH, ("OrbEnumDispatch(): no remove lock\n"));

	  return CompleteIrp(Irp, status, 0);
	}
#if 0
      // These two are special:
      // since we already opened COM port, IRP_MJ_CREATE will fail
      if ((major == IRP_MJ_CREATE) || (major == IRP_MJ_CLOSE)) 
	{
	  // Emulate CREATE/CLOSE stuff
	  DbgOut( ORB_DBG_DISPATCH, ("OrbEnumDispatch(): CREATE/CLOSE\n"));
	  IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);

	  return CompleteIrp(Irp, STATUS_SUCCESS, 0);
	}
#endif
      IoSkipCurrentIrpStackLocation(Irp);
      // Call lower driver (serenum)
      status = IoCallDriver(pdevExt->nextDevObj, Irp);
      DbgOut( ORB_DBG_DISPATCH, ("OrbEnumDispatch(): PDO call %d, status %x\n", major, status));
      // Release remove lock
      IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
    } else {
      // Complete IRP with STATUS_NOT_SUPPORTED
      CompleteIrp(Irp, status, 0);
    }
  DbgOut( ORB_DBG_DISPATCH, ("OrbEnumDispatch(): exit, status %x\n", status));

  return status;
}
