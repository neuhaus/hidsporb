//
// ioctl.c
//

#define	_REPORT_
// Define ORB_REPORT_HACKS to include keyboard and mouse reports in SpaceOrbReport
//#define	ORB_REPORT_HACKS
#include "hidsporb.h"

// This is the heart of HID minidriver
NTSTATUS
OrbInternalIoctl(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	PDEVICE_EXTENSION devExt;
	ULONG ioctl;
	NTSTATUS status;

	devExt = (PDEVICE_EXTENSION) GET_DEV_EXT(devObj);
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	ioctl = irpSp->Parameters.DeviceIoControl.IoControlCode;
	DbgOut(ORB_DBG_IOCTL, ("OrbInternalIoctl(): enter %x\n", ioctl));
	// Get remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
	// Fail if unsuccessful
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_IOCTL, ("OrbInternalIoctl(): remove lock failed, status %x\n", status));
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
		DbgOut(ORB_DBG_IOCTL, ("OrbInternalIoctl(): remove lock failed, status %x\n", status));
		IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
		status = CompleteIrp(Irp, STATUS_DELETE_PENDING, 0);
		goto failed;
	}
	status = STATUS_NOT_SUPPORTED;

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
		status = OrbGetString(devObj, Irp, (ULONG) (((ULONG) irpSp->Parameters.DeviceIoControl.Type3InputBuffer) & 0xffff));
		break;
	case IOCTL_HID_GET_FEATURE:
		status = OrbGetSetFeature(devObj, Irp, TRUE);
		break;
	case IOCTL_HID_SET_FEATURE:
		status = OrbGetSetFeature(devObj, Irp, FALSE);
		break;
	case IOCTL_HID_GET_MANUFACTURER_STRING:
	case IOCTL_HID_GET_PRODUCT_STRING:
	case IOCTL_HID_GET_SERIALNUMBER_STRING:
		status = OrbGetString(devObj, Irp, ioctl);
		break;		
	//case IOCTL_GET_PHYSICAL_DESCRIPTOR:
	case IOCTL_HID_ACTIVATE_DEVICE:
	case IOCTL_HID_DEACTIVATE_DEVICE:
	//case IOCTL_HID_GET_INDEXED_STRING:
	//case IOCTL_HID_WRITE_REPORT:
		status = STATUS_SUCCESS;
	default:
		status = CompleteIrp(Irp, status, 0);
	}
	// Release remove lock
	IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
failed:
	DbgOut(ORB_DBG_IOCTL, ("OrbInternalIoctl(): exit %x, status %x\n", ioctl, status));

	return status;
}

