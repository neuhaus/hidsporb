//
// serial.c
//
// Serial I/O library used both by hidsporb and orbenum drivers
//

// This file is independent from driver which uses it
#include "serinc.h"

#ifndef	IoReuseIrp
NTKERNELAPI VOID
IoReuseIrp(IN OUT PIRP Irp, IN NTSTATUS Iostatus);
#endif

// Used to issue Ioctls w/o parameters
NTSTATUS
OrbSerSyncIoctl(IN PDEVICE_OBJECT devObj,
		IN BOOLEAN Internal,
		IN ULONG Ioctl,
		IN PKEVENT event,
		IN PIO_STATUS_BLOCK Iosb)
{
	// Simply call *Ex version of this function
	return OrbSerSyncIoctlEx(devObj, Internal, Ioctl, event, Iosb, NULL, 0, NULL, 0);
}

// Used to issue Ioctls with parameters
NTSTATUS
OrbSerSyncIoctlEx(IN PDEVICE_OBJECT devObj,
		IN BOOLEAN Internal,
		IN ULONG Ioctl,
		IN PKEVENT event,
		IN PIO_STATUS_BLOCK Iosb,
		IN PVOID inBuffer,
		IN ULONG inBufferLen,
		IN PVOID outBuffer,
		IN ULONG outBufferLen)
{
	PIRP Irp;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	DbgOut(ORB_DBG_SER, ("OrbSerIoctlEx(): enter\n"));
	// Clear event
	KeClearEvent(event);
	// Build ioctl Irp
	Irp = IoBuildDeviceIoControlRequest(Ioctl,
					devObj,
					inBuffer,
					inBufferLen,
					outBuffer,
					outBufferLen,
					Internal,
					event,
					Iosb);
	// Check if we allocated Irp
	if (Irp == NULL) {
		DbgOut(ORB_DBG_SER, ("OrbSerIoctlEx(): no IRP\n"));
		goto failed;
	}
	// We have Irp, call driver
	status = IoCallDriver(devObj, Irp);
	// Wait for completion
	if (status == STATUS_PENDING) {
		KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
	}
	// Set status
	status = Iosb->Status;
failed:
	DbgOut(ORB_DBG_SER, ("OrbSerIoctlEx(): exit, status %x\n", status));

	return status;
}

// This function reads upto bufSize characters from COM port
NTSTATUS
OrbSerRead(IN PDEVICE_OBJECT devObj, IN PIRP readIrp, OUT PCHAR readBuffer, IN ULONG bufSize, OUT PULONG bufRead)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PIRP Irp = NULL;
	PIO_STACK_LOCATION irpSp;
	ULONG nRead;

	//DbgOut(ORB_DBG_SER, ("OrbSerRead(): enter buf %p, size %d, bufRead %p\n", readBuffer, bufSize, bufRead));

	// Check all parameters
	if ((readBuffer == NULL) || (bufSize == 0) || (bufRead == NULL)) {
		DbgOut(ORB_DBG_SER, ("OrbSerRead(): invalid parameter passed\n"));
		status = STATUS_INVALID_PARAMETER;
		goto failed;
	}
	// Initialize bufRead
	*bufRead = 0;

	// Allocate Irp if not given
	if (readIrp == NULL) {
		Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
		if (Irp == NULL) {
			DbgOut(ORB_DBG_SER, ("OrbSerRead(): no Irp\n"));
			goto failed;
		}
		readIrp = Irp;
	}

	nRead = 0;
	// Read loop
	while (nRead < bufSize) {
		// Reinitialize Irp
		IoReuseIrp(readIrp, STATUS_SUCCESS);
		// Skip our stack location
		IoSetNextIrpStackLocation(readIrp);
		// Get current Irp stack location
		irpSp = IoGetCurrentIrpStackLocation(readIrp);
		// Set up stack
		// Set up function code
		irpSp->MajorFunction = IRP_MJ_READ;
		// Set up offset & length
		irpSp->Parameters.Read.ByteOffset.QuadPart = 0;
		// TODO: should we read full buffer in one step?
		irpSp->Parameters.Read.Length = 1;
		// Set up buffer pointer
		readIrp->AssociatedIrp.SystemBuffer = readBuffer;
		// Call next driver and wait for completion
		status = CallNextDriverWait(devObj, readIrp);
		// Check if read failed
		if (!NT_SUCCESS(status)) {
			DbgOut(ORB_DBG_SER, ("OrbSerRead(): failed, nRead %d %x\n", nRead, status));
			break;
		}
		// Update readBuffer & friends
		readBuffer++;
		nRead++;
		//*bufRead++;
	}
	// Free Irp, if allocated
	if (Irp != NULL) {
		IoFreeIrp(Irp);
	}
	*bufRead = nRead;
