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
#include "orb_comm.h"

#ifndef DEBUG_H
#include "debug.h"
#endif
#ifndef SERIAL_ROUTINES_H
#include "serial_routines.h"
#endif
#ifndef IRP_QUEUE_H
#include "irp_queue.h"
#endif

#define BALL_DATA_PACKET_LENGTH 12
#define RESET_PACKET_LENGTH 51
#define BUTTON_DATA_PACKET_LENGTH 5
#define ERROR_PACKET_LENGTH 4
#define NULL_REGION_PACKET_LENGTH 3
#define TERMINATOR_PACKET_LENGTH 1

void
clear_orb_buffer( PDEVICE_EXTENSION device_extension,
		  orb_packet_type new_packet_type )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		("clear_orb_buffer enter" ));
  device_extension->packet_buffer_cursor = 0;
  device_extension->current_packet_type = new_packet_type;
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK, 
		("clear_orb_buffer exit" ));
}

void
complete_incoming_packet( PDEVICE_EXTENSION device_extension )
{
  clear_orb_buffer( device_extension, HSO_unknown_packet );
  set_orb_has_spoken( device_extension );

  //send our new null region packet if appropriate
  if ( orb_has_spoken( device_extension ) && ( device_extension->new_null_region_pending ) )
    {
      HSO_LOG( FILE_ORB_COMM | HSO_MESSAGE2,
		    ("Sending new null region packet: region %d",
		     device_extension->null_region ) );
      send_null_region_packet( device_extension->device_object,
			       device_extension->null_region );
      device_extension->new_null_region_pending = FALSE;
    }
  
  HSO_complete_read_queue_head( device_extension );
}

void
parse_orb_character( PDEVICE_EXTENSION device_extension,
		     UCHAR ch )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		("parse_orb_character entered : %c", ch ));
  /* first off, if we're going beyond the approved length of
     of the buffer, something is wrong.  Rather than crashing
     with an assertion, just clear the buffer; the rest of this
     packet, if legitimate, will be lost, but that's better
     than bringing the system to ruin--and the orb will
     continue transmitting just fine
  */
  if ( device_extension->packet_buffer_cursor >= (ORB_PACKET_BUFFER_LENGTH - 1 ))
    {
      clear_orb_buffer( device_extension, HSO_unknown_packet );
    }

  /* now if it's a readable character that marks the beginning
     of an orb packet, we also clear the orb buffer */
  switch (ch )
    {
    case 'R' :
      clear_orb_buffer( device_extension, HSO_reset_packet );
      break;
    case 'D' :
      clear_orb_buffer( device_extension, HSO_ball_data_packet );
      break;
    case 'K' :
      clear_orb_buffer( device_extension, HSO_button_data_packet );
      break;
    case 'E' :
      clear_orb_buffer( device_extension, HSO_error_packet );
      break;
    case 'N' :
      clear_orb_buffer( device_extension, HSO_null_region_packet );
      break;
    case '\0x0d' :
      clear_orb_buffer( device_extension, HSO_terminator_packet );
      break;
    }

  /* now let's add the character to the buffer if we're in a packet we
     understand */
  if ( device_extension->current_packet_type != HSO_unknown_packet )
    {
      /* add the character to the buffer */
      device_extension->packet_buffer[ device_extension->packet_buffer_cursor ] = ch;
      ++(device_extension->packet_buffer_cursor);
    }
  /* now if we're finished with the packet, parse it! */
  switch ( device_extension->current_packet_type )
    {
    case HSO_reset_packet :
      if ( device_extension->packet_buffer_cursor == RESET_PACKET_LENGTH )
	{
	  parse_reset_packet( device_extension,
			      device_extension->packet_buffer,
			      device_extension->packet_buffer_cursor );
	  break;
	}
    case HSO_ball_data_packet :
      if ( device_extension->packet_buffer_cursor == BALL_DATA_PACKET_LENGTH )
	{
	  parse_ball_data_packet( device_extension, 
				  device_extension->packet_buffer,
				  device_extension->packet_buffer_cursor );
	}
      break;

    case HSO_button_data_packet :
      if ( device_extension->packet_buffer_cursor == BUTTON_DATA_PACKET_LENGTH )
	{
	  parse_button_data_packet( device_extension,
				    device_extension->packet_buffer,
				    device_extension->packet_buffer_cursor );
	}
      break;
      
    case HSO_error_packet :
      if ( device_extension->packet_buffer_cursor == ERROR_PACKET_LENGTH )
	{
	  parse_error_packet( device_extension,
			      device_extension->packet_buffer,
			      device_extension->packet_buffer_cursor );
	}
      break;
      
    case HSO_null_region_packet :
      if ( device_extension->packet_buffer_cursor == NULL_REGION_PACKET_LENGTH )
	{
	  parse_null_region_packet( device_extension,
				    device_extension->packet_buffer,
				    device_extension->packet_buffer_cursor );
	}
      break;

    case HSO_terminator_packet :
      if ( device_extension->packet_buffer_cursor == TERMINATOR_PACKET_LENGTH )
	{
	  parse_terminator_packet( device_extension,
				   device_extension->packet_buffer,
				   device_extension->packet_buffer_cursor );
	}
      break;

    }
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "parse_orb_character" ));
} 

