//
// io.c
//

#include "hidsporb.h"

#ifndef	KeReadStateEvent
NTKERNELAPI
LONG
KeReadStateEvent(IN PRKEVENT Event);
#endif

// Detect and set up Orb
// Start thread to do all I/O
// Note, we shouldn't do any kind of I/O in this routine
NTSTATUS
OrbStartIo(IN PDEVICE_EXTENSION devExt)
{
	PDEVICE_OBJECT serObj;
	NTSTATUS status;
	HANDLE thread;
	ULONG orbModel;
	ULONG i;

	DbgOut(ORB_DBG_IO, ("OrbStartIo(): enter\n"));
	serObj = devExt->nextDevObj;
	// Try to open COM port
	status = OrbSerOpenPort(serObj, devExt->readIrp);
	// Fail if we couldn't do it
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_IO, ("OrbStartIo(): COM port open failed, status %x\n", status));
		goto failed;
	}
	// Detect ORB
	status = OrbDetect(serObj, GET_ORB_DATA(devExt), devExt->readIrp);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		OrbSerClosePort(serObj, devExt->readIrp);
		goto failed;
	}
	// Initialize Orb
	status = OrbInit(serObj, GET_ORB_DATA(devExt), devExt->readIrp, OrbCompletePacket, devExt);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		// Clean up
		OrbCleanup(serObj, GET_ORB_DATA(devExt), devExt->readIrp);
		// Close port
		OrbSerClosePort(serObj, devExt->readIrp);
		goto failed;
	}
	// Create thread
	status = PsCreateSystemThread(&thread, THREAD_ALL_ACCESS, NULL,
			NULL, NULL, OrbReadThread, devExt->devObj);
	// Fail if we couldn't create thread
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_IO, ("OrbStartIo(): create thread failed\n"));
		// Clean up
		OrbCleanup(serObj, GET_ORB_DATA(devExt), devExt->readIrp);
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
		DbgOut(ORB_DBG_IO, ("OrbStartIo(): cant reference thread\n"));
		// Tell the thread to terminate
		KeSetEvent(&devExt->threadTermEvent, 0, FALSE);
		// Clean up
		OrbCleanup(serObj, GET_ORB_DATA(devExt), devExt->readIrp);
		// Close COM port
		OrbSerClosePort(serObj, devExt->readIrp);
		goto failed;
	}
	// Indicate thread is started
	devExt->threadStarted = TRUE;
	// Close thread handle
	ZwClose(thread);
failed:
	DbgOut(ORB_DBG_IO, ("OrbStartIo(): exit, status %x\n"));

	return status;
}

// Flush queue, terminate thread,
// power down Orb and close COM port
VOID
OrbStopIo(IN PDEVICE_EXTENSION devExt)
{
	PDEVICE_OBJECT serObj;

	DbgOut(ORB_DBG_IO, ("OrbStopIo(): enter\n"));
	// Flush pending requests queue
	OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
	// Don't bother if thread hasn't been started
	if (devExt->threadStarted == FALSE) {
		DbgOut(ORB_DBG_IO, ("OrbStopIo(): thread not started\n"));
		goto exit;
	}
	// Tell thread to terminate
	KeSetEvent(&devExt->threadTermEvent, FALSE, 0);
	DbgOut(ORB_DBG_IO, ("OrbStopIo(): waiting for thread to terminate...\n"));
	// Wait for thread termination
	KeWaitForSingleObject(&devExt->threadTerminated, Executive, KernelMode, FALSE, NULL);
	DbgOut(ORB_DBG_IO, ("OrbStopIo(): thread terminated\n"));
	// Dereference thread object
	ObDereferenceObject(devExt->threadObj);
	serObj = devExt->nextDevObj;
	// Clean up
	OrbCleanup(serObj, GET_ORB_DATA(devExt), devExt->readIrp);
	// Close COM port
	DbgOut(ORB_DBG_IO, ("OrbStopIo(): closing port\n"));
	OrbSerClosePort(serObj, devExt->readIrp);
	// Flush pending requests queue again
	OrbFlushQueue(devExt, STATUS_DELETE_PENDING);
exit:
	DbgOut(ORB_DBG_IO, ("OrbStopIo(): exit\n"));
}

//
// This is where all ORB I/O happens
//
VOID
OrbReadThread(IN PDEVICE_OBJECT devObj)
{
	PDEVICE_EXTENSION devExt;
	PORB_DATA orbData;
	PDEVICE_OBJECT serObj;
	SERIAL_TIMEOUTS timeouts;
	NTSTATUS status;

	// Get our device extension
	devExt = GET_DEV_EXT(devObj);
	// Get Orb data
	orbData = GET_ORB_DATA(devExt);
	// Detect ORB
	serObj = devExt->nextDevObj;
	// Set zero timeouts
	RtlZeroMemory(&timeouts, sizeof(timeouts));
	status = OrbSerSetTimeouts(serObj, &timeouts);
	// Clear packet
	OrbClearBuffer(orbData, ORB_UNKNOWN_PACKET);
	// Read loop
	while (KeReadStateEvent(&devExt->threadTermEvent) == FALSE) {
		// Read and parse packet
		status = OrbReadPacket(orbData, serObj, devExt->readIrp);
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
