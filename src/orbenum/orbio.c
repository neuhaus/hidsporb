//
// orbio.c
//

#include "orbenum.h"

// This function initializes COM port and powers up Orb
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
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set hand flow\n"));
		goto failed;
	}
	// Power down
	status = OrbPowerDown(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to power down\n"));
		goto failed;
	}
	// Delay 200ms
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
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbInitComPort(): failed to set line control\n"));
		goto failed;
	}
	// Flush buffers
	status = OrbSerFlush(devObj,
			SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	// Go ahead and power up the orb
	status = OrbPowerUp(devObj);
	// Flush buffers
	//status = OrbSerFlush(devObj,
	//		SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	// Now, Orb is powered up and working
	KeStallExecutionProcessor(1500);
failed:
	DbgOut(("OrbInitComPort(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbPowerUp(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(("OrbPowerUp(): enter\n"));
	// Phase 1
	// Clear DTR
	status = OrbSerClrDtr(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): clear DTR failed\n"));
		goto failed;
	}
	// Clear RTS
	status = OrbSerClrRts(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): clear RTS failed\n"));
		goto failed;
	}
	// Delay 200ms
	KeStallExecutionProcessor(200);
	// Phase 2
	// Set DTR
	status = OrbSerSetDtr(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbPowerUp(): set DTR failed\n"));
		goto failed;
	}
	KeStallExecutionProcessor(200);
	// Set RTS
	status = OrbSerSetRts(devObj);
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

NTSTATUS
OrbPowerDown(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(("OrbPowerDown(): enter\n"));
	status = OrbSerSetDtr(devObj);
	status = OrbSerClrRts(devObj);
	status = OrbSerFlush(devObj,
			SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	DbgOut(("OrbPowerDown(): exit, status %x\n", status));

	return status;
}

VOID
OrbReadSomething(IN PDEVICE_OBJECT devObj)
{
	CHAR buffer[256], c;
	ULONG nRead, i;
	NTSTATUS status;
	SERIAL_TIMEOUTS timeouts;

	DbgOut(("OrbReadSomething(): enter\n"));
	timeouts.ReadIntervalTimeout = MAXULONG;
	timeouts.ReadTotalTimeoutMultiplier = MAXULONG;
	timeouts.ReadTotalTimeoutConstant = 200;
	status = OrbSerSetTimeouts(devObj, &timeouts);
	if (!NT_SUCCESS(status)) {
		//
		DbgOut(("OrbReadSomething(): failed timeout\n"));
	}
	// Write something
	status = OrbSerWriteChar(devObj, '\0d');
	if (!NT_SUCCESS(status)) {
		DbgOut(("OrbReadSomething(): cant write\n"));
		goto failed;
	}
	// Set up first read call
	nRead = 0;
	status = OrbSerReadChar(devObj, &buffer[nRead]);
	// While we're reading something
	while (NT_SUCCESS(status) && (nRead < 253)) {
		buffer[nRead] &= 0x7f;
		nRead++;
		status = OrbSerReadChar(devObj, &buffer[nRead]);
	}
	DbgOut(("OrbReadSomething(): read %d bytes, status %x\n", nRead, status));
	// Print buffer contents
	DbgOut(("OrbReadSomething(): Buffer:\n"));
	//buffer[nRead] = 0;
	for (i = 0; i < nRead; i++) {
		c = buffer[i] & 0x7f;
		if ((c >= ' ') && (c <= 126))
			DbgOut(("%c\n", c));
	}
	//DbgOut(("OrbReadSomething(): %s\n", &buffer[0]));
	DbgOut(("\nOrbReadSomething(): Buffer end\n"));
failed:
	DbgOut(("OrbReadSomething(): exit\n"));
}
