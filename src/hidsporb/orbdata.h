#ifndef ORBDATA_H
#define ORBDATA_H

#include <wdm.h>
#ifndef ORB_H
#include "orb.h"
#endif

#define	ORB_PACKET_BUFFER_LENGTH	60
#define	ORB_MAX_PACKET_TYPE	        11
#define ORB_UNKNOWN_PACKET              0


typedef struct _ORB_DATA *PORB_DATA;

typedef VOID (*PORB_PARSE_CHAR_FUNC)(IN PORB_DATA orbData, IN UCHAR c);
typedef VOID (*PORB_PARSE_PACKET_FUNC)(IN PORB_DATA orbData, IN PVOID context);

// SpaceOrb data
typedef struct _ORB_DATA {
	// Orb model
	ULONG		orbModel;
	// Has ORB been detected?
	BOOLEAN		orbDetected;
	// Packet stuff
	ULONG		currPacketType; 			// Current packet type
	CHAR		packetBuffer[ORB_PACKET_BUFFER_LENGTH]; // packet buffer
	USHORT		bufferCursor;				// Current position in buffer
	ULONG		numPackets[ORB_MAX_PACKET_TYPE];	// Packets stat data
        CHAR            in_escape_mode; //used in parsing 3003/4000 data
	// Parse function & context
	// Context for packet parsing functions
	PVOID			parsePacketContext;
	PORB_PARSE_CHAR_FUNC	parseCharFunc;
	PORB_PARSE_PACKET_FUNC	parsePacketFunc;
	// ORB data
	LONG		Axes[ORB_NUM_AXES];			// Physical axes
	ULONG		buttons[ORB_MAX_PHYS_BUTTONS];		// Physical buttons
	// logical mapping stuff
	// Fields for physical-to-logical control bindings
	// axis map--for each logical axis, an entry in this map contains the
	//index of the physical axis to use
	ULONG		AxisMap[ORB_NUM_AXES];		// Physical/Logical axis map
	// whether or not to use chording on this device.  If chording
	// is used, buttons A/B on the orb set up a context for the remaining
	// four buttons
	BOOLEAN		use_chording;
	// upcoming null region.  Note that the region is not actually
	// set in the orb until it's processed during orb_comm, thus the
	// two elements for "new_null_region_pending" and "null_region"
	//
	int		null_region;
  	// sensitivities -- determines the "response curve" used by each axis
	int sensitivities[ORB_NUM_AXES];
  	// polarities -- determines "which direction is positive" on each axis
	int polarities[ORB_NUM_AXES];
	// gains -- determines "amplification" of each axis
	int gains[ORB_NUM_AXES];
	// precision settings--first sensitivity (response curve when
	// precision button is pressed)
	int precision_sensitivity;
	// gain to use when button is pressed
	int precision_gain;
	// what button type -- logical or physical -- for precision
	int precision_button_type;
	// what button index for precision
	int precision_button_index;
	BOOLEAN new_null_region_pending;
	// Mutex protection
	FAST_MUTEX dataMutex;
} ORB_DATA, *PORB_DATA;

VOID
OrbInitData(IN PORB_DATA orbData);

// Locking support
VOID
OrbLockData(IN PORB_DATA orbData);

VOID
OrbUnlockData(IN PORB_DATA orbData);

void
OrbDataSetPhysicalAxis( PORB_DATA orb_data,
			int index, 
			LONG value );

void
OrbDataSetPhysicalAxes( PORB_DATA orb_data,
			LONG tx, 
			LONG ty, 
			LONG tz,
			LONG rx,
			LONG ry,
			LONG rz );

void
OrbDataSetPhysicalButton( PORB_DATA orb_data,
			  int index,
			  LONG value );

void
OrbDataSetPhysicalButtons7( PORB_DATA orb_data,
			    int button_0,
			    int button_1,
			    int button_2,
			    int button_3,
			    int button_4,
			    int button_5,
			    int button_6);

void
OrbDataSetPhysicalButtons8( PORB_DATA orb_data,
			    int button_0,
			    int button_1,
			    int button_2,
			    int button_3,
			    int button_4,
			    int button_5,
			    int button_6,
			    int button_7 );

void
OrbDataSetPhysicalButtons12( PORB_DATA orb_data,
			     int button_0,
			     int button_1,
			     int button_2,
			     int button_3,
			     int button_4,
			     int button_5,
			     int button_6,
			     int button_7,
			     int button_8, 
			     int button_9,
			     int button_10,
			     int button_11 );

#endif
