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
  DbgOut( ORB_DBG_PARSE, 
	  ( "OrbClearBuffer: Clearing to packet type %d", packetType ));
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