failed:
	//DbgOut((ORB_DBG_SER, "OrbSerRead(): exit nRead %d, status %x\n", nRead, status));

	return status;
}

// This function reads 1 character from COM port
NTSTATUS
OrbSerReadChar(IN PDEVICE_OBJECT devObj, IN PIRP Irp, OUT PCHAR pChar)
{
	NTSTATUS status;
	ULONG nRead;

	status = OrbSerRead(devObj, Irp, pChar, 1, &nRead);
	// We don't care about errors here yet
	// maybe later

	return status;
}

// This function writes upto bufSize characters into COM port
NTSTATUS
OrbSerWrite(IN PDEVICE_OBJECT devObj, IN PIRP writeIrp, IN PCHAR writeBuffer, IN ULONG bufSize, OUT PULONG bufWritten)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PIRP Irp = NULL;
	PIO_STACK_LOCATION irpSp;
	ULONG nWritten;

	DbgOut(ORB_DBG_SER, ("OrbSerWrite(): enter buf %p, size %d, bufWritten %p\n", writeBuffer, bufSize, bufWritten));

	// Check all parameters
	if ((writeBuffer == NULL) || (bufSize == 0) || (bufWritten == NULL)) {
		DbgOut(ORB_DBG_SER, ("OrbSerWrite(): invalid parameter passed\n"));
		status = STATUS_INVALID_PARAMETER;
		goto failed;
	}
	// Initialize bufRead
	*bufWritten = 0;

	// Allocate Irp if not given
	if (writeIrp == NULL) {
		Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
		if (Irp == NULL) {
			DbgOut(ORB_DBG_SER, ("OrbSerWrite(): no Irp\n"));
			goto failed;
		}
		writeIrp = Irp;
	}

	nWritten = 0;
	// Read loop
	while (nWritten < bufSize) {
		// Reinitialize Irp
		IoReuseIrp(writeIrp, STATUS_SUCCESS);
		// Skip our stack location
		IoSetNextIrpStackLocation(writeIrp);
		// Get current Irp stack location
		irpSp = IoGetCurrentIrpStackLocation(writeIrp);
		// Set up stack
		// Set up function code
		irpSp->MajorFunction = IRP_MJ_WRITE;
		// Set up offset & length
		irpSp->Parameters.Read.ByteOffset.QuadPart = 0;
		// TODO: should we read full buffer in one step?
		irpSp->Parameters.Read.Length = 1;
		// Set up buffer pointer
		writeIrp->AssociatedIrp.SystemBuffer = writeBuffer;
		// Call next driver and wait for completion
		status = CallNextDriverWait(devObj, writeIrp);
		// Check if read failed
		if (!NT_SUCCESS(status)) {
			DbgOut(ORB_DBG_SER, ("OrbWriteRead(): failed, nWritten %d %x\n", nWritten, status));
			break;
		}
		// Update writeBuffer & friends
		writeBuffer++;
		nWritten++;
		//*bufRead++;
	}
	// Free Irp, if allocated
	if (Irp != NULL) {
		IoFreeIrp(Irp);
	}
	*bufWritten = nWritten;
failed:
	DbgOut(ORB_DBG_SER, ("OrbSerWrite(): exit nWritten %d, status %x\n", nWritten, status));

	return status;
}