// This function fills HID get device descriptor
NTSTATUS
OrbGetDeviceDescriptor(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PHID_DESCRIPTOR hidDesc;
	HID_DESCRIPTOR hidDesc1;
	ULONG len;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(ORB_DBG_IOCTL, ("OrbGetDeviceDescriptor(): enter\n"));
	// Get buffer & size
	hidDesc = Irp->UserBuffer;
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	//if (len < (sizeof(HID_DESCRIPTOR) + sizeof(hidDesc1.DescriptorList[0]))) {
	if (len < sizeof(HID_DESCRIPTOR)) {
		DbgOut(ORB_DBG_IOCTL, ("OrbGetDeviceDescriptor(): small buffer %d\n", len));
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
	// XXX hackery
	hidDesc->bNumDescriptors	= 1;
	hidDesc->DescriptorList[0].bReportType = HID_REPORT_DESCRIPTOR_TYPE;
	hidDesc->DescriptorList[0].wReportLength = sizeof(SpaceOrbReport);
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, sizeof(HID_DESCRIPTOR));
failed:
	DbgOut(ORB_DBG_IOCTL, ("OrbGetDeviceDescriptor(): exit, status %x\n"));

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
	DbgOut(ORB_DBG_IOCTL, ("OrbGetReportDescriptor(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < sizeof(SpaceOrbReport)) {
		DbgOut(ORB_DBG_IOCTL, ("OrbGetReportDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	// Simply copy our report descriptor
	RtlCopyMemory(Irp->UserBuffer, SpaceOrbReport, sizeof(SpaceOrbReport));
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, sizeof(SpaceOrbReport));
failed:
	DbgOut(ORB_DBG_IOCTL, ("OrbGetReportDescriptor(): exit, status %x\n"));

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
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	DbgOut(ORB_DBG_REPORT, ("OrbReadReport(): enter, rep len %d\n", len));
	// Fail if the buffer is smaller than needed
	if (len < HIDSPORB_REPORT_SIZE) {
		DbgOut(ORB_DBG_REPORT, ("OrbReadReport(): %d, small buffer\n", len));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, 0);
		goto failed;
	}
	devExt = GET_DEV_EXT(devObj);
	// Queue this request
	status = OrbQueueReadReport(devExt, Irp);
failed:
	DbgOut(ORB_DBG_REPORT, ("OrbReadReport(): exit, status %x\n"));

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
	DbgOut(ORB_DBG_IOCTL, ("OrbGetDeviceAttributes(): enter\n"));
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	// Fail if the buffer is smaller than needed
	if (len < sizeof(HID_DEVICE_ATTRIBUTES)) {
		DbgOut(ORB_DBG_IOCTL, ("OrbGetReportDescriptor(): small buffer\n"));
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
	DbgOut(ORB_DBG_IOCTL, ("OrbGetDeviceAttributes(): exit, status %x\n"));

	return status;
}

// Note: these are taken from DDK
typedef struct _ORB_STRINGS {
	// Manufacturer
	UCHAR m_len;	// 9*2
	UCHAR m_type;	// 3
	UCHAR m_lang[2];// 9,0
	WCHAR m_str[9]; // "SpaceTec"
	// Product
	UCHAR p_len;	// 13*2
	UCHAR p_type;	// 3
	WCHAR p_str[13];// "SpaceOrb 360"
	// Serial #
	UCHAR s_len;	// 2*2
	UCHAR s_type;	// 3
	WCHAR s_str[2];	// "0"
	// Unsupported string
	UCHAR u_len;	// 19*2
	UCHAR u_type;	// 3
	WCHAR u_str[19];// "UNSUPPORTED STRING"
} ORB_STRINGS;

static ORB_STRINGS orbStrings = {
  18, 3, {9, 0}, { 'S', 'p', 'a', 'c', 'e', 'T', 'e', 'c' },
  26, 3, { 'S', 'p', 'a', 'c', 'e', 'O', 'r', 'b', ' ', '3', '6', '0' },
  4, 3,  { '0' },
  38, 3, { 'U', 'N', 'S', 'U', 'P', 'P', 'O', 'R', 'T', 'E', 'D', ' ', 'S', 'T', 'R', 'I', 'N', 'G' }
};

// This function is used to return string descriptors
NTSTATUS
OrbGetString(IN PDEVICE_OBJECT devObj, IN PIRP Irp, ULONG reqStr)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	ULONG len, len1;
	PWCHAR pStr, pStr1;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	len = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	//reqStr = (ULONG) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer) & 0xffff;
	DbgOut(ORB_DBG_IOCTL, ("OrbGetString(): enter %x\n", reqStr));
	// Set up device attributes
	pStr = Irp->UserBuffer;
	switch (reqStr) {
	case HID_STRING_ID_IMANUFACTURER:
	case IOCTL_HID_GET_MANUFACTURER_STRING:
		pStr1 = &orbStrings.m_str[0];
		len1 = orbStrings.m_len;
	break;
	case HID_STRING_ID_IPRODUCT:
	case IOCTL_HID_GET_PRODUCT_STRING:
		pStr1 = &orbStrings.p_str[0];
		len1 = orbStrings.p_len;
	break;
	case HID_STRING_ID_ISERIALNUMBER:
	case IOCTL_HID_GET_SERIALNUMBER_STRING:
		pStr1 = &orbStrings.s_str[0];
		len1 = orbStrings.s_len;
	break;
	default:
		pStr1 = &orbStrings.u_str[0];
		len1 = orbStrings.u_len;
	}
	DbgOut(ORB_DBG_IOCTL, ("OrbGetString(): string is %ws\n", pStr1));
	// Fail if the buffer is smaller than needed
	if (len < len1) {
		DbgOut(ORB_DBG_IOCTL, ("OrbGetStringDescriptor(): small buffer\n"));
		status = CompleteIrp(Irp, STATUS_BUFFER_TOO_SMALL, len + 1);
		goto failed;
	}
	// Copy string
	RtlCopyMemory(pStr, pStr1, len1);
	// Complete request
	status = CompleteIrp(Irp, STATUS_SUCCESS, len1);
failed:
	DbgOut(ORB_DBG_IOCTL, ("OrbGetString(): exit, status %x\n"));

	return status;
}

//#if DBG

static PCHAR chordStr[] = {
	"no chording",
	"chording"
};

static
VOID
OrbPrintFeatureData(IN PHIDSPORB_FEATURE_DATA featureData)
{
	ULONG i;

	DbgOut(ORB_DBG_FEATURE, ("OrbPrintFeatureData(): begin\n"));
	for (i = 0; i < ORB_NUM_AXES; i++) {
		// Axis map
		DbgOut(ORB_DBG_FEATURE, ("Axis %01d->%01d ", i, featureData->axis_map[i]));
		// sensitivity
		DbgOut(ORB_DBG_FEATURE, ("sens %02d ", featureData->sensitivities[i]));
		// polarities
		DbgOut(ORB_DBG_FEATURE, ("pola %01d ", featureData->polarities[i]));
		// gains
		DbgOut(ORB_DBG_FEATURE, ("gain %02d\n", featureData->gains[i]));
	}
	// Print other stuff
	DbgOut(ORB_DBG_FEATURE, ("chord %01d ", (LONG) featureData->use_chording));
	DbgOut(ORB_DBG_FEATURE, ("nullr %02d ", (LONG) featureData->null_region));
	DbgOut(ORB_DBG_FEATURE, ("psens %02d ", (LONG) featureData->precision_sensitivity));
	DbgOut(ORB_DBG_FEATURE, ("pgain %02d ", (LONG) featureData->precision_gain));
	DbgOut(ORB_DBG_FEATURE, ("pbtnt %01d ", (LONG) featureData->precision_button_type));
	DbgOut(ORB_DBG_FEATURE, ("pbtni %01d\n", (LONG) featureData->precision_button_index));

	DbgOut(ORB_DBG_FEATURE, ("OrbPrintFeatureData(): end\n"));
}
//#else
//#define	OrbPrintFeatureData(_x_)
//#endif

// Note, this is temporary
// make sure it's in sync with HIDSPORB_*_PACKET_ID
static ULONG featureSizes[] = { 0, 0, HIDSPORB_FEATURE_PACKET_SIZE, HIDSPORB_SENSITIVITY_CURVE_PACKET_SIZE };

// Get/set feature
NTSTATUS
OrbGetSetFeature(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN BOOLEAN Get)
{
	PDEVICE_EXTENSION devExt;
	PHID_XFER_PACKET packet;
	UCHAR reportId;
	ULONG len;
	PHIDSPORB_FEATURE_DATA featureData;
	PHIDSPORB_SENSITIVITY_CURVE curveData;
	NTSTATUS status = STATUS_BUFFER_TOO_SMALL;
	PORB_DATA orbData;
	ULONG i;

	// Get device extension
	devExt = GET_DEV_EXT(devObj);
	// Get Orb data
	orbData = GET_ORB_DATA(devExt);
	// Initialize IoStatus
	Irp->IoStatus.Information = 0;
	// Get packet pointer and other stuff
	packet = (PHID_XFER_PACKET) Irp->UserBuffer;
	reportId = packet->reportId;
	// Get pointers & packet length
	featureData = &(((PHIDSPORB_FEATURE_PACKET) (packet->reportBuffer))->feature_data);
	curveData =  &(((PHIDSPORB_SENSITIVITY_CURVE_PACKET) (packet->reportBuffer))->curve);
	len = packet->reportBufferLen;
	DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): packet rep %p repid %d len %d\n", featureData, (ULONG) reportId, len));
	// Check if report ID is correct
	if ((reportId < HIDSPORB_FEATURE_MIN_ID) || ((reportId > HIDSPORB_FEATURE_MAX_ID))) {
		DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): invalid report id %d\n", (ULONG) reportId));
		goto failed;
	}
	// Fail if buffer is too small
	if (len < featureSizes[reportId]) {
		DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): small packet rep %d len %d\n", (ULONG) reportId, len));
		goto failed;
	}
	// How much we copied
	len = featureSizes[reportId];
	// Note, we should think about protecting data from change
	// (via spinlock). E.g. we must first copy featureData to local buffer,
	// then lock all device data (Axes, mapping stuff and so) and only
	// then copy all what needed
	// devExt->Axes[] must be protected by spinlock too.
	// consider what axes we can get when OrbReadThread() is running on
	// another CPU and changing Axes[] behind our back.
	// Determine request
	switch (reportId) {
	case HIDSPORB_FEATURE_PACKET_ID:
	// Get/set axis map, sensitivity and other stuff
	// Copy mapping/sens/pol/gains data to user
	// Lock ORB data from changing
	OrbLockData(orbData);
	if (Get) {
		for (i = 0; i < ORB_NUM_AXES; i++) {
			// Axis map
			featureData->axis_map[i] = (UCHAR) orbData->AxisMap[i];
			// sensitivity
			featureData->sensitivities[i] = (UCHAR) orbData->sensitivities[i];
			// polarities
			featureData->polarities[i] = (UCHAR) orbData->polarities[i];
			// gains
			featureData->gains[i] = (UCHAR) orbData->gains[i];
		}
		// copy other stuff
		featureData->use_chording = (char)(orbData->use_chording);
		featureData->null_region = (char)(orbData->null_region);
		featureData->precision_sensitivity = (char)(orbData->precision_sensitivity);
		featureData->precision_gain = (char)(orbData->precision_gain);
		featureData->precision_button_type = (char)(orbData->precision_button_type);
		featureData->precision_button_index = (char)(orbData->precision_button_index);
		// Print feature data
		OrbPrintFeatureData(featureData);
		DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): Get ORB chording %d\n", (LONG) orbData->use_chording));
	} else {
		// Copy mapping/sens/pol/gains data from user
		// We don't check anything yet
		OrbPrintFeatureData(featureData);
		// Fail if something is wrong with feature
		if (!OrbCheckFeature(featureData)) {
			len = 0;
			status = STATUS_INVALID_PARAMETER;
			// Don't forget to do this!
			OrbUnlockData(orbData);
			break;
		}
		// Now set up everything
		for (i = 0; i < ORB_NUM_AXES; i++) {
			OrbSetAxisMapping(devExt, i, featureData->axis_map[i]);
			OrbSetSensitivity(devExt, i, featureData->sensitivities[i]);
			OrbSetPolarity(devExt, i, featureData->polarities[i]);
			OrbSetGain(devExt, i, featureData->gains[i]);
#if 0
			// Axis map
			orbData->AxisMap[i] = featureData->axis_map[i];
			// sensitivity
			orbData->sensitivities[i] = featureData->sensitivities[i];
			// polarities
			orbData->polarities[i] = featureData->polarities[i];
			// gains
			orbData->gains[i] = featureData->gains[i];
#endif
		}
		OrbSetChording(devExt, featureData->use_chording);
		OrbSetNullRegion(devExt, featureData->null_region);
		OrbSetPrecisionSensitivity(devExt, featureData->precision_sensitivity);
		OrbSetPrecisionGain(devExt, featureData->precision_gain);
		OrbSetPrecisionButton(devExt, featureData->precision_button_type,
					featureData->precision_button_index);
#if 0
		orbData->use_chording = featureData->use_chording;
		orbData->null_region = featureData->null_region;
		orbData->precision_sensitivity = featureData->precision_sensitivity;
		orbData->precision_gain = featureData->precision_gain;
		orbData->precision_button_type = featureData->precision_button_type;
		orbData->precision_button_index = featureData->precision_button_index;
#endif
		DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): Set ORB chording %d\n", (LONG) orbData->use_chording));
	}
	OrbUnlockData(orbData);
	// Indicate success
	Irp->IoStatus.Information = len;
	status = STATUS_SUCCESS;
	break;
	case HIDSPORB_CURVE_PACKET_ID:
	status = STATUS_INVALID_PARAMETER;
	// Get curve
	OrbLockData(orbData);
	if (Get) {
		// Copy curve if Id is valid
		if (OrbIsValidCurveId(curveData->curve_id)) {
			// Copy curve to user
			RtlMoveMemory(curveData->curve, charts[curveData->curve_id], 1024 * sizeof(USHORT));
			// Indicate success
			status = STATUS_SUCCESS;
			Irp->IoStatus.Information = len;
		}
	} else {
	// Set curve
		// Check for validness
		if (OrbIsValidCurveId(curveData->curve_id) && OrbIsValidCurve(curveData->curve)) {
			// Copy curve
			RtlMoveMemory(charts[curveData->curve_id], curveData->curve, 1024 * sizeof(USHORT));
			// Indicate success
			status = STATUS_SUCCESS;
			Irp->IoStatus.Information = len;
		}
	}
	OrbUnlockData(orbData);
	}
failed:

	DbgOut(ORB_DBG_FEATURE, ("OrbGetSetFeature(): exit, status %x\n", status));
	return CompleteIrp(Irp, status, Irp->IoStatus.Information);
}
