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
#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif

#define DRIVER_OBJECT_EXTENSION_REGPATH 1

/* #define HSO_get_registry_path( driver_object ) \ */
/*    (PUNICODE_STRING)IoGetDriverObjectExtension(driver_object, (PVOID) DRIVER_OBJECT_EXTENSION_REGPATH ) */


void
HSO_store_registry_path( PDRIVER_OBJECT driver_object,
			 PUNICODE_STRING registry_path );

void
HSO_retrieve_settings_from_registry( PDEVICE_OBJECT device_object,
				     PDEVICE_EXTENSION device_extension,
				     HANDLE handle );

NTSTATUS
HSO_set_axis_mapping( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_sensitivity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_polarity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_gain( PDEVICE_EXTENSION device_extension,
	      LONG logical_axis_number,
	      LONG gain );

NTSTATUS
HSO_set_precision_sensitivity( PDEVICE_EXTENSION device_extension,
			       LONG sensitivity );

NTSTATUS
HSO_set_precision_gain( PDEVICE_EXTENSION device_extension,
			LONG gain );

NTSTATUS
HSO_set_precision_button( PDEVICE_EXTENSION device_extension,
			  LONG button_type,
			  LONG button_index );


NTSTATUS
HSO_set_chording( PDEVICE_EXTENSION device_extension,
		  BOOLEAN use_chording );

NTSTATUS
HSO_set_null_region( PDEVICE_OBJECT device_object,
		     int null_region );

NTSTATUS
HSO_save_settings( PDEVICE_EXTENSION device_extension );


#endif
