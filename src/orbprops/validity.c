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
