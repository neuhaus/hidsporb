//
// parse.c
//
// SpaceOrb packet parser
//

#include "hidsporb.h"

// Read packet
NTSTATUS
OrbReadPacket(IN PDEVICE_EXTENSION devExt)
{
	PDEVICE_OBJECT serObj;
	NTSTATUS status;
	UCHAR c;

	serObj = devExt->nextDevObj;
	// Read 1 character from port
	status = OrbSerReadChar(serObj, devExt->readIrp, &c);
	if (NT_SUCCESS(status)) {
		// Parse character
		OrbParseChar(devExt, c);
	}

	return status;
}

// Packet length stuff
USHORT OrbPacketLen[] = {
	0, 	// XXX
	51,	// Reset
	12,	// Ball data
	5,	// Button data
	4,	// Error
	3,	// Null region
	1	// Term
};

// Clear buffer and set packet type
VOID
OrbClearBuffer(IN PDEVICE_EXTENSION devExt, IN ULONG packetType)
{
	devExt->bufferCursor = 0;
	devExt->currPacketType = packetType;
}

// Parse character. Parse a packet if complete
VOID
OrbParseChar(IN PDEVICE_EXTENSION devExt, IN UCHAR c)
{
	// Clear buffer if full
	if (devExt->bufferCursor >= (ORB_PACKET_BUFFER_LENGTH - 1)) {
		OrbClearBuffer(devExt, ORB_UNKNOWN_PACKET);
	}
	// Initialize packet type according to current char
	switch (c) {
	case 'R':
		OrbClearBuffer(devExt, ORB_RESET_PACKET);
		break;
	case 'D':
		OrbClearBuffer(devExt, ORB_BALL_DATA_PACKET);
		break;
	case 'K':
		OrbClearBuffer(devExt, ORB_BUTTON_DATA_PACKET);
		break;
	case 'E':
		OrbClearBuffer(devExt, ORB_ERROR_PACKET);
		break;
	case 'N':
		OrbClearBuffer(devExt, ORB_NULL_REGION_PACKET);
		break;
	case '\0x0d':
		OrbClearBuffer(devExt, ORB_TERM_PACKET);
		break;
	}
	// We don't care about unknown packets
	if (devExt->currPacketType == ORB_UNKNOWN_PACKET) {
		return;
	}
	// Store character in buffer
	devExt->packetBuffer[devExt->bufferCursor] = c;
	devExt->bufferCursor++;
	// See if packet is complete
	if (devExt->bufferCursor == OrbPacketLen[devExt->currPacketType]) {
		// Parse packet
		OrbParsePacket(devExt, devExt->packetBuffer, devExt->bufferCursor);
	}
}

static VOID (*OrbFuncs[])(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length) = {
	NULL, // XXX todo
	OrbParseReset,
	OrbParseBallData,
	OrbParseButtonData,
	OrbParseError,
	OrbParseNullRegion,
	OrbParseTerm
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
OrbParsePacket(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	USHORT pktType;

	// Determine packet type
	pktType = devExt->currPacketType;
	if (pktType < (sizeof(pktStr)/sizeof(pktStr[0]))) {
		DbgOut(("OrbParsePacket(): %s packet, len %d", pktStr[pktType], length));
	}
	// Call appropriate function
	(*OrbFuncs[pktType])(devExt, buffer, length);
}

// Simple
VOID
OrbParseReset(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	OrbClearBuffer(devExt, ORB_UNKNOWN_PACKET);
}

VOID
OrbXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length)
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

// Todo: move to debug.c?

PCHAR strAxis[ORB_NUM_AXES] = { "TX", "TY", "TZ", "RX", "RY", "RZ" };

VOID
OrbPrintAxes(IN PDEVICE_EXTENSION devExt)
{
	ULONG i;

	DbgOut(("OrbPrintAxes(): "));
	for (i = 0; i < ORB_NUM_AXES; i++) {
		DbgOut(("%s %d ", strAxis[i], devExt->Axes[i]));
	}
	DbgOut(("\n"));
}

VOID
OrbParseBallData(IN PDEVICE_EXTENSION devExt, IN PCHAR pch, IN USHORT length)
{
	LONG tx, ty, tz, rx, ry, rz;

	// Xor packet
	OrbXorPacket(pch, ORB_BALL_DATA_PACKET, length);
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
	devExt->Axes[0] = ((((long) tx) << 22) >> 22 ); 
	devExt->Axes[1] = ((((long) ty) << 22) >> 22 );
	devExt->Axes[2] = ((((long) tz) << 22) >> 22 );
	devExt->Axes[3] = ((((long) rx) << 22) >> 22 ); 
	devExt->Axes[4] = ((((long) ry) << 22) >> 22 );
	devExt->Axes[5] = ((((long) rz) << 22) >> 22 );
	// Debug: print Axes
	OrbPrintAxes(devExt);
	// Get and complete queued packet
	OrbCompletePacket(devExt);
}

VOID
OrbParseButtonData(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	UCHAR buttons;

	buttons = buffer[2];
	DbgOut(("OrbParseButtonData(): buttons %x", (ULONG) buttons));
	devExt->buttons[0] = buttons & 0x01;
	devExt->buttons[1] = buttons & 0x02;
	devExt->buttons[2] = buttons & 0x04;
	devExt->buttons[3] = buttons & 0x08;
	devExt->buttons[4] = buttons & 0x10;
	devExt->buttons[5] = buttons & 0x20;
	devExt->buttons[6] = buttons & 0x40;
	OrbCompletePacket(devExt);
}

VOID
OrbParseError(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	OrbCompletePacket(devExt);
}

VOID
OrbParseNullRegion(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	OrbCompletePacket(devExt);
}

VOID
OrbParseTerm(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length)
{
	OrbCompletePacket(devExt);
}
