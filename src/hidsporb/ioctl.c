//
// ioctl.c
//

#define	_REPORT_
#include "hidsporb.h"

// This is the heart of HID minidriver
NTSTATUS
OrbInternalIoctl(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	PDEVICE_EXTENSION devExt;
	ULONG ioctl;
	NTSTATUS status = STATUS_NOT_SUPPORTED;

	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	ioctl = irpSp->Parameters.DeviceIoControl.IoControlCode;
	DbgOut(("OrbInternalIoctl(): enter %d\n", ioctl));
	// Get remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// Fail if unsuccessful
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInternalIoctl(): remove lock failed, status %x\n", status));
		status = CompleteIrp(Irp, STATUS_DELETE_PENDING, 0);
		goto failed;
	}
	// Fail if device is removed
	// strange, HID sends us IRP_MN_QUERY_REMOVE
	// and waits until all it's requests are
	// completed with STATUS_DELETE_PENDING
	// After that, HID sends us IRP_MN_REMOVE_DEVICE
	// This is a temporary workaround, I'll fix it soon
	if (devExt->Removed) {
		DbgOut(("OrbInternalIoctl(): remove lock failed, status %x\n", status));
		IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
		status = CompleteIrp(Irp, STATUS_DELETE_PENDING, 0);
		goto failed;
	}

	// Determine which function to call
	switch (ioctl) {
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		status = OrbGetDeviceDescriptor(devObj, Irp);
		break;
	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		status = OrbGetReportDescriptor(devObj, Irp);
		break;
	case IOCTL_HID_READ_REPORT:
		status = OrbReadReport(devObj, Irp);
		break;
	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		status = OrbGetDeviceAttributes(devObj, Irp);
		break;
	case IOCTL_HID_GET_STRING:
		status = OrbGetString(devObj, Irp);
		break;
#if 0
	case IOCTL_HID_GET_FEATURE:
		status = OrbGetFeature(devObj, Irp);
		break;
	case IOCTL_HID_SET_FEATURE:
		status = OrbSetFeature(devObj, Irp);
		break;
#endif
	case IOCTL_GET_PHYSICAL_DESCRIPTOR:
	case IOCTL_HID_ACTIVATE_DEVICE:
	case IOCTL_HID_DEACTIVATE_DEVICE:
	case IOCTL_HID_GET_INDEXED_STRING:
	case IOCTL_HID_WRITE_REPORT:
		status = STATUS_SUCCESS;
	default:
		status = CompleteIrp(Irp, status, 0);
	}
	// Release remove lock
	IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
failed:
	DbgOut(("OrbInternalIoctl(): exit %d, status %x\n", ioctl, status));

	return status;
}

// This function fills HID get device descriptor
NTSTATUS
OrbGetDeviceDescriptor(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PHID_DESCRIPTOR hidDesc;
	ULONG len;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(("OrbGetDeviceDescriptor(): enter\n"));
	// Get buffer & size
	hidDesc = Irp->UserBuffer;
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < sizeof(HID_DESCRIPTOR)) {
		DbgOut(("OrbGetDeviceDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	// Init HID descriptor
	RtlZeroMemory(hidDesc, sizeof(HID_DESCRIPTOR));
	hidDesc->bLength		= sizeof(HID_DESCRIPTOR);
	hidDesc->bDescriptorType	= HID_HID_DESCRIPTOR_TYPE;
	hidDesc->bcdHID			= HID_REVISION;
	// Note, we can make many descriptors
	// to emulate mouse and other kinds of devices
	// We'll hack with these later
	hidDesc->bNumDescriptors	= 1;
	hidDesc->DescriptorList[0].bReportType = HID_REPORT_DESCRIPTOR_TYPE;
	hidDesc->DescriptorList[0].wReportLength = sizeof(SpaceOrbReport);
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, sizeof(HID_DESCRIPTOR));
failed:
	DbgOut(("OrbGetDeviceDescriptor(): exit, status %x\n"));

	return status;
}

// This function fills HID report descriptor
NTSTATUS
OrbGetReportDescriptor(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	ULONG len;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(("OrbGetReportDescriptor(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < sizeof(SpaceOrbReport)) {
		DbgOut(("OrbGetReportDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	// Simply copy our report descriptor
	RtlCopyMemory(Irp->UserBuffer, SpaceOrbReport, sizeof(SpaceOrbReport));
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, sizeof(SpaceOrbReport));
failed:
	DbgOut(("OrbGetReportDescriptor(): exit, status %x\n"));

	return status;
}

// This function places Irp on queue and completes it when ORB sends
// some data
NTSTATUS
OrbReadReport(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	PDEVICE_EXTENSION devExt;
	NTSTATUS status;
	ULONG len;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(("OrbReadReport(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < HIDSPORB_REPORT_SIZE) {
		DbgOut(("OrbGetReportDescriptor(): %d, small buffer\n", len));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	devExt = GET_DEV_EXT(devObj);
	// Queue this request
	status = OrbQueueReadReport(devExt, Irp);
failed:
	DbgOut(("OrbReadReport(): exit, status %x\n"));

	return status;
}

// This function fills HID_GET_DEVICE_ATTRIBUTES
NTSTATUS
OrbGetDeviceAttributes(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	ULONG len;
	PHID_DEVICE_ATTRIBUTES devAttr;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(("OrbGetDeviceAttributes(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < sizeof(HID_DEVICE_ATTRIBUTES)) {
		DbgOut(("OrbGetReportDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	// Set up device attributes
	devAttr = Irp->UserBuffer;
	RtlZeroMemory(devAttr, sizeof(HID_DEVICE_ATTRIBUTES));
	devAttr->Size		= sizeof(HID_DEVICE_ATTRIBUTES);
	devAttr->VendorID	= HIDSPORB_VENDOR_ID;
	devAttr->ProductID	= HIDSPORB_PRODUCT_ID;
	devAttr->VersionNumber	= 1; // HIDSPORB version number
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, sizeof(HID_DEVICE_ATTRIBUTES));
failed:
	DbgOut(("OrbGetDeviceAttributes(): exit, status %x\n"));

	return status;
}

// This function is used to return string
// descriptors
// Note: This doesn't really work now
// We must return USB_STRING_DESCRIPTOR stuff
NTSTATUS
OrbGetString(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	ULONG reqStr, len, len1;
	PCHAR pStr, pStr1;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(("OrbGetString(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	reqStr = (ULONG) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer) & 0xffff;
	// Set up device attributes
	pStr = Irp->UserBuffer;
	switch (reqStr) {
	case HID_STRING_ID_IMANUFACTURER:
		pStr1 = "Two Guys, Inc.";
	break;
	case HID_STRING_ID_IPRODUCT:
		pStr1 = "SpaceOrb 360";
	break;
	case HID_STRING_ID_ISERIALNUMBER:
		pStr1 = "42";
	break;
	default:
		pStr1 = "UNSUPPORTED STRING";
	}
	DbgOut(("OrbGetString(): string is %s\n", pStr1));
	len1 = strlen(pStr1);
	// Fail if the buffer is smaller than needed
	if (len < (len1 + 1)) {
		DbgOut(("OrbGetStringDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, len + 1);
		goto failed;
	}
	// Copy string
	RtlCopyMemory(pStr, pStr1, len1 + 1);
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, len + 1);
failed:
	DbgOut(("OrbGetString(): exit, status %x\n"));

	return status;
}

// Get feature
NTSTATUS
OrbGetFeature(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;
	PIO_STACK_LOCATION irpSp;
}

NTSTATUS
OrbSetFeature(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
}
