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
USHORT BallPacketLen[] = {
	0, 	// XXX
	15,     // displacement packet
	3,      // button/key packet
        3,      // 4k advanced button/key packet
        3,      // communications mode packet (not processed)
        3,      // sensitization mode packet (not processed)
        4,      // movement mode packet (not processed)
        2,      // null region packet (not processed)
        5,      // update rate packet (not processed)
        61,     // reset packet
        7,      // error packet
        13,     // zero packet (not processed)
};

// Forward declaration of several functions internal to this 
// unit

VOID
SBallParsePacket(IN PORB_DATA orbData);

VOID
SBallParseDisplacementPacket(IN PORB_DATA orbData);

VOID
SBallParseButtonKeyPacket(IN PORB_DATA orbData);

VOID 
SBallParseAdvancedButtonKeyPacket( IN PORB_DATA orbData );

VOID
SBallParseResetPacket( IN PORB_DATA orbData );

VOID
SBallParseErrorPacket(IN PORB_DATA orbData);

VOID
SBallParseNullRegionPacket(IN PORB_DATA orbData);

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
	orbData->orbModel = 1;
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
  ULONG bytes_written = 0;
  PIRP write_irp = NULL;
  NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
  //Orb reset string:
  //CB\r : binary comm mode
  //FT@, FR@ : set linear response (hidsporb will handle scaling!)
  //p@r@r : set new pulse timing (to what, I'm not sure yet)
  //MSS\r : set comm mode to send displacement packets
  char Orb_reset_string[] = "CB\rFT@\rFR@\rP@r@r\rMSS\r";
  // Initialize port
  status = OrbInitComPort(serObj);
  // fail if we coudn't do it
  if (!NT_SUCCESS(status)) {
    DbgOut(ORB_DBG_SBALL, ("SBallInit(): COM port init failed, status %x\n", status));
    goto failed;
  }
  //okay, we initialized the comm port; now we must set the
  //ball into "MSS" mode
  //allocate an Irp
  write_irp = IoAllocateIrp( serObj->StackSize + 1, FALSE );
  if ( write_irp == NULL )
    {
      DbgOut( ORB_DBG_SBALL, ( "SBallInit(): cannot allocate write IRP" ));
      //we go to failed here, but we never reset "status"... a problem?
      goto failed;
    }

  status = OrbSerWrite( serObj, write_irp, Orb_reset_string, 
			sizeof( Orb_reset_string ), &bytes_written );
  
  if (!NT_SUCCESS( status ))
    {
      DbgOut( ORB_DBG_SBALL, ("SBallInit(): Could not write mode sequence"));
      goto failed;
    }
	
  // Set up callback routines
  orbData->parseCharFunc = SBallParseChar;
  orbData->parsePacketFunc = parsePacketFunc;
  orbData->parsePacketContext = parseContext;
  orbData->in_escape_mode = 0;
  status = STATUS_SUCCESS;
 failed:
  if (write_irp) 
    {
      IoFreeIrp( write_irp );
    }
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
	DbgOut( ORB_DBG_SBALL, ( "SBallParsing 0x%x", c ));

	//first handle escape mode stuff so we don't have to worry about that
	//in the actual parsing.  If we are in escape mode, modify this
	//character and get out.  If we're not in escape mode and we get
	//an escape character ("^") then set the escape flag and just
	//eat the character.
	if ( orbData->in_escape_mode == 1 )
	  {
	    orbData->in_escape_mode = 0;
	    switch (c) 
	      {
	      case '^':
		break;
		
	      case 'Q':
	      case 'S':
	      case 'M':
		c &= 0x1f; //convert this character to unescaped form
	      }
	  }
	else if ( c == '^' )
	  {
	    orbData->in_escape_mode = 1;
	    return;
	  }

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
	  /* 	case '@': */ //disabled for now since we *send* reset packets
/* 	  OrbClearBuffer(orbData, BALL_RESET_PACKET); */
/* 	  break; */
	case 'E':
	  OrbClearBuffer(orbData, BALL_ERROR_PACKET);
	  break;
	case 'N':
	  OrbClearBuffer(orbData, BALL_NULL_REGION_PACKET);
	//eat all of the following packet types
	case 'Z': //zero packet
	case 'C': //comm mode packet
	case 'F': //sensitization packet
	case 'M': //movement mode packet
	case 'P': //update rate packet
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
/* 	DbgOut( ORB_DBG_SBALL,  */
/* 		("SballParseChar; cursor=%d, current packet=%d",  */
/* 		 orbData->bufferCursor, orbData->currPacketType )); */
	if (orbData->bufferCursor == BallPacketLen[orbData->currPacketType]) {
		// Parse packet
		SBallParsePacket(orbData);
	}
}


