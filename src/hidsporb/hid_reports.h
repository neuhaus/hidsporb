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

#ifndef HID_REPORTS_H
#define HID_REPORTS_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif


#define MAXBYTES_GAME_REPORT    ( 512 )
/*was 256... changed after setting number of buttons to 16?*/

#define NUM_AXES 6
#define HSO_NUMBER_DESCRIPTORS (1)

#include <pshpack1.h>

typedef struct _HIDSPORB_INPUT_DATA
{
unsigned char report_id;
ULONG   Axis[NUM_AXES];
USHORT  button_map;
/*  UCHAR   Button[NUM_BUTTONS]; */
} HIDSPORB_INPUT_DATA, *PHIDSPORB_INPUT_DATA;
typedef struct _HIDSPORB_INPUT_DATA UNALIGNED *PUHIDSPORB_INPUT_DATA;

#include <poppack.h>


NTSTATUS
HSO_generate_report
(
 PDEVICE_OBJECT   p_device_object,
 UCHAR           rgGameReport[MAXBYTES_GAME_REPORT],
 PUSHORT         p_report_size
 );

VOID 
HSO_generate_hid_data
(
 PDEVICE_EXTENSION       p_device_extension,
 PUHIDSPORB_INPUT_DATA    pHIDData
 );
 


#endif
