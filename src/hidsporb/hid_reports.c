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

#include "hid_reports.h"
#include "hidtoken.h"
#include "hidusage.h"
#include "hidport.h"
#ifndef DEBUG_H
#include "debug.h"
#endif
#ifndef CHARTS_H
#include "charts.h"
#endif

#ifdef ALLOC_PRAGMA
/*      #pragma alloc_text (PAGE, HSO_generate_report) */
#endif

#ifndef HIDSPORB_REPORT_DESCRIPTOR_H
#include "hidsporb_report_descriptor.h"
#endif
  

  /*
    Generates a report descriptor for the orb device.  Basically just
    copies the Report_descriptor buffer from hidsporb_report_descriptor.h
  */
  NTSTATUS 
    HSO_generate_report
    (
     IN PDEVICE_OBJECT       p_device_object, 
     OUT UCHAR               game_report[MAXBYTES_GAME_REPORT],
     OUT PUSHORT             p_report_size
     )
{
  NTSTATUS    ntStatus = STATUS_SUCCESS;

  HSO_LOG( FILE_HID_REPORTS | HSO_FUNCTION_ENTRY | HSO_MESSAGE1,
	   ( "Entering generate_report; size = %d",
	     sizeof( Report_descriptor )));

  if ( sizeof( Report_descriptor ) > MAXBYTES_GAME_REPORT ) 
    {
      ntStatus = STATUS_BUFFER_TOO_SMALL;
      *p_report_size = 0x0;
      RtlZeroMemory( game_report, sizeof( game_report ) );
    }
  else
    {
      ntStatus = STATUS_SUCCESS;
      *p_report_size = sizeof( Report_descriptor );
      RtlMoveMemory( game_report, Report_descriptor, sizeof( Report_descriptor ));
    }

  return ntStatus;
}




ULONG
HSO_logical_axis_value( PDEVICE_EXTENSION device_extension,
			int index,
			int use_precision )
{
  LONG looked_up;
  LONG base = device_extension->physical_axes[ device_extension->axis_map[ index ] ];
  LONG this_axis_gain = ( use_precision != 0 ) ? device_extension->precision_gain
    : device_extension->gains[ index ];
  LONG this_axis_sensitivity = ( use_precision != 0 ) ? device_extension->precision_sensitivity
    : device_extension->sensitivities[ index ];
  LONG with_gain;
  LONG dividend;
  LONG multiplicand;
  const long Scale = 1000;
  // figure the gain multiplier.  This is an odd beastie.  Basically,
  //at a gain of 50, the scale is even.  At higher levels of gain, the signal
  //is "amplified" so that at gain = 100, values are multiplied by 5.  At
  //gain = 75, values are multiplied by 3.
  if ( this_axis_gain < 50 )
    {
      dividend = (( 50 - this_axis_gain ) * Scale * 10 / 125) + (1 * Scale );
      with_gain = (base * Scale) / dividend;
    }
  else if ( this_axis_gain == 50 )
    {
      with_gain = base;
    }
  else 
    {
      multiplicand = (( this_axis_gain - 50 ) * Scale * 10 / 125) + (1 * Scale );
      with_gain = base * multiplicand / Scale;
    }
  //add 512 to get an array index into the sensitivity curves
  with_gain = with_gain + 512;  
  //clip to get a valid index
  if ( with_gain < 0 )
    {
      with_gain = 0;
    }
  if ( with_gain > 1023 ) 
    {
      with_gain = 1023;
    }
  //flop according to polarity
  switch ( device_extension->polarities[ index ] )
    {
    case HIDSPORB_POLARITY_NEGATIVE :
      with_gain = (1023 - with_gain);
      break;
    case HIDSPORB_POLARITY_ZERO :
      with_gain = 512;
      break;
    case HIDSPORB_POLARITY_POSITIVE :
      break;
    }
  //finally, look up the chart.
  looked_up = charts[ this_axis_sensitivity ][ with_gain ];
  return looked_up;
}

