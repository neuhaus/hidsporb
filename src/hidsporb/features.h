#ifndef FEATURES_H
#define FEATURES_H

/*

Copyright (c) 2001, Victor B. Putz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.  

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of the project nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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
#define HIDSPORB_VENDOR_ID 0x5a8
#define HIDSPORB_PRODUCT_ID 0x360

#define HIDSPORB_FEATURE_PACKET_ID 2
#define HIDSPORB_CURVE_PACKET_ID 3

#endif
