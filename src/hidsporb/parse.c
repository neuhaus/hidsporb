//
// parse.c
//
// SpaceOrb generic packet parser engine
//

#include "hidsporb.h"

// Read packet
NTSTATUS
OrbReadPacket(IN PORB_DATA orbData, IN PDEVICE_OBJECT serObj, IN PIRP readIrp)
{
	NTSTATUS status;
	UCHAR c;

	// Read 1 character from port
	status = OrbSerReadChar(serObj, readIrp, &c);
	if (NT_SUCCESS(status)) {
		// Parse character
		(*orbData->parseCharFunc)(orbData, c);
	}

	return status;
}

// Clear buffer and set packet type
VOID
OrbClearBuffer(IN PORB_DATA orbData, IN ULONG packetType)
{
	orbData->bufferCursor = 0;
	orbData->currPacketType = packetType;
}

// Call packet parsing function
VOID
OrbParseCallFunc(IN PORB_DATA orbData)
{
	// Call packet parsing function
	(*orbData->parsePacketFunc)(orbData, orbData->parsePacketContext);
	// Clear buffer
	OrbClearBuffer(orbData, ORB_UNKNOWN_PACKET);
}

// Todo: move to debug.c?

PCHAR strAxis[ORB_NUM_AXES] = { "TX", "TY", "TZ", "RX", "RY", "RZ" };

VOID
OrbPrintAxesBtns(IN PORB_DATA orbData)
{
	ULONG i;

	DbgOut(ORB_DBG_PARSE, ("OrbPrintAxes(): "));
	for (i = 0; i < ORB_NUM_AXES; i++) {
		DbgOut(ORB_DBG_PARSE, ("%s %d ", strAxis[i], orbData->Axes[i]));
	}
	DbgOut(ORB_DBG_PARSE, ("btns %01d%01d%01d%01d%01d%01d%01d\n", (orbData->buttons[0] != 0),
				(orbData->buttons[1] != 0), (orbData->buttons[2] != 0),
				(orbData->buttons[3] != 0), (orbData->buttons[4] != 0),
				(orbData->buttons[5] != 0), (orbData->buttons[6] != 0)));
	DbgOut(ORB_DBG_PARSE, ("\n"));
}
