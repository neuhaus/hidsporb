//
// initunlo.c
//
// Initialization & unload functions
//
#define	DEFINE_GUID
#include <initguid.h>

#include "hidsporb.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, OrbUnload)
#endif

// Dummy function for now
NTSTATUS
OrbCompleteIrpInc(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

// Dummy function for now
NTSTATUS
OrbCompleteIrpDec(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	HID_MINIDRIVER_REGISTRATION hidReg;

	DbgOut(ORB_DBG_ALL, ("OrbDriverEntry()\n"));

	//
	// Create dispatch points for the IRPs.
	//
	// Save registry path
	OrbStoreRegistryPath(DriverObject, RegistryPath);

	DriverObject->DriverUnload			= OrbUnload;
	DriverObject->MajorFunction[IRP_MJ_PNP]		= OrbPnp;
	DriverObject->MajorFunction[IRP_MJ_POWER]	= OrbPower;
	DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = OrbInternalIoctl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = OrbCompleteIrpInc;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = OrbCompleteIrpDec;
	DriverObject->DriverExtension->AddDevice	= OrbAddDevice;

	// Set up HID minidrv registration
	RtlZeroMemory(&hidReg, sizeof(hidReg));
	hidReg.Revision = HID_REVISION;
	hidReg.DriverObject = DriverObject;
	hidReg.RegistryPath = RegistryPath;
	hidReg.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
	hidReg.DevicesArePolled	= FALSE;
	// Register minidriver
	status = HidRegisterMinidriver(&hidReg);

	return status;
}

VOID
OrbUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE ();

	//
	// The device object(s) should be NULL now
	// (since we unload, all the devices objects associated with this
	// driver must have been deleted.
	//

	ASSERT(DriverObject->DeviceObject == NULL);
	// Free key names
	OrbFreeRegistryPath();
	DbgOut(ORB_DBG_ALL, ("OrbUnload()\n"));

	return;
}
