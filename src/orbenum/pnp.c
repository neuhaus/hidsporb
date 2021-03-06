//
// pnp.c
//

#include "orbenum.h"

// Everything is nonpaged now
#if 0
#pragma alloc_text (PAGE, OrbEnumAddDevice)
#pragma alloc_text (PAGE, OrbEnumDispatchPnp)
#pragma alloc_text (PAGE, OrbEnumDispatchSystemControl)
#endif

NTSTATUS
OrbEnumAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo)
{

	PDEVICE_OBJECT    devObj = NULL;
	PDEVICE_EXTENSION devExt;
	UNICODE_STRING	  ntDeviceName;
	UNICODE_STRING	  win32DeviceName;
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();
	DbgOut(ORB_DBG_PNP, ("OrbEnumAddDevice(): enter\n"));
#if 0
	// Initialize device name
	RtlInitUnicodeString(&ntDeviceName, ORBENUM_DEVICE_NAME);
#endif
	//
	// Create a device object.
	//
	status = IoCreateDevice(DriverObject,
				sizeof(DEVICE_EXTENSION),
				NULL /* &ntDeviceName */,
				FILE_DEVICE_UNKNOWN,
				0,
				FALSE,
				&devObj);
	// Fail if we can't create device object
	if (!NT_SUCCESS(status)) {
		//
		// Either not enough memory to create a deviceobject or another
		// deviceobject with the same name exits. This could happen
		// if you install another instance of this device.
		//
		DbgOut(ORB_DBG_PNP, ("OrbEnumAddDevice(): IoCreateDevice() failed %x\n", status));

		return status;
	}
#if 0
	RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);
	// Create symbolic link so user-mode programs can access this
	// BTW, this is not really useful, just for debugging
	// I'll remove symlink later
	status = IoCreateSymbolicLink(&win32DeviceName, &ntDeviceName);
	// If we we couldn't create the link then
	// abort installation.
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_PNP, ("OrbEnumAddDevice(): IoCreateSymLink() failed %x\n", status));
		IoDeleteDevice(devObj);
		return status;
	}
#endif
	devExt = (PDEVICE_EXTENSION) devObj->DeviceExtension;
	// Save bus PDO for reference
	devExt->busPdo = pdo;
	// Attach to BUS device stack
	// in our case we will have root bus created PDO
	devExt->nextDevObj = IoAttachDeviceToDeviceStack(devObj, pdo);
	// We couldn't attach to lower BUS device
	if (devExt->nextDevObj == NULL) {
		// Delete symlink & device object
#if 0
		IoDeleteSymbolicLink(&win32DeviceName);
#endif
		IoDeleteDevice(devObj);

		return STATUS_NO_SUCH_DEVICE;
	}
	IoInitializeRemoveLock(&devExt->RemoveLock, ORBENUM_TAG,
				1,
				5);
	// We don't hold a pagefile, so we don't care!
	devObj->Flags |= DO_POWER_PAGABLE;
	// Initialize device extension
	// indicate it's FDO
	devExt->Flags = 0;
	// Pointer to self
	devExt->devObj = devObj;
	devExt->Removed = FALSE;
	devExt->Started = FALSE;
	devExt->numDevices = 0;
	devExt->DriverObject = DriverObject;
	// Zero device array
	RtlZeroMemory(&devExt->devArray[0], sizeof(devExt->devArray));
	// Initialize array mutex
	ExInitializeFastMutex(&devExt->devArrayMutex);
	OrbInitPortList(devExt);
	// Set up PNP notification to know about COM ports
	// Note that IoRegisterPnpNfy references our driverobject
	// so make sure we deregister this stuff before (!!!) unloading
	status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
						PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
						(LPGUID) &GUID_CLASS_COMPORT, //SERENUM_BUS_ENUMERATOR,
						devExt->DriverObject,
						OrbPnpNotifyCallback,
						devExt,
						&devExt->notifyEntry);
	//GUID_CLASS_COMPORT
	// Fail if unsuccessful
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_PNP, ("OrbPnpNotifyReg(): failed %x\n", status));
#if 0
		// Delete symlink
		IoDeleteSymbolicLink(&win32DeviceName);
#endif
		// Detach from BUS
		IoDetachDevice(devExt->nextDevObj);
		// Delete device object
		IoDeleteDevice(devObj);

		return STATUS_NO_SUCH_DEVICE;
	}
	// OK, we're ready to go!
	devObj->Flags &= ~DO_DEVICE_INITIALIZING;
	DbgOut(ORB_DBG_PNP, ("OrbEnumAddDevice: %p to %p->%p \n", devObj, 
						devExt->nextDevObj,
						pdo));
	DbgOut(ORB_DBG_PNP, ("OrbEnumAddDevice(): exit\n"));

	return STATUS_SUCCESS;
}

// Common IRP_MJ_PNP handler
NTSTATUS
OrbEnumDispatchPnp(IN PDEVICE_OBJECT devObj, IN PIRP Irp)
{
	PDEVICE_EXTENSION devExt;

	PAGED_CODE();
	devExt = (PDEVICE_EXTENSION) devObj->DeviceExtension;
	// Check if this is PDO
	if (devExt->Flags) {
		// Call PDO handler
		return OrbPdoPnp((PPDO_EXTENSION) devExt, Irp);
	} else {
		// Call FDO handler
		return OrbFdoPnp(devExt, Irp);
	}
}
