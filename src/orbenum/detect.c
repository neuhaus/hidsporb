//
// detect.c
// ORB detection routines
//

// Define ORB_SIMULATION so orbenum will always find devices
//#define	ORB_SIMULATION
#include "orbenum.h"

ULONG OrbEnumNumDevices = 0;
extern ORB_MODEL orbModels[];

// This function is used to detect if there is ORB present
ULONG
OrbDetect(IN PDEVICE_OBJECT serObj)
{
	PIRP Irp;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	ULONG model = 0xffff;
	CHAR buffer[257];
	ULONG size, i;

	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): enter\n"));
#ifdef	ORB_SIMULATION
	// Simulation
	// There is always ORB
	model = ((OrbEnumNumDevices++) & 1);
#else
	// Initialize COM port
	status = OrbInitComPort(serObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		OrbPowerDown(serObj);
		goto failed;
	}
	// Read string from ORB
	size = OrbReadData(serObj, buffer, sizeof(buffer)-1);
	// Check if we read something
	if (size == 0) {
		DbgOut(ORB_DBG_DETECT, ("OrbDetect(): nothing read\n"));
		OrbPowerDown(serObj);
		goto failed;
	}
	// Replace junk charactes with spaces, this will let us use strstr()
	for (i = 0; i < sizeof(buffer)-1; i++) {
		// Replace junk character with space
		if (buffer[i] < ' ') {
			buffer[i] = ' ';
		}
	}
	// Check which ORB is that
	buffer[size] = 0;
	if (strstr(buffer, "R Spaceball (R)") != NULL) {
		DbgOut(ORB_DBG_DETECT, ("OrbDetect(): detected SPACEORB\n"));
		model = 0;
	} else
	if (strstr(buffer, "@1 Spaceball alive and well") != NULL) {
		DbgOut(ORB_DBG_DETECT, ("OrbDetect(): detected SPACEBALL\n"));
		model = 1;
	}
	// Power down ORB
	OrbPowerDown(serObj);
#endif
failed:
	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): exit, model %d\n", model));

	return model;
}

//
// Called when port arrives
//
NTSTATUS
OrbPortArrival(IN PDEVICE_EXTENSION devExt, IN PORB_NOTIFY_CONTEXT ctx)
{
	PORB_PORT port;
	UNICODE_STRING linkName;
	PFILE_OBJECT fileObj;
	PDEVICE_OBJECT serObj, pdo;
	PPDO_EXTENSION pdevExt;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	ULONG numDevice, model, free = 1, instanceId;

	// Get remove lock
	status = IoAcquireRemoveLock(&devExt->RemoveLock, ctx);
	// Fail if we can't
	// this means device is removing
	if (!NT_SUCCESS(status)) {
		goto failed;
	}
	// Get ORB_PORT structure
	port = OrbGetPort(devExt, ctx->linkName, wcslen(ctx->linkName), TRUE);
	// Fail if we can't
	if (port == NULL) {
		IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
		goto failed;
	}
	// Initialize link name string
	RtlInitUnicodeString(&linkName, ctx->linkName);
	// Open COM port device object
	status = IoGetDeviceObjectPointer(&linkName,
					STANDARD_RIGHTS_ALL,
					&fileObj,
					&serObj);
	// Fail if we can't
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): cant open %ws\n", ctx->linkName));
		// Clean up
		OrbWakeupPort(devExt, port, ORB_DEVICE_ARRIVING);
		IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
		goto failed;
	}
	// We've got device and file objects
	// Detect ORB
	DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): got dev %p file %p\n", serObj, fileObj));
	model = OrbDetect(serObj);
	// Do nothing if ORB is not detected
	if (model == 0xffff) {
		DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): ORB not detected at %ws\n", ctx->linkName));
		// Clean up
		OrbWakeupPort(devExt, port, ORB_DEVICE_ARRIVING);
		IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
		ObDereferenceObject(fileObj);
		goto failed;
	}
	// OK, ORB is detected, create PDO for it
	status = OrbCreatePdo(devExt, &pdo);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): ORB detected at %ws, but no PDO\n", ctx->linkName));
		// Clean up
		OrbWakeupPort(devExt, port, ORB_DEVICE_ARRIVING);
		IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
		ObDereferenceObject(fileObj);
		goto failed;
	}
	// We created PDO, install it into devArray
	OrbLockPdos(devExt);
	numDevice = InterlockedIncrement(&devExt->numDevices) - 1;
	// Get new InstanceId
	instanceId = OrbGetNextPdoNumber(devExt);
	// Fail if we couldn't
	if (instanceId == ORB_MAX_DEVICES) {
		DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): cant get instanceid?\n"));
		OrbUnlockPdos(devExt);
		pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
		// Mark PDO as removed and delete it
		OrbMarkPdoAsRemoved(pdevExt);
		OrbDeletePdo(pdevExt);
		goto cleanup;
	}
	OrbInitPdo(pdo, serObj, NULL, ctx->linkName, orbModels[model].model,
			orbModels[model].hardwareId, orbModels[model].deviceId,
			instanceId, numDevice);
	devExt->devArray[numDevice] = pdo;
	OrbUnlockPdos(devExt);
	// Don't free linkName, it's used by PDO
	free = 0;
cleanup:
	// Reference serObj and deref fileObj
	ObReferenceObject(serObj);
	ObDereferenceObject(fileObj);
	// Clean up
	OrbWakeupPort(devExt, port, ORB_DEVICE_ARRIVING);
	// NOTE: ORB minibus FDO might be not started yet!
	// Force bus rescan _only_ if FDO has been started
	if (devExt->Started) {
		DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): rescanning bus\n"));
		IoInvalidateDeviceRelations(devExt->busPdo, BusRelations);
	}
	IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
	// Indicate success
	status = STATUS_SUCCESS;
