//
// report.h
// SpaceOrb report descriptor and reports
//

//[Un]define these to enable additional reports
#define	ORB_ENABLE_MOUSE_REPORT		1
#define	ORB_ENABLE_KEYBOARD_REPORT	1

#if	ORB_ENABLE_MOUSE_REPORT
#define	ORB_MOUSE_REPDESC_SIZE	52
#else
#define	ORB_MOUSE_REPDESC_SIZE	0
#endif
#if	ORB_ENABLE_KEYBOARD_REPORT
#define	ORB_KEYBOARD_REPDESC_SIZE	34
#else
#define	ORB_KEYBOARD_REPDESC_SIZE	0
#endif

// NOTE: keep these in sync with real report descriptors!!!
//#define	ORB_FEATURE_REPDESC_SIZE	171
#define	ORB_FEATURE_REPDESC_SIZE		(84*2)
//#define	ORB_FEATURE_REPDESC_SIZE	0
#define	ORB_TOTAL_REPDESC_SIZE		(61+ORB_MOUSE_REPDESC_SIZE+ORB_KEYBOARD_REPDESC_SIZE+ORB_FEATURE_REPDESC_SIZE)

#ifdef	_REPORT_

// Combo joy/mouse/keyboard report
unsigned char SpaceOrbReport[ORB_TOTAL_REPDESC_SIZE] = {
    // This is main joystick top-level collection
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x15, 0x00,                    // LOGICAL_MINIMUM (0)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //     REPORT_ID (1)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE (Z)
    0x09, 0x33,                    //     USAGE (Rx)
    0x09, 0x34,                    //     USAGE (Ry)
    0x09, 0x35,                    //     USAGE (Rz)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0x00, 0x04,              //     LOGICAL_MAXIMUM (1024)
    0x75, 0x20,                    //     REPORT_SIZE (32)
    0x95, 0x06,                    //     REPORT_COUNT (6)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,                    //   USAGE_MAXIMUM (Button 16)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x55, 0x00,                    //   UNIT_EXPONENT (0)
    0x65, 0x00,                    //   UNIT (None)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
#if 1
    // This is feature report
    // NOTE: it must be in the first top-level collection!
    0x85, 0x02,                    //   Report ID (2)
    0x06, 0x00, 0xff,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x09, 0x02,                    //   USAGE (Vendor Usage 2)
    0x09, 0x03,                    //   USAGE (Vendor Usage 3)
    0x09, 0x04,                    //   USAGE (Vendor Usage 4)
    0x09, 0x05,                    //   USAGE (Vendor Usage 5)
    0x09, 0x06,                    //   USAGE (Vendor Usage 6)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x07,                    //   USAGE (Vendor Usage 7)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x08,                    //   USAGE (Vendor Usage 8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x09,                    //   USAGE (Vendor Usage 9)
    0x09, 0x0a,                    //   USAGE (Vendor Usage 10)
    0x09, 0x0b,                    //   USAGE (Vendor Usage 11)
    0x09, 0x0c,                    //   USAGE (Vendor Usage 12)
    0x09, 0x0d,                    //   USAGE (Vendor Usage 13)
    0x09, 0x0e,                    //   USAGE (Vendor Usage 14)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x0f,                    //   USAGE (Vendor Usage 15)
    0x09, 0x10,                    //   USAGE (Vendor Usage 16)
    0x09, 0x11,                    //   USAGE (Vendor Usage 17)
    0x09, 0x12,                    //   USAGE (Vendor Usage 18)
    0x09, 0x13,                    //   USAGE (Vendor Usage 19)
    0x09, 0x14,                    //   USAGE (Vendor Usage 20)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x02,                    //   LOGICAL_MAXIMUM (2)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x19, 0x15,                    //   USAGE_MINIMUM (Vendor Usage 21)
    0x29, 0x1a,                    //   USAGE_MAXIMUM (Vendor Usage 26)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x64,                    //   LOGICAL_MAXIMUM (100)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1b,                    //   USAGE (Vendor Usage 27)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1c,                    //   USAGE (Vendor Usage 28)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x64,                    //   LOGICAL_MAXIMUM (100)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1d,                    //   USAGE (Vendor Usage 29)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x02,                    //   LOGICAL_MAXIMUM (2)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1e,                    //   USAGE (Vendor Usage 30)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x0f,                    //   LOGICAL_MAXIMUM (15)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x85, 0x03,                    //   REPORT_ID (3) /80
    0x09, 0x00,                    //   USAGE (Undefined)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x96, 0x00, 0x04,              //   REPORT_COUNT (1024)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
#endif
    0xc0			   // End Collection (Application) TLC?
#if ORB_ENABLE_MOUSE_REPORT
    // TLC 2, mouse top-level collection
    // Second report
    ,0x05, 0x01,	   	   //   USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x04,			   //   Report ID 4
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION (Physical)
    0xc0                           // END_COLLECTION (Application)
#endif
// Keyboard top-level collection
#if ORB_ENABLE_KEYBOARD_REPORT
        ,0x05,   0x01,       // Usage Page (Generic Desktop),
        0x09,   0x06,       // Usage (Keyboard),
        0xA1,   0x01,       // Collection (Application),
        0x85,   0x05,       // Report Id (5)
        0x05,   0x07,       // usage page key codes
        0x19,   0xe0,       // usage min left control
        0x29,   0xe7,       // usage max keyboard right gui
        0x75,   0x01,       // report size 1
        0x95,   0x08,       // report count 8
        0x81,   0x02,       // input (Variable)
        0x19,   0x00,       // usage min 0
        0x29,   0x91,       // usage max 91
        0x26,   0xff, 0x00, // logical max 0xff
        0x75,   0x08,       // report size 8
        0x95,   0x01,       // report count 1
        0x81,   0x00,       // Input (Data, Array),
        0xc0                // End Collection
#endif
#if 0
// Feature top-level collection
//    ,0x05, 0x01,                 //   USAGE_PAGE (Generic Desktop)
    ,0x06, 0xbc, 0xff,             //   USAGE_PAGE (Vendor 0xffbc)
    0x09, 0x88,                    //   USAGE (0x88)
    0xa1, 0x01,                    //   Collection (Application)
    0x85, 0x04,                    //   REPORT_ID (4)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x09, 0x02,                    //   USAGE (Vendor Usage 2)
    0x09, 0x03,                    //   USAGE (Vendor Usage 3)
    0x09, 0x04,                    //   USAGE (Vendor Usage 4)
    0x09, 0x05,                    //   USAGE (Vendor Usage 5)
    0x09, 0x06,                    //   USAGE (Vendor Usage 6)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x07,                    //   USAGE (Vendor Usage 7)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x08,                    //   USAGE (Vendor Usage 8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x09,                    //   USAGE (Vendor Usage 9)
    0x09, 0x0a,                    //   USAGE (Vendor Usage 10)
    0x09, 0x0b,                    //   USAGE (Vendor Usage 11)
    0x09, 0x0c,                    //   USAGE (Vendor Usage 12)
    0x09, 0x0d,                    //   USAGE (Vendor Usage 13)
    0x09, 0x0e,                    //   USAGE (Vendor Usage 14)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x0f,                    //   USAGE (Vendor Usage 15)
    0x09, 0x10,                    //   USAGE (Vendor Usage 16)
    0x09, 0x11,                    //   USAGE (Vendor Usage 17)
    0x09, 0x12,                    //   USAGE (Vendor Usage 18)
    0x09, 0x13,                    //   USAGE (Vendor Usage 19)
    0x09, 0x14,                    //   USAGE (Vendor Usage 20)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x02,                    //   LOGICAL_MAXIMUM (2)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x19, 0x15,                    //   USAGE_MINIMUM (Vendor Usage 21)
    0x29, 0x1a,                    //   USAGE_MAXIMUM (Vendor Usage 26)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x64,                    //   LOGICAL_MAXIMUM (100)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1b,                    //   USAGE (Vendor Usage 27)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x05,                    //   LOGICAL_MAXIMUM (5)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1c,                    //   USAGE (Vendor Usage 28)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x64,                    //   LOGICAL_MAXIMUM (100)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1d,                    //   USAGE (Vendor Usage 29)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x02,                    //   LOGICAL_MAXIMUM (2)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x1e,                    //   USAGE (Vendor Usage 30)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x0f,                    //   LOGICAL_MAXIMUM (15)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x96, 0x00, 0x04,              //   REPORT_COUNT (1024)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0xc0                           // END_COLLECTION
#endif
};

#endif

#include <pshpack1.h>

typedef struct _HIDSPORB_REPORT {
	char reportId;			// report ID 1 for joystick
	ULONG Axes[ORB_NUM_AXES];	// 0..1023 range
	USHORT buttonMap;		// 16 buttons
} HIDSPORB_REPORT, *PAHIDSPORB_REPORT;

typedef struct _HIDSPORB_REPORT UNALIGNED *PHIDSPORB_REPORT;

typedef struct _HIDSPORB_MOUSE_REPORT {
	char reportId;	// report ID 2 for mouse
	UCHAR buttons;	// Only low 3 bits are used
	CHAR Axes[2];	// -127 ... 127 range
} HIDSPORB_MOUSE_REPORT, *PAHIDSPORB_MOUSE_REPORT;

typedef struct _HIDSPORB_MOUSE_REPORT UNALIGNED *PHIDSPORB_MOUSE_REPORT;

#include <poppack.h>

#define	HIDSPORB_REPORT_SIZE	sizeof(HIDSPORB_REPORT)
