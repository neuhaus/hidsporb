//
// io.c
//

#include "hidsporb.h"

#ifndef	KeReadStateEvent
NTKERNELAPI
LONG
KeReadStateEvent(IN PRKEVENT Event);
#endif

// Set up & power up SpaceOrb
// Start thread to do all I/O
// Note, we shouldn't do any kind of I/O in this routine
NTSTATUS
OrbStartIo(IN PDEVICE_EXTENSION devExt)
{
	PDEVICE_OBJECT serObj;
	NTSTATUS status;
	HANDLE thread;

	DbgOut(("OrbStartIo(): enter\n"));
	serObj = devExt->nextDevObj;
	// Try to open COM port
	status = OrbSerOpenPort(serObj, devExt->readIrp);
	// Fail if we couldn't do it
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbStartIo(): COM port open failed, status %x\n", status));
		goto failed;
	}
	// Now, initialize port
	status = OrbInitComPort(serObj);
	// fail if we coudn't do it
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbStartIo(): COM port init failed, status %x\n", status));
		OrbPowerDown(serObj);
		OrbSerClosePort(serObj, devExt->readIrp);
		goto failed;
	}
	// Create thread
	status = PsCreateSystemThread(&thread, THREAD_ALL_ACCESS, NULL,
			NULL, NULL, OrbReadThread, devExt->devObj);
	// Fail if we couldn't create thread
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbStartIo(): create thread failed\n"));
		// Power down Orb
		OrbPowerDown(serObj);
		// Close COM port
		OrbSerClosePort(serObj, devExt->readIrp);
		goto failed;
	}
	// Reference thread object
	// We dereference thread object in OrbStopIo()
	status = ObReferenceObjectByHandle(thread, THREAD_ALL_ACCESS, NULL,
					KernelMode,&devExt->threadObj, NULL);
	// Fail if we couldn't reference thread
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbStartIo(): cant reference thread\n"));
		// Tell the thread to terminate
		KeSetEvent(&devExt->threadTermEvent, 0, FALSE);
	} else {
		devExt->threadStarted = TRUE;
	}
	// Close thread handle
	ZwClose(thread);
failed:
	DbgOut(("OrbStartIo(): exit, status %x\n"));

	return status;
}

// Flush queue, terminate thread,
// power down Orb and close COM port
VOID
OrbStopIo(IN PDEVICE_EXTENSION devExt)
{
	PDEVICE_OBJECT serObj;

	DbgOut(("OrbStopIo(): enter\n"));
	// Flush pending requests queue
	OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
	// Don't bother if thread hasn't been started
	if (devExt->threadStarted == FALSE) {
		DbgOut(("OrbStopIo(): thread not started\n"));
		goto exit;
	}
	// Tell thread to terminate
	KeSetEvent(&devExt->threadTermEvent, FALSE, 0);
	DbgOut(("OrbStopIo(): waiting for thread to terminate...\n"));
	// Wait for thread termination
	KeWaitForSingleObject(&devExt->threadTerminated, Executive, KernelMode, FALSE, NULL);
	DbgOut(("OrbStopIo(): thread terminated\n"));
	// Dereference thread object
	ObDereferenceObject(devExt->threadObj);
	// Power down
	serObj = devExt->nextDevObj;
	DbgOut(("OrbStopIo(): powering down\n"));
	OrbPowerDown(serObj);
	DbgOut(("OrbStopIo(): closing port\n"));
	// Close COM port
	OrbSerClosePort(serObj, devExt->readIrp);
	// Flush pending requests queue again
	OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
exit:
	DbgOut(("OrbStopIo(): exit\n"));
}

//
// This is where all ORB I/O happens
//
VOID
OrbReadThread(IN PDEVICE_OBJECT devObj)
{
	PDEVICE_EXTENSION devExt;
	PDEVICE_OBJECT serObj;
	SERIAL_TIMEOUTS timeouts;
	NTSTATUS status;

	devExt = GET_DEV_EXT(devObj);
	// Detect ORB
	serObj = devExt->nextDevObj;
	status = OrbDetect(serObj);
	// Set zero timeouts
	RtlZeroMemory(&timeouts, sizeof(timeouts));
	status = OrbSerSetTimeouts(serObj, &timeouts);
	// Clear packet
	OrbClearBuffer(devExt, ORB_UNKNOWN_PACKET);
	// Read loop
	while (KeReadStateEvent(&devExt->threadTermEvent) == FALSE) {
		// Read and parse packet
		status = OrbReadPacket(devExt);
		// Break if we have error
		if (!NT_SUCCESS(status)) {
			break;
		}
	}
	// Cancel Irp
	IoCancelIrp(devExt->readIrp);
	// Indicate thread is terminated
	KeSetEvent(&devExt->threadTerminated, 0, FALSE);
	devExt->threadStarted = FALSE;
	// Terminate thread
	PsTerminateSystemThread(0);	
}
