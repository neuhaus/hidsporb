//
// packet.h
//
// Packet parser engine header file
//

#define	ORB_UNKNOWN_PACKET	0
#define	ORB_RESET_PACKET	1
#define	ORB_BALL_DATA_PACKET	2
#define	ORB_BUTTON_DATA_PACKET	3
#define	ORB_ERROR_PACKET	4
#define	ORB_NULL_REGION_PACKET	5
#define	ORB_TERM_PACKET		6
#define	ORB_MAX_PACKET_TYPE	6

#define	ORB_PACKET_BUFFER_LENGTH	60

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
	// Parse function & context
	// Context for packet parsing functions
	PVOID			parsePacketContext;
	PORB_PARSE_CHAR_FUNC	parseCharFunc;
	PORB_PARSE_PACKET_FUNC	parsePacketFunc;
	// ORB data
	LONG		Axes[ORB_NUM_AXES];			// Physical axes
	ULONG		buttons[ORB_NUM_PHYS_BUTTONS];		// Physical buttons
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
} ORB_DATA, *PORB_DATA;
