//
// orbio.c
//
//
// Orb I/O library
//

#include "hidsporb.h"

// This function initializes COM port and powers up Orb
// Note, we could extend it to support different Orb models
// e.g. like that:
// OrbInitComPort(IN PDEVICE_OBJECT devObj, IN ULONG baudRate, ...)
NTSTATUS
OrbInitComPort(IN PDEVICE_OBJECT devObj)
{
	SERIAL_BAUD_RATE baudRate;
	SERIAL_LINE_CONTROL lineControl;
	SERIAL_HANDFLOW handFlow;
	SERIAL_TIMEOUTS timeouts;
	NTSTATUS status;

	DbgOut(("OrbInitComPort(): enter\n"));
	// Phase 1
	// Set wait mask to zero
	// Initialize port
	// Set up timeous
	RtlZeroMemory(&timeouts, sizeof(timeouts));
	// Set timeouts
	status = OrbSerSetTimeouts(devObj, &timeouts);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set timeouts\n"));
		goto failed;
	}
	// Initialize handflow
	handFlow.ControlHandShake = SERIAL_DTR_CONTROL;
	handFlow.FlowReplace = SERIAL_RTS_CONTROL;
	handFlow.XonLimit = 0;
	handFlow.XoffLimit = 0;
	// Set handflow stuff
	status = OrbSerSetHandFlow(devObj, &handFlow);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set hand flow\n"));
		goto failed;
	}
	// Power down
	status = OrbPowerDown(devObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to power down\n"));
		goto failed;
	}
	// Wait a bit
	KeStallExecutionProcessor(200);
	// We could use that function:
	//KeDelayExecutionThread(KernelMode, FALSE, 200 * 10);
	// Phase 2
	// Initialize baud rate
	baudRate.BaudRate = 9600;
	// Try to set baud rate
	status = OrbSerSetBaudRate(devObj, &baudRate);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set baud rate\n"));
		goto failed;
	}
	// Initialize line control
//	lineControl.WordLength = 8;
//	lineControl.Parity = NO_PARITY;
//	lineControl.StopBits = STOP_BIT_1;
	lineControl.WordLength = 8;
	lineControl.Parity = 0;
	lineControl.StopBits = 1;
	// Set word size, stop bits and parity stuff
	status = OrbSerSetLineControl(devObj, &lineControl);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set line control\n"));
		goto failed;
	}
	// Flush buffers
	status = OrbSerFlush(devObj,
			SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	// Go ahead and power up the orb
	status = OrbPowerUp(devObj);
	// Wait a bit
	KeStallExecutionProcessor(1500);
	// Now, Orb is powered up and working
failed:
	DbgOut(("OrbInitComPort(): exit, status %x\n", status));

	return status;
}

// Power up the Orb
NTSTATUS
OrbPowerUp(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(("OrbPowerUp(): enter\n"));
	// Phase 1
	// Clear DTR
	status = OrbSerClrDtr(devObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): clear DTR failed\n"));
		goto failed;
	}
	// Clear RTS
	status = OrbSerClrRts(devObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): clear RTS failed\n"));
		goto failed;
	}
	// Delay 200ms
	KeStallExecutionProcessor(200);
	// Phase 2
	// Set DTR
	status = OrbSerSetDtr(devObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): set DTR failed\n"));
		goto failed;
	}
	// Wait a bit
	KeStallExecutionProcessor(200);
	// Set RTS
	status = OrbSerSetRts(devObj);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): set RTS failed\n"));
		goto failed;
	}
	// Wait a bit
	KeStallExecutionProcessor(200);
failed:
	DbgOut(("OrbPowerUp(): exit, status %x\n", status));

	return status;
}

// Power down the Orb
// pretty simple one
NTSTATUS
OrbPowerDown(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(("OrbPowerDown(): enter\n"));
	// Set DTR
	status = OrbSerSetDtr(devObj);
	// Clear RTS
	status = OrbSerClrRts(devObj);
	// Flush buffers
	status = OrbSerFlush(devObj,
			SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	DbgOut(("OrbPowerDown(): exit, status %x\n", status));

	return status;
}

// Detect ORB
NTSTATUS
OrbDetect(IN PDEVICE_OBJECT devObj)
{
	CHAR buffer[256], c;
	ULONG nRead, i;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	SERIAL_TIMEOUTS timeouts;
	PIRP Irp;

	DbgOut(("OrbDetect(): enter\n"));
	// Allocate Irp
	Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
	// Fail if couldn't allocate
	if (Irp == NULL) {
		DbgOut(("OrbDetect(): cant alloc Irp\n"));
		goto failed;
	}
	timeouts.ReadIntervalTimeout = MAXULONG;
	timeouts.ReadTotalTimeoutMultiplier = MAXULONG;
	timeouts.ReadTotalTimeoutConstant = 200;
	status = OrbSerSetTimeouts(devObj, &timeouts);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbDetect(): failed timeout\n"));
		goto failed;
	}
	// Write something
	status = OrbSerWriteChar(devObj, Irp, '\0d');
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbDetect(): cant write\n"));
		goto failed;
	}
	// Set up first read call
	nRead = 0;
	status = OrbSerReadChar(devObj, Irp, &buffer[nRead]);
	// Loop while we're reading something
	while (NT_SUCCESS(status) && (nRead < 253)) {
		buffer[nRead] &= 0x7f;
		nRead++;
		// Read next character
		status = OrbSerReadChar(devObj, Irp, &buffer[nRead]);
	}
	DbgOut(("OrbDetect(): read %d bytes, status %x\n", nRead, status));
	// Print buffer contents
	DbgOut(("OrbDetect(): Buffer:\n"));
	// Print buffer for debugging purposes
	for (i = 0; i < nRead; i++) {
		c = buffer[i];
		if ((c >= ' ') && (c <= 126))
			DbgOut(("%c\n", c));
	}
	DbgOut(("\OrbDetect(): Buffer end\n"));
	// Free Irp
	IoFreeIrp(Irp);
failed:
	DbgOut(("OrbDetect(): exit, status %x\n", status));

	return status;
}
