//
// detect.c
// ORB detection routines
//

#include "orbenum.h"

ULONG OrbEnumNumDevices = 0;

//
// Some notes on attaching to serial driver stack
// after we have PDOs we must set their StackSize to
// be correct (ie we must include serial devobj stack too!!!)
// failure to do this leads to bugs!
// if we attach by IoAttachDeviceToDeviceStack(), we don't need to do this
//

// dbgdbg
// this is just a temorarily hack
ULONG notagain = 0;

// This function is used to detect if there is Orb present
NTSTATUS
OrbDetect(IN PDEVICE_OBJECT fdo, IN PDEVICE_OBJECT pdo)
{
  PIRP Irp;
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;

  DbgOut( ORB_DBG_DETECT, ("OrbDetect(): enter\n"));
  // Note:
  // we could use IoGetDeviceProperty()
  // to get PNP IDs
  // Allocate Irp
  Irp = IoAllocateIrp(fdo->StackSize, FALSE);
  if (Irp == NULL) 
    {
      DbgOut( ORB_DBG_DETECT, ("OrbDetect(): cant alloc IRP\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto failed;
    }
  IoSetNextIrpStackLocation(Irp);
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  // Set up Query ID call
  irpSp->MajorFunction = IRP_MJ_PNP;
  irpSp->MinorFunction = IRP_MN_QUERY_ID;
  irpSp->Parameters.QueryId.IdType = BusQueryHardwareIDs;
  // Set up completion routine
  status = CallNextDriverWait(fdo, Irp);
  DbgOut( ORB_DBG_DETECT, ("OrbDetect(): returned status %x\n", status));
  if (NT_SUCCESS(status)) 
    {
      ExFreePool((PVOID) Irp->IoStatus.Information);
    }
  IoFreeIrp(Irp);
 failed:
  DbgOut( ORB_DBG_DETECT, ("OrbDetect(): exit %x\n", status));

  return status;
}
