//
// sorb.c
//
// SpaceOrb 360 detection/packet parser engine
//

#include "sorbinc.h"

// Packet length stuff
// Todo: add other packet types (query, etc)
USHORT OrbPacketLen[] = {
	0, 	// XXX
	51,	// Reset
	12,	// Ball data
	5,	// Button data
	4,	// Error
	3,	// Null region
	1	// Term
};

// This function tries to detect ORB and must return STATUS_SUCCESS
// if SpaceORB is detected, or STATUS_NO_SUCH_DEVICE if not.
NTSTATUS
SOrbDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp)
{
	// for now, we don't do any detection
	// will write this later
	// Set model number
	// OrbInit/Cleanup() expect these fields to be set by Detect()
	orbData->orbModel = 0;
	orbData->orbDetected = TRUE;

	return STATUS_SUCCESS;
}

// This function tries to initialize Space ORB, rules are the same as with
// SOrbDetect().
NTSTATUS
SOrbInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData,
	 IN PIRP initIrp, PORB_PARSE_PACKET_FUNC parsePacketFunc,
	 IN PVOID parseContext)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	// Initialize port
	status = OrbInitComPort(serObj);
	// fail if we coudn't do it
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_IO, ("SOrbInit(): COM port init failed, status %x\n", status));
		goto failed;
	}
	// Set up callback routines
	orbData->parseCharFunc = SOrbParseChar;
	orbData->parsePacketFunc = parsePacketFunc;
	orbData->parsePacketContext = parseContext;
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_DETECT | ORB_DBG_PARSE, ("SOrbInit(): exit, status %x\n", status));

	return status;
}

// This function cleans up everything
NTSTATUS
SOrbCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp)
{
	// Initialize fields
	orbData->parseCharFunc = NULL;
	orbData->parsePacketFunc = NULL;
	orbData->parsePacketContext = NULL;
	orbData->orbDetected = FALSE;
	orbData->orbModel = 0;

	// Power down
	return OrbPowerDown(serObj);
}

// Parse character. Parse packet if complete
VOID
SOrbParseChar(IN PORB_DATA orbData, IN UCHAR c)
{
	// Clear buffer if full
	if (orbData->bufferCursor >= (ORB_PACKET_BUFFER_LENGTH - 1)) {
		OrbClearBuffer(orbData, ORB_UNKNOWN_PACKET);
	}
	// Initialize packet type according to current char
	switch (c) {
	case 'R':
		OrbClearBuffer(orbData, ORB_RESET_PACKET);
		break;
	case 'D':
		OrbClearBuffer(orbData, ORB_BALL_DATA_PACKET);
		break;
	case 'K':
		OrbClearBuffer(orbData, ORB_BUTTON_DATA_PACKET);
		break;
	case 'E':
		OrbClearBuffer(orbData, ORB_ERROR_PACKET);
		break;
	case 'N':
		OrbClearBuffer(orbData, ORB_NULL_REGION_PACKET);
		break;
	case '\0x0d':
		OrbClearBuffer(orbData, ORB_TERM_PACKET);
		break;
	}
	// We don't care about unknown packets
	if (orbData->currPacketType == ORB_UNKNOWN_PACKET) {
		return;
	}
	// Store character in buffer
	orbData->packetBuffer[orbData->bufferCursor] = c;
	orbData->bufferCursor++;
	// See if packet is complete
	if (orbData->bufferCursor == OrbPacketLen[orbData->currPacketType]) {
		// Parse packet
		SOrbParsePacket(orbData);
	}
}

// Uncrypt packet contents
VOID
SOrbXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length)
{
	ULONG i;
	PCHAR strXor = "D.SpaceWare";

	if (length > OrbPacketLen[pktType]) {
		length = OrbPacketLen[pktType];
	}
	// Xor
	for (i = 0; i < length; i++) {
		buffer[i] ^= strXor[i];
	}
}

static VOID (*SOrbFuncs[])(IN PORB_DATA orbData) = {
	NULL, // XXX todo
	SOrbParseReset,
	SOrbParseBallData,
	SOrbParseButtonData,
	SOrbParseError,
	SOrbParseNullRegion,
	SOrbParseTerm
};

PCHAR pktStr[] = {
	"unknown",
	"reset",
	"balldata",
	"buttondata",
	"error",
	"nullregion",
	"term"
};

