//
// misc.c
//
// Miscelanneous functions
//

#include "hidsporb.h"

// Complete Irp
NTSTATUS
CompleteIrp(IN PIRP Irp, IN NTSTATUS status, IN ULONG information)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

// Call next driver
NTSTATUS
CallNextDriver(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	IoSkipCurrentIrpStackLocation(Irp);

	return IoCallDriver(devObj, Irp);
}

// CallNextDriverWait() completion function
NTSTATUS
CallComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PKEVENT event)
{
	// Propagate pending thing up the stack
	if (Irp->PendingReturned) {
		IoMarkIrpPending(Irp);
	}

	// We're done
	KeSetEvent(event, FALSE, 0);

	// Prevent I/O manager from freeing this Irp
	return STATUS_MORE_PROCESSING_REQUIRED;
}

// Call driver and wait for completion
NTSTATUS
CallNextDriverWait(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	KEVENT event;
	NTSTATUS status;

	// Call next driver
	//DbgOut(("CallNextDriverWait(): enter\n"));
	// Copy our stack location to next
	IoCopyCurrentIrpStackLocationToNext(Irp);
	// Initialize event for completion routine
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Set completion routine
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) CallComplete,
				&event, TRUE, TRUE, TRUE);
	// Call driver
	status = IoCallDriver(devObj, Irp);
	// Wait for completion
	if (STATUS_PENDING == status) {
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
	}
	// Get status from Irp
	status = Irp->IoStatus.Status;
	//DbgOut(("CallNextDriverWait(): exit %x\n", status));

	return status;
}