// This function writes 1 character from COM port
NTSTATUS
OrbSerWriteChar(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN CHAR Char)
{
	NTSTATUS status;
	ULONG nWritten;

	status = OrbSerWrite(devObj, Irp, &Char, 1, &nWritten);
	// We don't care about errors here yet
	// maybe later

	return status;
}

NTSTATUS
OrbSerOpenPort(IN PDEVICE_OBJECT devObj, IN PIRP openIrp)
{
	PIRP Irp = NULL;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	DbgOut(ORB_DBG_SER, ("OrbOpenPort(): enter\n"));
	// Allocate Irp, if not given
	if (openIrp == NULL) {
		Irp = IoAllocateIrp(devObj->StackSize, FALSE);
		if (Irp == NULL) {
			DbgOut(ORB_DBG_SER, ("OrbOpenPort(): no Irp\n"));
			goto failed;
		}
		openIrp = Irp;
	}
	// Init Irp
	IoReuseIrp(openIrp, STATUS_SUCCESS);
	IoSetNextIrpStackLocation(openIrp);
	// Get next stack location
	irpSp = IoGetCurrentIrpStackLocation(openIrp);
	RtlZeroMemory(irpSp, sizeof(IO_STACK_LOCATION));
	// Set up stack location
	irpSp->MajorFunction = IRP_MJ_CREATE;
	// Call serial driver
	status = CallNextDriverWait(devObj, openIrp);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_SER, ("OrbOpenPort(): cant open, status %x\n", status));
	}
	// Free Irp, if allocated
	if (Irp != NULL) {
		IoFreeIrp(Irp);
	}
failed:
	DbgOut(ORB_DBG_SER, ("OrbOpenPort(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbSerClosePort(IN PDEVICE_OBJECT devObj, IN PIRP closeIrp)
{
	PIRP Irp = NULL;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	DbgOut(ORB_DBG_SER, ("OrbClosePort(): enter\n"));
	// Allocate Irp, if not given
	if (closeIrp == NULL) {
		Irp = IoAllocateIrp(devObj->StackSize, FALSE);
		if (Irp == NULL) {
			DbgOut(ORB_DBG_SER, ("OrbClosePort(): no Irp\n"));
			goto failed;
		}
		closeIrp = Irp;
	}
	// Init Irp
	IoReuseIrp(closeIrp, STATUS_SUCCESS);
	IoSetNextIrpStackLocation(closeIrp);
	// Get next stack location
	irpSp = IoGetCurrentIrpStackLocation(closeIrp);
	RtlZeroMemory(irpSp, sizeof(IO_STACK_LOCATION));
	// Set up stack location
	irpSp->MajorFunction = IRP_MJ_CLOSE;
	// Call serial driver
	status = CallNextDriverWait(devObj, closeIrp);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_SER, ("OrbClosePort(): cant close?, status %x\n", status));
	}
	// Init Irp again
	IoReuseIrp(closeIrp, STATUS_SUCCESS);
	IoSetNextIrpStackLocation(closeIrp);
	// Get next stack location
	irpSp = IoGetCurrentIrpStackLocation(closeIrp);
	RtlZeroMemory(irpSp, sizeof(IO_STACK_LOCATION));
	// Set up stack location
	irpSp->MajorFunction = IRP_MJ_CLEANUP;
	// Call serial driver
	status = CallNextDriverWait(devObj, closeIrp);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_SER, ("OrbClosePort(): cant clean up?, status %x\n", status));
	}
	if (Irp != NULL) {
		IoFreeIrp(Irp);
	}
failed:
	DbgOut(ORB_DBG_SER, ("OrbClosePort(): exit, status %x\n", status));

	return status;

}

NTSTATUS
OrbSerGetLineControl(IN PDEVICE_OBJECT devObj, PSERIAL_LINE_CONTROL lineControl)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_LINE_CONTROL,
				&event, &iosb, NULL, 0,
				lineControl, sizeof(SERIAL_LINE_CONTROL));

	return status;
}