// Parse packet
VOID
SOrbParsePacket(IN PORB_DATA orbData)
{
	USHORT pktType;

	// Determine packet type
	pktType = orbData->currPacketType;
	if (pktType < (sizeof(pktStr)/sizeof(pktStr[0]))) {
		DbgOut(ORB_DBG_PARSE, ("SOrbParsePacket(): %s packet, len %d", pktStr[pktType], orbData->bufferCursor));
	}
	// Update packets stat
	orbData->numPackets[orbData->currPacketType]++;
	// Call appropriate function
	(*SOrbFuncs[pktType])(orbData);
}

// Simple
VOID
SOrbParseReset(IN PORB_DATA orbData)
{
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse ball data packet
VOID
SOrbParseBallData(IN PORB_DATA orbData)
{
	LONG tx, ty, tz, rx, ry, rz;
	PUCHAR pch;

	pch = orbData->packetBuffer;
	// Xor packet
	SOrbXorPacket(pch, ORB_BALL_DATA_PACKET, orbData->bufferCursor);
	// note that this starts with buffer[2], which means we're
	// ignoring the new copy of the button data for now
	tx = ((pch[2] & 0x7F) << 3) | ((pch[3] & 0x70) >> 4);
	ty = ((pch[3] & 0x0F) << 6) | ((pch[4] & 0x7E) >> 1);
	tz = ((pch[4] & 0x01) << 9) | ((pch[5] & 0x7F) << 2) | ((pch[6] & 0x60) >> 5);
	rx = ((pch[6] & 0x1F) << 5) | ((pch[7] & 0x7C) >> 2);
	ry = ((pch[7] & 0x03) << 8) | ((pch[8] & 0x7F) << 1) | ((pch[9] & 0x40) >> 6);
	rz = ((pch[9]  & 0x3F) << 4) | ((pch[10] & 0x78) >> 3);
	// set timer data?  removing this for now
	// handle->timer = ((pch[10] & 0x07) << 7) | (pch[11] & 0x7F);
	// Set axes
	orbData->Axes[0] = ((((long) tx) << 22) >> 22);
	orbData->Axes[1] = ((((long) ty) << 22) >> 22);
	orbData->Axes[2] = ((((long) tz) << 22) >> 22);
	orbData->Axes[3] = ((((long) rx) << 22) >> 22); 
	orbData->Axes[4] = ((((long) ry) << 22) >> 22);
	orbData->Axes[5] = ((((long) rz) << 22) >> 22);
	// Debug: print Axes
	OrbPrintAxesBtns(orbData);
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse button packet
VOID
SOrbParseButtonData(IN PORB_DATA orbData)
{
	UCHAR buttons;
	PUCHAR buffer;

	buffer = orbData->packetBuffer;
	// Get buttons
	buttons = buffer[2];
	DbgOut(ORB_DBG_PARSE, ("SOrbParseButtonData(): buttons %x", (ULONG) buttons));
	// Update buttons
	orbData->buttons[0] = buttons & 0x01;
	orbData->buttons[1] = buttons & 0x02;
	orbData->buttons[2] = buttons & 0x04;
	orbData->buttons[3] = buttons & 0x08;
	orbData->buttons[4] = buttons & 0x10;
	orbData->buttons[5] = buttons & 0x20;
	orbData->buttons[6] = buttons & 0x40;
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse error packet
// todo: maybe we reset orb here???
VOID
SOrbParseError(IN PORB_DATA orbData)
{
	BOOLEAN brownout, checksum, hardflt;
	PUCHAR buffer;

	buffer = orbData->packetBuffer;
	// Print error info
	hardflt = (buffer[1] & 1) != 0;
	checksum = (buffer[1] & 2) != 0;
	brownout = (buffer[1] & 4) != 0;
	DbgOut(ORB_DBG_PARSE, ("SOrbParseError(): hardflt %d checksum %d brownout %d\n", hardflt, checksum, brownout));
	// Call appropriate function
	OrbParseCallFunc(orbData);
}

// Parse null region packet
VOID
SOrbParseNullRegion(IN PORB_DATA orbData)
{
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse terminator packet
VOID
SOrbParseTerm(IN PORB_DATA orbData)
{
	// Call callback function
	OrbParseCallFunc(orbData);
}
