#ifndef VALIDITY_H
#define VALIDITY_H

int is_valid_axis( int axis );
int is_valid_sensitivity( int sensitivity );
int is_valid_polarity( int polarity );
int is_valid_gain( int gain );
int is_valid_button_type( int button_type );
int is_valid_physical_button_index( int button_index );
int is_valid_logical_button_index( int button_index );
int is_valid_null_region( int new_null_region );
int OrbIsValidCurveId(IN ULONG curve_id);
int OrbIsValidCurve(PUSHORT buffer);

#endif
