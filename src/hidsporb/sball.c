//
// sorb.c
//
// SpaceOrb 360 detection/packet parser engine
//
#ifndef SORBINC_H
#include "sorbinc.h"
#endif
#ifndef BALLPACKET_H
#include "ballpacket.h"
#endif

// Packet length stuff
// Todo: add other packet types (query, etc)
USHORT OrbPacketLen[] = {
	0, 	// XXX
	15,     // displacement packet
	3,      // button/key packet
        3,      // 4k advanced button/key packet
        3,      // communications mode packet
        3,      // sensitization mode packet
        4,      // movement mode packet
        2,      // null region packet
        5,      // update rate packet
        61,     // reset packet
        7,      // error packet
        13,     // zero packet
};

// Forward declaration of several functions internal to this 
// unit

VOID
SBallXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length);

VOID
SBallParsePacket(IN PORB_DATA orbData);

VOID
SBallParseReset(IN PORB_DATA orbData);

VOID
SBallParseBallData(IN PORB_DATA orbData);

VOID
SBallParseButtonData(IN PORB_DATA orbData);

VOID
SBallParseError(IN PORB_DATA orbData);

VOID
SBallParseNullRegion(IN PORB_DATA orbData);

VOID
SBallParseTerm(IN PORB_DATA orbData);



// This function tries to detect ORB and must return STATUS_SUCCESS
// if SpaceORB is detected, or STATUS_NO_SUCH_DEVICE if not.
NTSTATUS
SBallDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp)
{
	// for now, we don't do any detection
	// will write this later
	// Set model number
	// OrbInit/Cleanup() expect these fields to be set by Detect()
	orbData->orbModel = 0;
	orbData->orbDetected = TRUE;

	return STATUS_SUCCESS;
}

// This function tries to initialize SpaceBall 2003/3003/4000 series;
// rules are the same as with SBallDetect().
NTSTATUS
SBallInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData,
	 IN PIRP initIrp, PORB_PARSE_PACKET_FUNC parsePacketFunc,
	 IN PVOID parseContext)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	// Initialize port
	status = OrbInitComPort(serObj);
	// fail if we coudn't do it
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_IO, ("SBallInit(): COM port init failed, status %x\n", status));
		goto failed;
	}
	// Set up callback routines
	orbData->parseCharFunc = SBallParseChar;
	orbData->parsePacketFunc = parsePacketFunc;
	orbData->parsePacketContext = parseContext;
	status = STATUS_SUCCESS;
failed:
	DbgOut(ORB_DBG_DETECT | ORB_DBG_SBALL, ("SBallInit(): exit, status %x\n", status));

	return status;
}

// This function cleans up everything
NTSTATUS
SBallCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp)
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
SBallParseChar(IN PORB_DATA orbData, IN UCHAR c)
{
	// Clear buffer if full
	if (orbData->bufferCursor >= (ORB_PACKET_BUFFER_LENGTH - 1)) {
		OrbClearBuffer(orbData, ORB_UNKNOWN_PACKET);
	}
	DbgOut( ORB_DBG_SBALL, ( "Parsing 0x%x", c ));
	// Initialize packet type according to current char
	switch (c) {
	case 'D':
	  OrbClearBuffer(orbData, BALL_DISPLACEMENT_PACKET);
	  break;
	case 'K' :
	  OrbClearBuffer(orbData, BALL_BUTTON_KEY_PACKET);
	  break;
	case '.' :
	  OrbClearBuffer(orbData, BALL_ADVANCED_BUTTON_PACKET);
	  break;
	case '@':
	  OrbClearBuffer(orbData, BALL_RESET_PACKET);
	  break;
	case 'E':
	  OrbClearBuffer(orbData, BALL_ERROR_PACKET);
	  break;
	//eat all of the following packet types
	case 'Z': //zero packet
	case 'C': //comm mode packet
	case 'F': //sensitization packet
	case 'M': //movement mode packet
	case 'N': //null region packet
	case 'P': //update rate packet
	default:
	  OrbClearBuffer(orbData, ORB_UNKNOWN_PACKET);
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
		SBallParsePacket(orbData);
	}
}

// Uncrypt packet contents
VOID
SBallXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length)
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

static VOID (*SBallFuncs[])(IN PORB_DATA orbData) = {
	NULL, // XXX todo
	SBallParseReset,
	SBallParseBallData,
	SBallParseButtonData,
	SBallParseError,
	SBallParseNullRegion,
	SBallParseTerm
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
SBallParsePacket(IN PORB_DATA orbData)
{
	USHORT pktType;

	// Determine packet type
	pktType = orbData->currPacketType;
	if (pktType < (sizeof(pktStr)/sizeof(pktStr[0]))) {
		DbgOut(ORB_DBG_SBALL, ("SBallParsePacket(): %s packet, len %d", pktStr[pktType], orbData->bufferCursor));
	}
	// Update packets stat
	orbData->numPackets[orbData->currPacketType]++;
	// Call appropriate function
	(*SBallFuncs[pktType])(orbData);
}

// Simple
VOID
SBallParseReset(IN PORB_DATA orbData)
{
        DbgOut( ORB_DBG_SBALL, ("parsing reset packet" ) );
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse ball data packet
VOID
SBallParseBallData(IN PORB_DATA orbData)
{
	LONG tx, ty, tz, rx, ry, rz;
	PUCHAR pch;

	pch = orbData->packetBuffer;
	// Xor packet
	SBallXorPacket(pch, ORB_BALL_DATA_PACKET, orbData->bufferCursor);
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
SBallParseButtonData(IN PORB_DATA orbData)
{
	UCHAR buttons;
	PUCHAR buffer;

	buffer = orbData->packetBuffer;
	// Get buttons
	buttons = buffer[2];
	DbgOut( ORB_DBG_SBALL, ("Buttons: (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", buffer[ 0 ], buffer[ 1 ], buffer[ 2 ], buffer[ 3 ], buffer[ 4 ], buffer[ 5 ] ));
	DbgOut(ORB_DBG_SBALL, ("SBallParseButtonData(): buttons %x", (ULONG) buttons));
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
SBallParseError(IN PORB_DATA orbData)
{
	BOOLEAN brownout, checksum, hardflt;
	PUCHAR buffer;

	buffer = orbData->packetBuffer;
	// Print error info
	hardflt = (buffer[1] & 1) != 0;
	checksum = (buffer[1] & 2) != 0;
	brownout = (buffer[1] & 4) != 0;
	DbgOut(ORB_DBG_SBALL, ("SBallParseError(): hardflt %d checksum %d brownout %d\n", hardflt, checksum, brownout));
	// Call appropriate function
	OrbParseCallFunc(orbData);
}

// Parse null region packet
VOID
SBallParseNullRegion(IN PORB_DATA orbData)
{       
        DbgOut( ORB_DBG_SBALL, ("Parsing null region packet" ));
	// Call callback function
	OrbParseCallFunc(orbData);
}

// Parse terminator packet
VOID
SBallParseTerm(IN PORB_DATA orbData)
{
  DbgOut( ORB_DBG_SBALL, ("Parsing Term packet")  );
	// Call callback function
	OrbParseCallFunc(orbData);
}
