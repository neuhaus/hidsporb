#ifndef VALIDITY_H
#define VALIDITY_H

int OrbIsValidAxis( int axis );
int OrbIsValidSensitivity( int sensitivity );
int OrbIsValidPolarity( int polarity );
int OrbIsValidGain( int gain );
int OrbIsValidButtonType( int button_type );
int OrbIsValidPhysicalButtonIndex( int button_index );
int OrbIsValidLogicalButtonIndex( int button_index );
int OrbIsValidNullRegion( int new_null_region );
int OrbIsValidCurveId(IN ULONG curve_id);
int OrbIsValidCurve(PUSHORT buffer);

#endif
