//
//
//

#include "orbenum.h"

NTSTATUS
CompleteIrp(IN PIRP Irp, IN NTSTATUS status, IN ULONG information)
{
  Irp->IoStatus.Status = status;
  Irp->IoStatus.Information = information;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return status;
}

NTSTATUS
CallNextDriver(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
  IoSkipCurrentIrpStackLocation(Irp);

  return IoCallDriver(devObj, Irp);
}

NTSTATUS
CallComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PKEVENT event)
{
  if (Irp->PendingReturned) 
    {
      IoMarkIrpPending(Irp);
    }

  KeSetEvent(event, FALSE, 0);

  return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CallNextDriverWait(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
  KEVENT event;
  NTSTATUS status;

  // Call next driver
  //DbgOut(("CallNextDriverWait(): enter\n"));
  IoCopyCurrentIrpStackLocationToNext(Irp);
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) CallComplete,
			 &event, TRUE, TRUE, TRUE);
  status = IoCallDriver(devObj, Irp);
  if (STATUS_PENDING == status) 
    {
      KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
  status = Irp->IoStatus.Status;
  //DbgOut(("CallNextDriverWait(): exit %x\n", status));

  return status;
}
