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
#ifndef ORB_COMM_H
#define ORB_COMM_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif

void
clear_orb_buffer( 
		 PDEVICE_EXTENSION device_extension, 
		 orb_packet_type new_packet_type 
		 );
		  

void
parse_orb_character( 
		    PDEVICE_EXTENSION device_extension,
		    UCHAR ch
		    );

void 
parse_reset_packet( 
		   PDEVICE_EXTENSION device_extension,
		   UCHAR* pch,
		   int length
		   );

void 
parse_ball_data_packet( 
		       PDEVICE_EXTENSION device_extension,
		       UCHAR* pch,
		       int length 
		       );

void 
parse_button_data_packet( 
			 PDEVICE_EXTENSION device_extension,
			 UCHAR* pch,
			 int length 
			 );

void 
parse_error_packet( 
		   PDEVICE_EXTENSION device_extension,
		   UCHAR* pch,
		   int length 
		   );

void
parse_null_region_packet( 
			 PDEVICE_EXTENSION device_extension,
			 UCHAR* pch,
			 int length 
			 );

void 
parse_terminator_packet( 
			PDEVICE_EXTENSION device_extension,
			UCHAR* pch, 
			int length 
			);

void
send_query_packet( 
		  PDEVICE_OBJECT device_object 
		  );

void
send_pulse_packet( 
		  PDEVICE_OBJECT device_object,
		  int pulse_value 
		  );

void
send_data_request_packet( 
			 PDEVICE_OBJECT device_object 
			 );

void
send_null_region_request_packet( 
				PDEVICE_OBJECT device_object 
				);

void
send_null_region_packet( 
			PDEVICE_OBJECT device_object, 
			int new_region 
			);

void 
send_terminator_packet( 
		       PDEVICE_OBJECT device_object 
		       );
		   
int
orb_has_spoken( 
	       PDEVICE_EXTENSION device_extension 
	       );

void
set_orb_has_spoken( 
		   PDEVICE_EXTENSION device_extension 
		   );

void
complete_incoming_packet( 
			 PDEVICE_EXTENSION device_extension 
			 );

#endif

