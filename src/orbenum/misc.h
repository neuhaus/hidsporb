//
// misc.h
//

// Misc functions
NTSTATUS
CompleteIrp(IN PIRP, IN NTSTATUS, IN ULONG_PTR);

NTSTATUS
CallNextDriver(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
CallComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PKEVENT event);

NTSTATUS
CallNextDriverWait(IN PDEVICE_OBJECT devObj, IN PIRP Irp);
