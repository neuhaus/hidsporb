//
// validity.c
// axis, gain, sensitivity, etc validity checks
//

#include "hidsporb.h"

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
OrbIsValidAxis( int axis )
{
  return in_inclusive_range( axis, 0, 5 );
}
 
int
OrbIsValidSensitivity( int sensitivity )
{
  return in_inclusive_range( sensitivity, 0, 5 );
}

int 
OrbIsValidPolarity( int polarity )
{
  return in_inclusive_range( polarity, 0, 2 );
}

int
OrbIsValidGain( int gain )
{
  return in_inclusive_range( gain, 0, 100 );
}

int
OrbIsValidButtonType( int button_type )
{
  return in_inclusive_range( button_type, 0, 2 );
}

int
OrbIsValidPhysicalButtonIndex( int button_index ) 
{
  return in_inclusive_range( button_index, 0, 5 );
}

int
OrbIsValidLogicalButtonIndex( int button_index )
{
  return in_inclusive_range( button_index, 0, 15 );
}

int 
OrbIsValidNullRegion( int region )
{
  return in_inclusive_range( region, 0, 127 );
}

int
OrbIsValidCurveId(IN ULONG curve_id)
{
	return in_inclusive_range( curve_id, 0, 5 );
}

int
OrbIsValidCurve(IN PUSHORT buffer)
{
	int result = 1;
	int i;

	for (i = 0; i < 1024; ++i) {
		// Check if curve is valid
		if (!(in_inclusive_range( buffer[i], 0, 1023 ))) {
			result = 0;
			// We can exit already
			break;
		}
	}

	return result;
}