failed:
	// Free link name, if needed
	if (free) {
		ExFreePool(ctx->linkName);
	}
	DbgOut(ORB_DBG_DETECT, ("OrbPortArrival(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbPortRemoval(IN PDEVICE_EXTENSION devExt, IN PORB_NOTIFY_CONTEXT ctx)
{
	PORB_PORT port;
	PPDO_EXTENSION pdevExt, pdevExt1;
	ULONG i, found = 0, len;
	NTSTATUS status;

	DbgOut(ORB_DBG_DETECT, ("OrbPortRemoval(): enter %ws\n", ctx->linkName));
	// Try to get remove lock first
	status = IoAcquireRemoveLock(&devExt->RemoveLock, ctx);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		// We don't care
		goto failed;
	}
	// Get or find port
	len = wcslen(ctx->linkName);
	port = OrbGetPort(devExt, ctx->linkName, len, FALSE);
	// Fail if no success
	if (port == NULL) {
		// Bad thing, what do we do?
		// Should we fail here?
		DbgOut(ORB_DBG_DETECT, ("OrbPortRemoval(): no port\n"));
	}
	// FIXME: add code to wait for pending ARRIVAL
	// It's not here yet!!!
	// Lock devArray
	OrbLockPdos(devExt);
	// Find matching port
	for (i = 0; i < devExt->numDevices; i++) {
		pdevExt = (PPDO_EXTENSION) devExt->devArray[i]->DeviceExtension;
		DbgOut(ORB_DBG_DETECT, ("OrbPortRemoval(): devnum %d devobj %p linkNam %ws\n",
					i,
					devExt->devArray[i],
					pdevExt->linkName));
		//if (wcscmp(pdevExt->linkName, ctx->linkName) == 0) {
		if (len != wcslen(pdevExt->linkName)) {
			continue;
		}
		// Exit from loop if we found device
		if (RtlCompareMemory(pdevExt->linkName, ctx->linkName, len * sizeof(WCHAR)) == (len * sizeof(WCHAR))) {
			found = 1;
			break;
		}
	}
	// Remove device from array if found
	if (found) {
		devExt->devArray[i] = NULL;
		devExt->numDevices--;
		// Copy array
		if (devExt->numDevices != i) {
			RtlCopyMemory(&devExt->devArray[i],
					&devExt->devArray[i+1],
					(devExt->numDevices - i) * sizeof(PDEVICE_OBJECT));
		}
		// Update PDOs numDevice
		for (i = 0; i < devExt->numDevices; i++) {
			pdevExt1 = (PPDO_EXTENSION) devExt->devArray[i]->DeviceExtension;
			pdevExt1->numDevice = i;
		}
	}
	// Unlock devArray
	OrbUnlockPdos(devExt);
	// Release remove lock
	IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
	// Delete PDO if found
	if (found) {
		// Mark PDO as removed
		OrbMarkPdoAsRemoved(pdevExt);
		// Force bus rescan if FDO has been started
		if (devExt->Started) {
			IoInvalidateDeviceRelations(devExt->busPdo, BusRelations);
		}
	}
	status = STATUS_SUCCESS;
	// Clean up
	if (port) {
		OrbWakeupPort(devExt, port, ORB_DEVICE_REMOVING);
	}
failed:
	DbgOut(ORB_DBG_DETECT, ("OrbPortRemoval(): exit %ws, status %x\n", ctx->linkName, status));

	return status;
}

VOID
OrbDeletePdo(IN PPDO_EXTENSION pdevExt)
{
	PDEVICE_OBJECT serObj;
	PDEVICE_EXTENSION devExt;
	ULONG instanceId;

	// OK, we can do with PDO whatever we want
	devExt = (PDEVICE_EXTENSION) pdevExt->fdo->DeviceExtension;
	// Free link name buffer
	ExFreePool(pdevExt->linkName);
	// Get serial device object
	serObj = pdevExt->nextDevObj;
	// Zero next devobj
	pdevExt->nextDevObj = NULL;
	// Dereference COM port device object
	ObDereferenceObject(serObj);
	// Get instance ID
	instanceId = pdevExt->instanceId;
	pdevExt->instanceId = 0xffff;
	// Free instance id if any
	if (instanceId < ORB_MAX_DEVICES) {
		OrbFreePdoNumber(devExt, instanceId);
	}
	// Delete device object
	IoDeleteDevice(pdevExt->devObj);
}

VOID
OrbMarkPdoAsRemoved(IN PPDO_EXTENSION pdevExt)
{
	PDEVICE_EXTENSION devExt;

	// Mark PDO as removed
	pdevExt->Removed = TRUE;
	// Get FDO device extension
	devExt = (PDEVICE_EXTENSION) pdevExt->fdo->DeviceExtension;
	// Invalidate device state _only_ if FDO has been started
	if (devExt->Started) {
		IoInvalidateDeviceState(pdevExt->devObj);
	}
}

// Get next suitable PDO number
ULONG
OrbGetNextPdoNumber(IN PDEVICE_EXTENSION devExt)
{
	ULONG num, bit;

	// Find next available Instance ID
	for (num = 0, bit = 1; num < ORB_MAX_DEVICES; num++, bit <<= 1) {
		// Check if bit is zero
		if ((devExt->pdoMask & bit) == 0) {
			// Mark it as used and return new InstanceID
			devExt->pdoMask |= bit;

			return num;
		}
	}

	// We didn't find available InstanceID
	return ORB_MAX_DEVICES;
}

VOID
OrbFreePdoNumber(IN PDEVICE_EXTENSION devExt, IN ULONG instanceId)
{
	// Clear bit
	devExt->pdoMask &= ~(1 << instanceId);
}