NTSTATUS
OrbSerSetLineControl(IN PDEVICE_OBJECT devObj, PSERIAL_LINE_CONTROL lineControl)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_SET_LINE_CONTROL,
				&event, &iosb, lineControl, sizeof(SERIAL_LINE_CONTROL),
				NULL, 0);

	return status;
}

NTSTATUS
OrbSerGetTimeouts(IN PDEVICE_OBJECT devObj, PSERIAL_TIMEOUTS timeouts)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_TIMEOUTS,
				&event, &iosb, NULL, 0,
				timeouts, sizeof(SERIAL_TIMEOUTS));

	return status;
}

NTSTATUS
OrbSerSetTimeouts(IN PDEVICE_OBJECT devObj, PSERIAL_TIMEOUTS timeouts)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_SET_TIMEOUTS,
				&event, &iosb, timeouts, sizeof(SERIAL_TIMEOUTS),
				NULL, 0);

	return status;
}

NTSTATUS
OrbSerGetBaudRate(IN PDEVICE_OBJECT devObj, PSERIAL_BAUD_RATE baudRate)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_BAUD_RATE,
				&event, &iosb, NULL, 0,
				baudRate, sizeof(SERIAL_BAUD_RATE));

	return status;
}

NTSTATUS
OrbSerSetBaudRate(IN PDEVICE_OBJECT devObj, PSERIAL_BAUD_RATE baudRate)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_SET_BAUD_RATE,
				&event, &iosb, baudRate, sizeof(SERIAL_BAUD_RATE),
				NULL, 0);

	return status;
}

NTSTATUS
OrbSerGetHandflow(IN PDEVICE_OBJECT devObj, PSERIAL_HANDFLOW handFlow)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_HANDFLOW,
				&event, &iosb, NULL, 0,
				handFlow, sizeof(SERIAL_HANDFLOW));

	return status;
}

NTSTATUS
OrbSerSetHandFlow(IN PDEVICE_OBJECT devObj, PSERIAL_HANDFLOW handFlow)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_SET_HANDFLOW,
				&event, &iosb, handFlow, sizeof(SERIAL_HANDFLOW),
				NULL, 0);

	return status;
}

NTSTATUS
OrbSerGetDtr(IN PDEVICE_OBJECT devObj, PULONG Dtr)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_DTRRTS,
				&event, &iosb, NULL, 0,
				Dtr, sizeof(ULONG));

	return status;
}

NTSTATUS
OrbSerGetRts(IN PDEVICE_OBJECT devObj, PULONG Rts)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_DTRRTS,
				&event, &iosb, NULL, 0,
				Rts, sizeof(ULONG));

	return status;
}

NTSTATUS
OrbSerSetDtr(IN PDEVICE_OBJECT devObj)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctl(devObj, FALSE, IOCTL_SERIAL_SET_DTR,
				&event, &iosb);

	return status;
}

NTSTATUS
OrbSerClrDtr(IN PDEVICE_OBJECT devObj)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctl(devObj, FALSE, IOCTL_SERIAL_CLR_DTR,
				&event, &iosb);

	return status;
}

NTSTATUS
OrbSerSetRts(IN PDEVICE_OBJECT devObj)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctl(devObj, FALSE, IOCTL_SERIAL_SET_RTS,
				&event, &iosb);

	return status;
}

NTSTATUS
OrbSerClrRts(IN PDEVICE_OBJECT devObj)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctl(devObj, FALSE, IOCTL_SERIAL_CLR_RTS,
				&event, &iosb);

	return status;
}

NTSTATUS
OrbSerFlush(IN PDEVICE_OBJECT devObj, IN ULONG Mask)
{
	KEVENT event;
	NTSTATUS status;
	IO_STATUS_BLOCK iosb;

	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Send Ioctl
	status = OrbSerSyncIoctlEx(devObj, FALSE, IOCTL_SERIAL_GET_DTRRTS,
				&event, &iosb, &Mask, sizeof(ULONG),
				NULL, 0);

	return status;
}