static VOID (*SBallFuncs[])(IN PORB_DATA orbData) = {
	NULL, // XXX todo
	SBallParseDisplacementPacket,
	SBallParseButtonKeyPacket,
	NULL, //SBallParseAdvancedButtonKeyPacket,
	NULL, //comm packet
	NULL, //comm packet
	NULL, //sensitization packet
	NULL, //movement packet
	NULL, //SBallParseNullRegionPacket, //null region packet
	NULL, //update rate packet
	NULL, //SBallParseResetPacket,
	NULL, //SBallParseErrorPacket,
	NULL, //zero packet
	NULL, //SBallParseTerm
};

PCHAR ballPktStr[] = {
	"unknown",
	"displacement",
	"button/key",
	"advanced button/key",
	"comm",
	"sensitization",
	"movement",
	"null",
	"update rate",
	"reset",
	"error",
	"zero",
	"term"
};

// Parse packet
VOID
SBallParsePacket(IN PORB_DATA orbData)
{
	ULONG pktType;

	// Determine packet type
	pktType = orbData->currPacketType;
	if (pktType < (sizeof(ballPktStr)/sizeof(ballPktStr[0]))) {
		DbgOut(ORB_DBG_SBALL, ("SBallParsePacket(): %s packet, len %d", ballPktStr[pktType], orbData->bufferCursor));
	}
	// Update packets stat
	orbData->numPackets[orbData->currPacketType]++;
	// Call appropriate function
	if ( SBallFuncs[pktType] != NULL )
	  {
	    (*SBallFuncs[pktType])(orbData);
	  }
	else
	  {
	    DbgOut( ORB_DBG_SBALL, 
		    ( "SballParsePacket(): HANDLER NOT AVAILABLE for packet type %d", pktType ));
	  }
}


VOID
SBallParseDisplacementPacket( IN PORB_DATA orbData )
{
  unsigned int tx, ty, tz, rx, ry, rz;
  PUCHAR pch;  

  pch = orbData->packetBuffer;
#define DISP_HELPER( p, i ) ((p[i] << 8) | (p[i+1]))
  tx = DISP_HELPER( pch, 3 );
  ty = DISP_HELPER( pch, 5 );
  tz = DISP_HELPER( pch, 7 );
  rx = DISP_HELPER( pch, 9 );
  ry = DISP_HELPER( pch, 11 );
  rz = DISP_HELPER( pch, 13 );
  //we scale this down by shifting right 5, 
  //which is not quite right.  The orb 360 returned
  //values from -512 to +512, while the balls have 16-bit ranges.
  //It will be clipped
  //by the translation-to-game-axes routine anyway.
#define DISP_HELPER2( val ) ( ((((int) val) << 16 ) >> 16 ) >> 5 )
  OrbDataSetPhysicalAxes( orbData, 
			  DISP_HELPER2( tx ),
			  DISP_HELPER2( ty ),
			  DISP_HELPER2( tz ),
			  DISP_HELPER2( rx ),
			  DISP_HELPER2( ry ),
			  DISP_HELPER2( rz ) );
  DbgPrintAxes( ORB_DBG_SBALL, orbData );
  //call our callback
  OrbParseCallFunc( orbData );
}


#define SELECT_BIT( b, i ) ( (b & ( 1 << i ) ) != 0 )

// Parse button packet
VOID
SBallParseButtonKeyPacket(IN PORB_DATA orbData)
{
	UCHAR buttons;
	PUCHAR pch;

	pch = orbData->packetBuffer;
/* 	OrbDataSetPhysicalButtons8( orbData, */
/* 				    SELECT_BIT( pch[ 1 ], 6 ), */
/* 				    SELECT_BIT( pch[ 1 ], 7 ), */
/* 				    SELECT_BIT( pch[ 2 ], 2 ), */
/* 				    SELECT_BIT( pch[ 2 ], 3 ), */
/* 				    SELECT_BIT( pch[ 1 ], 0 ), */
/* 				    SELECT_BIT( pch[ 1 ], 1 ), */
/* 				    SELECT_BIT( pch[ 1 ], 2 ), */
/* 				    SELECT_BIT( pch[ 1 ], 4 ) ); */
	//for some reason the 3003 and 4000 differ in the interpretation
	//of these bytes; adjustments to the above will probably have to
	//wait until I get my hands on a 4000
	OrbDataSetPhysicalButtons8( orbData,
				    SELECT_BIT( pch[ 2 ], 5 ),
				    SELECT_BIT( pch[ 2 ], 4 ),
				    0,
				    0,
				    0,
				    0,
				    0,
				    0 );
	DbgOut( ORB_DBG_SBALL, 
		("Buttons: (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", 
		 orbData->buttons[ 0 ],
		 orbData->buttons[ 1 ],
		 orbData->buttons[ 2 ],
		 orbData->buttons[ 3 ],
		 orbData->buttons[ 4 ],
		 orbData->buttons[ 5 ],
		 orbData->buttons[ 6 ],
		 orbData->buttons[ 7 ],
		 orbData->buttons[ 8 ],
		 orbData->buttons[ 9 ],
		 orbData->buttons[ 10 ],
		 orbData->buttons[ 11 ] ));

	// Call callback function
	OrbParseCallFunc(orbData);
}

