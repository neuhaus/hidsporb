//
// ioctl.h
//

NTSTATUS
OrbInternalIoctl(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbGetDeviceDescriptor(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbGetReportDescriptor(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbReadReport(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbGetDeviceAttributes(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbGetString(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbGetSetFeature(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN BOOLEAN Get);