void
parse_reset_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		("Entering parse_reset_packet" ));
  if ( ( 'R' == pch[ 0 ] ) && ( RESET_PACKET_LENGTH == length ) )
    {
      pch[ length ] = 0;
      HSO_LOG( FILE_ORB_COMM | HSO_MESSAGE1,
		    ( "Reset packet: %s", pch ));
    }
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting parse_reset_packet" ));
}

void
parse_ball_data_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  /* parse that strange orb data; algorithm taken from liborb */
  unsigned int tx, ty, tz, rx, ry, rz;
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering parse_ball_data_packet" ));
  if ( ( 'D' == pch[ 0 ] ) && ( BALL_DATA_PACKET_LENGTH == length ))
    {
      /* xor our button data with the string "SpaceWare" */
      pch[2] = pch[2] ^ 'S';
      pch[3] = pch[3] ^ 'p';
      pch[4] = pch[4] ^ 'a';
      pch[5] = pch[5] ^ 'c';
      pch[6] = pch[6] ^ 'e';
      pch[7] = pch[7] ^ 'W';
      pch[8] = pch[8] ^ 'a';
      pch[9] = pch[9] ^ 'r';
      pch[10] = pch[10] ^ 'e';

      /* note that this starts with pch[2], which means we're
	 ignoring the new copy of the button data for now*/
      tx = ((pch[2] & 0x7F) << 3) | 
	((pch[3] & 0x70) >> 4);
 
      ty = ((pch[3] & 0x0F) << 6) | 
	((pch[4] & 0x7E) >> 1);
 
      tz = ((pch[4] & 0x01) << 9) | 
	((pch[5] & 0x7F) << 2) |
	((pch[6] & 0x60) >> 5);
 
      rx = ((pch[6] & 0x1F) << 5) | 
	((pch[7] & 0x7C) >> 2);
 
      ry = ((pch[7] & 0x03) << 8) | 
	((pch[8] & 0x7F) << 1) |
	((pch[9] & 0x40) >> 6);
 
      rz = ((pch[9]  & 0x3F) << 4) | 
	((pch[10] & 0x78) >> 3);

      /* set timer data?  removing this for now */ 
/*        handle->timer = ((pch[10] & 0x07) << 7) |  */
/*  	(pch[11] & 0x7F); */
 
      device_extension->physical_axes[0] = ((((long) tx) << 22) >> 22 ); 
      device_extension->physical_axes[1] = ((((long) ty) << 22) >> 22 );
      device_extension->physical_axes[2] = ((((long) tz) << 22) >> 22 );
      device_extension->physical_axes[3] = ((((long) rx) << 22) >> 22 ); 
      device_extension->physical_axes[4] = ((((long) ry) << 22) >> 22 );
      device_extension->physical_axes[5] = ((((long) rz) << 22) >> 22 );
      HSO_LOG( FILE_ORB_COMM | HSO_MESSAGE1,
		    ("Axis status: %d, %d, %d, %d, %d, %d",
/*  		     tx, ty, tz, rx, ry, rz )); */
		     device_extension->physical_axes[ 0 ],
		     device_extension->physical_axes[ 1 ],
		     device_extension->physical_axes[ 2 ],
		     device_extension->physical_axes[ 3 ],
		     device_extension->physical_axes[ 4 ],
		     device_extension->physical_axes[ 5 ] ));
    }
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting parse_ball_data_packet" ));
}