void
SBallParseAdvancedButtonKeyPacket( IN PORB_DATA orbData )
{
  PUCHAR pch;
  int is_lefty;

  //TODO: if we got this packet and the orb is not flagged as a 4000,
  //then it must be a 4000; set some flag indicating this
  pch = orbData->packetBuffer;
  is_lefty = ((pch[ 1 ] & 0x20) != 0);
  DbgOut( ORB_DBG_SBALL, ("Lefty mode set: 0x%x", is_lefty));
  
  //how to handle lefty mode?  For now, we will **invert** the buttons
  if ( is_lefty )
    {
      OrbDataSetPhysicalButtons12( orbData,
				   SELECT_BIT( pch[ 1 ], 4 ),
				   SELECT_BIT( pch[ 1 ], 3 ),
				   SELECT_BIT( pch[ 1 ], 2 ),
				   SELECT_BIT( pch[ 1 ], 1 ),
				   SELECT_BIT( pch[ 1 ], 0 ),
				   SELECT_BIT( pch[ 2 ], 7 ),
				   SELECT_BIT( pch[ 2 ], 5 ),
				   SELECT_BIT( pch[ 2 ], 4 ),
				   SELECT_BIT( pch[ 2 ], 3 ),
				   SELECT_BIT( pch[ 2 ], 2 ),
				   SELECT_BIT( pch[ 1 ], 7 ),
				   SELECT_BIT( pch[ 1 ], 6 ) );
    }
  else
    {
      OrbDataSetPhysicalButtons12( orbData,
				   SELECT_BIT( pch[ 1 ], 6 ),
				   SELECT_BIT( pch[ 1 ], 7 ),
				   SELECT_BIT( pch[ 2 ], 2 ),
				   SELECT_BIT( pch[ 2 ], 3 ),
				   SELECT_BIT( pch[ 2 ], 4 ),
				   SELECT_BIT( pch[ 2 ], 5 ),
				   SELECT_BIT( pch[ 2 ], 7 ),
				   SELECT_BIT( pch[ 1 ], 0 ),
				   SELECT_BIT( pch[ 1 ], 1 ),
				   SELECT_BIT( pch[ 1 ], 2 ),
				   SELECT_BIT( pch[ 1 ], 3 ),
				   SELECT_BIT( pch[ 1 ], 4 ) );
	DbgOut( ORB_DBG_SBALL, 
		("Buttons: (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", 
		 orbData->buttons[ 0 ],
		 orbData->buttons[ 1 ],
		 orbData->buttons[ 2 ],
		 orbData->buttons[ 3 ],
		 orbData->buttons[ 4 ],
		 orbData->buttons[ 5 ],
		 orbData->buttons[ 6 ],
		 orbData->buttons[ 7 ],
		 orbData->buttons[ 8 ],
		 orbData->buttons[ 9 ],
		 orbData->buttons[ 10 ],
		 orbData->buttons[ 11 ] ));
    }
}



// Simple
VOID
SBallParseResetPacket(IN PORB_DATA orbData)
{
  DbgOut( ORB_DBG_SBALL, ("parsing reset packet" ) );
  // Call callback function
  OrbParseCallFunc(orbData);
}


// Parse error packet
// todo: maybe we reset orb here???
VOID
SBallParseErrorPacket(IN PORB_DATA orbData)
{
  PUCHAR pch;

  pch = orbData->packetBuffer;
  // Print error info
#if 0
  DbgOut( ORB_DBG_SBALL, ("SBallParseError(): \nE1: %d\nE2: %d\nE3: %d\nE4: %d\nE5: %d\nE6: %d\nE7: %d\nE8: %d\nE9: %d\nE10: %d",
			  SELECT_BIT( pch[ 1 ], 6 ),
			  SELECT_BIT( pch[ 1 ], 7 ),
			  SELECT_BIT( pch[ 2 ], 2 ),
			  SELECT_BIT( pch[ 2 ], 3 ),
			  SELECT_BIT( pch[ 2 ], 4 ),
			  SELECT_BIT( pch[ 2 ], 5 ),
			  0,
			  SELECT_BIT( pch[ 1 ], 0 ),
			  SELECT_BIT( pch[ 1 ], 1 ),
			  SELECT_BIT( pch[ 1 ], 2 ),
			  ));
#endif
  
  // Call appropriate function
  OrbParseCallFunc(orbData);
}

// Parse null region packet
VOID
SBallParseNullRegionPacket(IN PORB_DATA orbData)
{ 
  PUCHAR pch;

  pch = orbData->packetBuffer;
      
  DbgOut( ORB_DBG_SBALL, ("SBallParseNullRegionPacket"));
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
