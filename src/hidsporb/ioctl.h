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

#ifndef IOCTL_H
#define IOCTL_H

#include <devioctl.h>

/* NOTE: THIS GUID DOES NOT APPEAR TO WORK */
// {FFB22B6F-C928-4f6b-8F83-F031070CC9F8}
DEFINE_GUID( GUID_CLASS_HIDSPORB, 
0xffb22b6f, 0xc928, 0x4f6b, 0x8f, 0x83, 0xf0, 0x31, 0x7, 0xc, 0xc9, 0xf8);

typedef struct _HIDSPORB_AXIS_MAPPING_DATA
{
  LONG logical_axis_number;
  LONG physical_axis_number;
} HIDSPORB_AXIS_MAPPING_DATA, *PHIDSPORB_AXIS_MAPPING_DATA;

typedef struct _HIDSPORB_CHORDING_DATA
{
  LONG use_chording;
} HIDSPORB_CHORDING_DATA, *PHIDSPORB_CHORDING_DATA;

NTSTATUS 
HSO_internal_ioctl
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );

NTSTATUS
HSO_get_device_descriptor
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );


NTSTATUS
HSO_get_report_descriptor
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );


NTSTATUS
hso_read_report
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );


NTSTATUS
HSO_get_attributes
(
 PDEVICE_OBJECT  p_device_object,
 PIRP            Irp
 );





#endif