/*
  The button map is a 16-bit value where the LSB returns the
  status of button 0 and the MSB returns the status of button 15
*/
USHORT
HSO_button_map( PDEVICE_EXTENSION device_extension )
{
  int chord_page;
  USHORT result = 0;
  int i;

  if ( device_extension->use_chording )
    {
      chord_page = ((device_extension->physical_buttons[ 1 ] != 0) ? 2 : 0) +
	((device_extension->physical_buttons[0] != 0) ? 1 : 0);

      for (i = NUM_PHYSICAL_BUTTONS - 2; i > 1; --i )
	{
	  result <<= 1;
	  result |= ( device_extension->physical_buttons[ i ] != 0 ) ? 1 : 0;
	}

      result <<= ( chord_page * 4 );
    }
  else
    {
      //loops seemed to work very strangely, here, so we'll go the brute-force
      //way...
#define PHYSICAL_BUTTON( x ) ((device_extension->physical_buttons[ x ] != 0 ) ? 1 : 0 )
      result = PHYSICAL_BUTTON( 0 ) |
	( PHYSICAL_BUTTON( 1 ) << 1 ) |
	( PHYSICAL_BUTTON( 2 ) << 2 ) |
	( PHYSICAL_BUTTON( 3 ) << 3 ) |
	( PHYSICAL_BUTTON( 4 ) << 4 ) |
	( PHYSICAL_BUTTON( 5 ) << 5 );
    }
  return result;
}

/*
Actually generates the HID report based on the current values of the
orb status
*/
VOID 
    HSO_generate_hid_data
    (
    IN      PDEVICE_EXTENSION       device_extension,
    IN  OUT PUHIDSPORB_INPUT_DATA    pHIDData
    )
{
    LONG    Idx;
    int use_precision = 0;
    /*
     *  Use a local buffer to assemble the report as the real buffer may not 
     *  be aligned.
     */
    HIDSPORB_INPUT_DATA  local_buffer;

    RtlZeroMemory( &local_buffer, sizeof( local_buffer ) );
    
    //set the report id
    local_buffer.report_id = 1;

    // get the button map and figure out if we're using precision
    local_buffer.button_map = HSO_button_map( device_extension );
    switch ( device_extension->precision_button_type )
      {
      case HIDSPORB_BUTTON_TYPE_NONE :
	use_precision = 0;
	break;
      case HIDSPORB_BUTTON_TYPE_PHYSICAL :
	if ( device_extension->physical_buttons[ device_extension->precision_button_index ] != 0 )
	  {
	    use_precision = 1;
	  }
	else
	  {
	    use_precision = 0;
	  }
	break;
      case HIDSPORB_BUTTON_TYPE_LOGICAL :
	if ( local_buffer.button_map & ( (USHORT)(1) << (device_extension->precision_button_index ) ) )
	  {
	    use_precision = 1;
	  }
	else
	  {
	    use_precision = 0;
	  }
	break;
      default :
	use_precision = 0;
	break;
      }
    // now copy the axes
    for(Idx = 0x0; Idx < NUM_AXES; Idx++ )
      {
	local_buffer.Axis[Idx] = HSO_logical_axis_value( device_extension, Idx, use_precision );
      }

    C_ASSERT( sizeof( *pHIDData ) == sizeof( local_buffer ) );
    RtlCopyMemory( pHIDData, &local_buffer, sizeof( local_buffer ) );

    HSO_LOG( FILE_HID_REPORTS |  HSO_MESSAGE2, \
	     ("HSO_generate_hid_data: Axes: X: %08x   Y: %08x   Z: %08x   Rx: %08x   Ry: %08x   Rz: %08x", \
	      local_buffer.Axis[0], local_buffer.Axis[1], local_buffer.Axis[2], local_buffer.Axis[3],
	      local_buffer.Axis[4], local_buffer.Axis[5]) ) ;
}













