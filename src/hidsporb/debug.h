#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG_BASE_H
#include "debug_base.h"
#endif

#if DBG
// PNP debugging
#define	ORB_DBG_PNP	(1 << 0)
// Serial I/O debugging
#define	ORB_DBG_SER	(1 << 1)
// I/O debugging
#define	ORB_DBG_IO	(1 << 2)
// Settings debugging
#define	ORB_DBG_SET	(1 << 3)
// Ioctl debugging
#define	ORB_DBG_IOCTL	(1 << 4)
// read_report debugging
#define	ORB_DBG_REPORT	(1 << 5)
// feature debugging
#define	ORB_DBG_FEATURE	(1 << 6)
// ORB parser debugging
#define	ORB_DBG_PARSE	(1 << 7)
// Detection code debugging
#define	ORB_DBG_DETECT	(1 << 8)
// IRP queue debugging
#define	ORB_DBG_IRPQ	(1 << 9)
//
#define	ORB_DBG_DISPATCH	(1 << 10)
// SpaceBall-specific parsing
#define ORB_DBG_SBALL   (1 << 11)
// SpaceOrb-specific parsing
#define ORB_DBG_SORB    (1 << 12)
// data parsing
#define ORB_DBG_DATA    (1 << 13)
//
#define	ORB_DBG_ALL	(ULONG) (~0L)

#define DbgPrintAxes( level, p_orb_data ) \
  DbgOut( level, ( "Orb Axes: tx: %d; ty: %d; tz: %d; rx: %d; ry: %d; rz: %d", \
		   p_orb_data->Axes[ 0 ], \
		   p_orb_data->Axes[ 1 ], \
		   p_orb_data->Axes[ 2 ], \
		   p_orb_data->Axes[ 3 ], \
		   p_orb_data->Axes[ 4 ], \
		   p_orb_data->Axes[ 5 ] ) )
#define DbgPrintButtons( level, p_orb_data ) \
  DbgOut( level, ( "Orb Buttons: %01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d", \
		   p_orb_data->buttons[ 0 ] != 0, \
		   p_orb_data->buttons[ 1 ] != 0, \
		   p_orb_data->buttons[ 2 ] != 0, \
		   p_orb_data->buttons[ 3 ] != 0, \
		   p_orb_data->buttons[ 4 ] != 0, \
		   p_orb_data->buttons[ 5 ] != 0, \
		   p_orb_data->buttons[ 6 ] != 0, \
		   p_orb_data->buttons[ 7 ] != 0, \
		   p_orb_data->buttons[ 8 ] != 0, \
		   p_orb_data->buttons[ 9 ] != 0, \
		   p_orb_data->buttons[ 10 ] != 0, \
		   p_orb_data->buttons[ 11 ] != 0 ) )


#else
#define DbgPrintAxes( level, p_orb_data )
#define DbgPrintButtons( level, p_orb_data )
#endif

#endif
