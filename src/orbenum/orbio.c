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

	DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): enter\n"));
	// Phase 1
	// Set wait mask to zero
	// Initialize port
	// Set up timeous
	RtlZeroMemory(&timeouts, sizeof(timeouts));
	// Set timeouts
	status = OrbSerSetTimeouts(devObj, &timeouts);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): failed to set timeouts\n"));
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
		DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): failed to set hand flow\n"));
		goto failed;
	}
	// Power down
	status = OrbPowerDown(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut( ORB_DBG_ORBIO, ("OrbInitComPort(): failed to power down\n"));
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
		DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): failed to set baud rate\n"));
		goto failed;
	}
	// Initialize line control
	// lineControl.WordLength = 8;
	// lineControl.Parity = NO_PARITY;
	// lineControl.StopBits = STOP_BIT_1;
#define SERIAL_8_DATA       ((UCHAR)0x08)
#define SERIAL_1_STOP       ((UCHAR)0x00)
#define SERIAL_NONE_PARITY  ((UCHAR)0x00)

	lineControl.WordLength = SERIAL_8_DATA;
	lineControl.Parity = SERIAL_1_STOP;
	lineControl.StopBits = SERIAL_NONE_PARITY;
	// Set word size, stop bits and parity stuff
	status = OrbSerSetLineControl(devObj, &lineControl);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): failed to set line control\n"));
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
	DbgOut(ORB_DBG_ORBIO, ("OrbInitComPort(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbPowerUp(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): enter\n"));
	// Phase 1
	// Clear DTR
	status = OrbSerClrDtr(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): clear DTR failed\n"));
		goto failed;
	}
	// Clear RTS
	status = OrbSerClrRts(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): clear RTS failed\n"));
		goto failed;
	}
	// Delay 200ms
	KeStallExecutionProcessor(200);
	// Phase 2
	// Set DTR
	status = OrbSerSetDtr(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): set DTR failed\n"));
		goto failed;
	}
	KeStallExecutionProcessor(200);
	// Set RTS
	status = OrbSerSetRts(devObj);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): set RTS failed\n"));
		goto failed;
	}
	// Wait a bit
	KeStallExecutionProcessor(200);
failed:
	DbgOut(ORB_DBG_ORBIO, ("OrbPowerUp(): exit, status %x\n", status));

	return status;
}

NTSTATUS
OrbPowerDown(IN PDEVICE_OBJECT devObj)
{
	NTSTATUS status;

	DbgOut(ORB_DBG_ORBIO, ("OrbPowerDown(): enter\n"));
	status = OrbSerSetDtr(devObj);
	status = OrbSerClrRts(devObj);
	status = OrbSerFlush(devObj,
			SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT | SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR);
	DbgOut(ORB_DBG_ORBIO, ("OrbPowerDown(): exit, status %x\n", status));

	return status;
}

ULONG
OrbReadData(IN PDEVICE_OBJECT devObj, PCHAR buffer, IN ULONG size)
{
	// CHAR buffer[256], c; changed vputz 20020506 so that buffer can be
	// passed in as argument
	CHAR c;
	ULONG nRead = 0, i;
	NTSTATUS status;
	SERIAL_TIMEOUTS timeouts;
	PIRP Irp;

	DbgOut(ORB_DBG_ORBIO, ("OrbReadData(): enter, buf %p len %d\n", buffer, size));
	timeouts.ReadIntervalTimeout = MAXULONG;
	timeouts.ReadTotalTimeoutMultiplier = MAXULONG;
	timeouts.ReadTotalTimeoutConstant = 200;
	// Set timeouts
	status = OrbSerSetTimeouts(devObj, &timeouts);
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbReadData(): failed timeouts\n"));
	}
	// Allocate read Irp
	Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
	// Fail if no Irp
	if (Irp == NULL) {
		DbgOut(ORB_DBG_ORBIO, ("OrbReadData(): no Irp\n"));
		goto failed;
	}
	// Write something
	status = OrbSerWriteChar(devObj, Irp, '\0d');
	// Fail if we couldn't write
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_ORBIO, ("OrbReadData(): cant write\n"));
		goto failed;
	}
	// Set up first read call
	nRead = 0;
	status = OrbSerReadChar(devObj, Irp, &buffer[nRead]);
	// While we're reading something
	while (NT_SUCCESS(status) && (nRead < size)) {
		// Save character
		buffer[nRead] &= 0x7f;
		nRead++;
		// Read next character
		status = OrbSerReadChar(devObj, Irp, &buffer[nRead]);
	}
	DbgOut(ORB_DBG_ORBIO, ("OrbReadDate(): read %d bytes, status %x\n", nRead, status));
	// Print buffer contents
	// NOTE: only for exteme debugging
#ifdef	NOT_YET
	DbgOut(ORB_DBG_ORBIO, ("OrbReadSomething(): Buffer:\n"));
	for (i = 0; i < nRead; i++) {
		c = buffer[i] & 0x7f;
		if ((c >= ' ') && (c <= 126)) {
			DbgOut(ORB_DBG_ORBIO, ("%c\n", c));
		}
	}
	DbgOut( ORB_DBG_ORBIO, ("\nOrbReadSomething(): Buffer end\n"));
#endif
failed:
	// free read Irp
	if (Irp) {
		IoFreeIrp(Irp);
	}
	DbgOut(ORB_DBG_ORBIO, ("OrbReadSomething(): exit\n"));

	return nRead;
}
