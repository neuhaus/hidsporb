#ifndef FEATURES_H
#define FEATURES_H

/*
  this module contains structures and definitions for feature reports.
  These are to be used in the driver itself and in applications
  communicating with the driver
*/


/*
  a HIDSPORB_FEATURE_DATA structure contains the bytes of data used in
  a hidsporb "feature report" (used by external user-mode programs to
  control the behaviour of the driver using HidD_SetFeature and 
  HidD_GetFeature.  The HIDSPORB_FEATURE_DATA structure is used inside
  the driver; its counterpart, the HIDSPORB_FEATURE_PACKET, is used
  by user-mode programs.
*/
#include <pshpack1.h>
typedef struct _HIDSPORB_FEATURE_DATA
{
  unsigned char axis_map[ 6 ];
  unsigned char use_chording;
  unsigned char null_region;
  unsigned char sensitivities[ 6 ];
  unsigned char polarities[ 6 ];
  unsigned char gains[ 6 ];
  unsigned char precision_sensitivity;
  unsigned char precision_gain;
  unsigned char precision_button_type;
  unsigned char precision_button_index;
} HIDSPORB_FEATURE_DATA, *PHIDSPORB_FEATURE_DATA;

#define HIDSPORB_FEATURE_DATA_SIZE (sizeof (HIDSPORB_FEATURE_DATA))
#include <poppack.h>


#include <pshpack1.h>
typedef struct _HIDSPORB_FEATURE_PACKET
{
  unsigned char report_id;
  HIDSPORB_FEATURE_DATA feature_data;
} HIDSPORB_FEATURE_PACKET, *PHIDSPORB_FEATURE_PACKET;

#define HIDSPORB_FEATURE_PACKET_SIZE (sizeof (HIDSPORB_FEATURE_PACKET))
#include <poppack.h>

#include <pshpack1.h>
typedef struct _HIDSPORB_SENSITIVITY_CURVE
{
  unsigned char curve_id;
  unsigned short curve[ 1024 ];
} HIDSPORB_SENSITIVITY_CURVE, *PHIDSPORB_SENSITIVITY_CURVE;

#define HIDSPORB_SENSITIVITY_CURVE_SIZE (sizeof (HIDSPORB_SENSITIVITY_CURVE) )
#include <poppack.h>

#include <pshpack1.h>
typedef struct _HIDSPORB_SENSITIVITY_CURVE_PACKET
{
  unsigned char report_id;
  HIDSPORB_SENSITIVITY_CURVE curve;
} HIDSPORB_SENSITIVITY_CURVE_PACKET, *PHIDSPORB_SENSITIVITY_CURVE_PACKET;
#include <poppack.h>

#define HIDSPORB_SENSITIVITY_CURVE_PACKET_SIZE (sizeof( HIDSPORB_SENSITIVITY_CURVE_PACKET ))

#define HIDSPORB_VERSION_NUMBER  ((USHORT) 1)
#define HIDSPORB_VENDOR_ID		0x5a8
#define HIDSPORB_PRODUCT_ID		0x360

#define HIDSPORB_FEATURE_PACKET_ID	2
#define HIDSPORB_CURVE_PACKET_ID	3
#define	HIDSPORB_FEATURE_MIN_ID		2
#define	HIDSPORB_FEATURE_MAX_ID		3

#endif