void
parse_button_data_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  UCHAR data = pch[ 2 ];
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering parse_button_data_packet" ));
  if ( ('K' == pch[0] ) && ( BUTTON_DATA_PACKET_LENGTH == length ) )
    {
      device_extension->physical_buttons[ 0 ] = data & 0x01;
      device_extension->physical_buttons[ 1 ] = data & 0x02;
      device_extension->physical_buttons[ 2 ] = data & 0x04;
      device_extension->physical_buttons[ 3 ] = data & 0x08;
      device_extension->physical_buttons[ 4 ] = data & 0x10;
      device_extension->physical_buttons[ 5 ] = data & 0x20;
      device_extension->physical_buttons[ 6 ] = data & 0x40;
    }
  /* now debug output... yes, cruder than a loop, but effective and
     it all goes away on a non-debug build*/
  HSO_LOG( FILE_ORB_COMM | HSO_MESSAGE1,
		("Button status: %d%d%d%d%d%d%d",
		 (device_extension->physical_buttons[ 0 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 1 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 2 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 3 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 4 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 5 ] == 0 ) ? 0 : 1,
		 (device_extension->physical_buttons[ 6 ] == 0 ) ? 0 : 1));
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		("exiting parse_button_data_packet" ));
}

void
parse_error_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering parse_error_packet" ));
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting parse_error_packet" ));
}

void
parse_null_region_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering parse_null_region_packet" ));
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting parse_null_region_packet" ));
}

void
parse_terminator_packet( PDEVICE_EXTENSION device_extension,
		    UCHAR* pch,
		    int length )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering parse_terminator_packet" ));
  complete_incoming_packet( device_extension );
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting parse_terminator_packet" ));
}

void
send_query_packet( PDEVICE_OBJECT device_object )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering send_query_packet" ));
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting send_query_packet" ));
}

void
send_pulse_packet( PDEVICE_OBJECT device_object,
		   int pulse_value)
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering send_pulse_packet" ));
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting send_pulse_packet" ));
}

void
send_data_request_packet( PDEVICE_OBJECT device_object )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering send_data_request_packet" ));
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting send_data_request_packet" ));
}

void
send_null_region_packet( PDEVICE_OBJECT device_object,
			 int new_null_region )
{
  UCHAR null_byte = (UCHAR)new_null_region;
  if ( ( 0 <= new_null_region ) && ( new_null_region < 129 ) )
    {
      //actually set the null region
      HSO_LOG( FILE_ORB_COMM | HSO_MESSAGE2,
		    ( "Sending packet with null byte %d", null_byte ) );
      null_byte = (UCHAR)new_null_region;
      HSO_write_character( device_object, 'N' );
      HSO_write_character( device_object, null_byte );
      HSO_write_character( device_object, 0x0d );
    }
}

void
send_null_region_request_packet( PDEVICE_OBJECT device_object )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering send_null_region_request_packet" ));
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting send_null_region_request_packet" ));
}

void
send_terminator_packet( PDEVICE_OBJECT device_object )
{
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_ENTRY,
		( "Entering send_terminator_packet" ));
  HSO_LOG( FILE_ORB_COMM | HSO_FUNCTION_EXIT_OK,
		( "exiting send_terminator_packet" ));
}



int
orb_has_spoken( PDEVICE_EXTENSION device_extension )
{
  return ( device_extension->orb_has_spoken != 0 );
}

void
set_orb_has_spoken( PDEVICE_EXTENSION device_extension )
{
  device_extension->orb_has_spoken = 1;
}
