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

#include "validity.h"

int
in_inclusive_range( int value, int min, int max )
{
  if ( (value >= min) && ( value <= max ) )
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
is_valid_axis( int axis )
{
  return in_inclusive_range( axis, 0, 5 );
}
 
int
is_valid_sensitivity( int sensitivity )
{
  return in_inclusive_range( sensitivity, 0, 5 );
}

int 
is_valid_polarity( int polarity )
{
  return in_inclusive_range( polarity, 0, 2 );
}

int
is_valid_gain( int gain )
{
  return in_inclusive_range( gain, 0, 100 );
}

int
is_valid_button_type( int button_type )
{
  return in_inclusive_range( button_type, 0, 2 );
}

int
is_valid_physical_button_index( int button_index ) 
{
  return in_inclusive_range( button_index, 0, 5 );
}

int
is_valid_logical_button_index( int button_index )
{
  return in_inclusive_range( button_index, 0, 15 );
}

int 
is_valid_null_region( int region )
{
  return in_inclusive_range( region, 0, 127 );
}

int
is_valid_curve_id( int curve_id )
{
  return in_inclusive_range( curve_id, 0, 5 );
}

int
is_valid_curve( unsigned short* buffer )
{
  int result = 1;
  int i;
  for ( i = 0; i < 1024; ++i )
    {
      if ( !(in_inclusive_range( buffer[i], 0, 1023 )))
	{
	  result = 0;
	}
    }
  return result;
}
