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
#ifndef SERIAL_ROUTINES_H
#define SERIAL_ROUTINES_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif

NTSTATUS 
HSO_get_line_control(
    PDEVICE_OBJECT p_device_object,
    PSERIAL_LINE_CONTROL    serial_line_control
    );

VOID
HSO_close_port(
	      PDEVICE_OBJECT device_object,
	      PDEVICE_EXTENSION device_extension,
	      PIRP Irp
	      );

VOID
HSO_restore_port(
		PDEVICE_OBJECT device_object,
		PDEVICE_EXTENSION device_extension
		);

NTSTATUS
HSO_initialize_port(
		    PDEVICE_OBJECT device_object,
		    PDEVICE_EXTENSION device_extension 
		    );

NTSTATUS
HSO_initialize_serial_device(
		PDEVICE_OBJECT device_object,
		PDEVICE_EXTENSION device_extension
);

NTSTATUS
HSO_detect_spaceorb(
		    PDEVICE_OBJECT device_object,
		    PDEVICE_EXTENSION device_extension
);



NTSTATUS
HSO_write_serial_port (
		       PDEVICE_OBJECT device_object, /* our FDO */
		       PCHAR               write_buffer,
		       ULONG               num_bytes,
		       PIO_STATUS_BLOCK    io_status_block
		       );

NTSTATUS
HSO_write_character(
	       PDEVICE_OBJECT device_object, /* our fdo */
	       UCHAR value
	       );

NTSTATUS
HSO_write_string(
		 PDEVICE_OBJECT device_object, /* our fdo */
		 PSZ buffer
		 );


NTSTATUS
HSO_start_read_thread(PDEVICE_OBJECT devObj, 
		      PDEVICE_EXTENSION devExt);


VOID
HSO_read_thread(PVOID Context);


NTSTATUS
HSO_read_packet(PDEVICE_OBJECT device_object, 
		PDEVICE_EXTENSION device_extension);




#endif







