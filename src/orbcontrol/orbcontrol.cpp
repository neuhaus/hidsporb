#include "orb_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONTRACT_H
#include "contract.h"
#endif
extern "C" {
#ifndef CHARTS_H
#include "charts.h"
#endif
}

char
polarity_symbol( int polarity )
{
  switch ( polarity )
    {
    case 0 : 
      return '-';
      break;
    case 1 :
      return '0';
      break;
    case 2 : 
      return '+';
      break;
    default :
      return '?';
      break;
    }
}

void
show_orb_status( orb_control& control )
{
      
  printf( "Control is initialized.  Current state of orb:\n" );
  printf( "\t   Axis map : %d/%d/%d/%d/%d/%d\n", 
	  control.physical_axis_from_logical_axis( 0 ),
	  control.physical_axis_from_logical_axis( 1 ),
	  control.physical_axis_from_logical_axis( 2 ),
	  control.physical_axis_from_logical_axis( 3 ),
	  control.physical_axis_from_logical_axis( 4 ),
	  control.physical_axis_from_logical_axis( 5 ) );
  printf( "\tSensitivity : %d/%d/%d/%d/%d/%d\n",
	  control.sensitivity( 0 ),
	  control.sensitivity( 1 ),
	  control.sensitivity( 2 ),
	  control.sensitivity( 3 ),
	  control.sensitivity( 4 ),
	  control.sensitivity( 5 ) );
  printf( "\t   Polarity : %c/%c/%c/%c/%c/%c\n",
	  polarity_symbol( control.polarity( 0 )),
	  polarity_symbol( control.polarity( 1 )),
	  polarity_symbol( control.polarity( 2 )),
	  polarity_symbol( control.polarity( 3 )),
	  polarity_symbol( control.polarity( 4 )),
	  polarity_symbol( control.polarity( 5 )));
  printf( "\t       Gain : %d/%d/%d/%d/%d/%d\n",
	  control.gain( 0 ),
	  control.gain( 1 ),
	  control.gain( 2 ),
	  control.gain( 3 ),
	  control.gain( 4 ),
	  control.gain( 5 ) );
  printf( "\tChording is: %s\n", 
	  ( control.using_chording() != 0 ) ? "On" : "Off" );
  printf( "\tFacing: %s\n",
	  ( control.current_facing() == orb_control::Horizontal_facing ? 
	    "Horizontal" : 
	    ( (control.current_facing() == orb_control::Vertical_facing ) ?
	      "Vertical" :
	      "Custom" ) ) );
  printf( "\tNull region: %d\n", control.null_region() );
  printf( "\tPrecision sensitivity: %d\n", control.precision_sensitivity() );
  printf( "\tPrecision gain: %d\n", control.precision_gain() );
  printf( "\tPrecision button: " );
  if ( control.precision_button_type() == 0 )
    {
      printf( "\tnone\n" );
    }
  else
    {
      printf( "%s button %d\n",
	      ( control.precision_button_type() == 1 ) ? "physical" : "logical",
	      control.precision_button_index() );
    }
  printf( "(warning: null region is in testing and may not work correctly)\t" );
}


//returns the number of arguments consumed during this parse
int
parse_set_axis( int arg_count,
		char** arg_vector,
		int arg_index,
		orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-axis" ) == 0 );
  REQUIRE( arg_count > ( arg_index + 2 ));
  int logical_axis = atoi( arg_vector[ arg_index + 1 ] );
  int physical_axis = atoi( arg_vector[ arg_index + 2 ] );
  if ( ( logical_axis >= 0 ) && (logical_axis <= 5) &&
       ( physical_axis >= 0 ) && (physical_axis <= 5) )
    {
      printf( "Setting logical axis %d to physical axis %d\n", 
	      logical_axis,
	      physical_axis );
      control.set_axis( logical_axis, physical_axis );
    }
  else
    {
      printf( "Invalid axis: must be in the range 0..5\n" );
    }
  return 3;
}

int
parse_set_chording( int arg_count,
		    char** arg_vector,
		    int arg_index,
		    orb_control& control )
{
  int new_chording;
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-chording" ) == 0 );
  REQUIRE( arg_count > ( arg_index + 1 ) );
  if ( strcmp( arg_vector[ arg_index + 1 ], "on" ) == 0 )
    {
      new_chording = 1;
    }
  else if ( strcmp( arg_vector[ arg_index + 1 ], "off" ) == 0 )
    {
      new_chording = 0;
    }
  else
    {
      new_chording = -1;
    }
  if ( new_chording != -1 )
    {
      printf( "Setting chording to %s\n", (new_chording != 0) ? "on" : "off" );
      control.set_chording( new_chording );
    }
  else
    {
      printf( "Invalid chording setting; must be 'on' or 'off'\n" );
    }
  return 2;
}

int
parse_set_facing(  int arg_count,
		   char** arg_vector,
		   int arg_index,
		   orb_control& control )
{
  orb_control::facing new_facing;
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-facing" ) == 0 );
  REQUIRE( arg_count > ( arg_index + 1 ) );
  if ( strcmp( arg_vector[ arg_index + 1 ], "horizontal" ) == 0 )
    {
      new_facing = orb_control::Horizontal_facing;
    }
  else if ( strcmp( arg_vector[ arg_index + 1 ], "vertical" ) == 0 )
    {
      new_facing = orb_control::Vertical_facing;
    }
  else
    {
      new_facing = orb_control::Custom_facing;
    }
  if ( new_facing != orb_control::Custom_facing )
    {
      printf( "Setting facing to %s\n", 
	      (new_facing == orb_control::Horizontal_facing) ? 
	      "horizontal" : "vertical" );
      control.set_facing( new_facing );
    }
  else
    {
      printf( "Invalid facing setting; must be 'horizontal' or 'vertical'\n" );
    }
  return 2;
}

int
parse_set_null_region( int arg_count,
		   char** arg_vector,
		   int arg_index,
		   orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-null-region" ) == 0 );
  REQUIRE( arg_count > arg_index + 1 );
  
  int new_null_region = atoi( arg_vector[ arg_index + 1 ] );

  if ( (new_null_region >= 0 ) && (new_null_region <= 127 ) )
    {
      printf( "Setting null region to %d\n" );
      control.set_null_region( new_null_region );
    }
  else
    {
      printf( "Invalid null region: must be in the range 0..127\n" );
    }
  return 2;
}

int
parse_set_sensitivity( int arg_count,
		       char** arg_vector,
		       int arg_index,
		       orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-sensitivity" ) == 0 );
  REQUIRE( arg_count > arg_index + 2 );
  
  int axis = atoi( arg_vector[ arg_index + 1 ] );
  int new_sensitivity = atoi( arg_vector[ arg_index + 2 ] );
  
  if ( ( axis >= 0 ) && ( axis <= 5 ) )
    {
      if ( ( new_sensitivity >= 0 ) && ( new_sensitivity <= 5 ) )
	{
	  printf( "Setting sensitivity for axis %d to curve %d\n",
		  axis, new_sensitivity );
	  control.set_sensitivity( axis, new_sensitivity );
	}
      else
	{
	  printf( "Invalid sensitivity; must be in range 0..5\n" );
	}
    }
  else
    {
      printf( "Invalid axis for --set-sensitivity; must be in range 0..5-n" );
    }
  return 3;
}

int
polarity_from_char( char ch )
{
  switch (ch)
    {
    case '-' : 
      return 0;
      break;
    case '0' :
      return 1;
      break;
    case '+' :
      return 2;
      break;
    default :
      return -1;
      break;
    }
}

int
parse_set_polarity( int arg_count,
		    char** arg_vector,
		    int arg_index,
		    orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-polarity" ) == 0 );
  REQUIRE( arg_count > arg_index + 2 );
  
  char polarity_symbol = arg_vector[ arg_index + 2 ][ 0 ];
  int axis = atoi( arg_vector[ arg_index + 1 ] );
  int new_polarity = polarity_from_char( polarity_symbol );

  if ( control.is_valid_axis( axis ) )
    {
      if ( control.is_valid_polarity( new_polarity ) )
	{
	  printf( "Setting polarity for axis %d to %c\n",
		  axis,
		  polarity_symbol );
	  control.set_polarity( axis, new_polarity );
	}
      else
	{
	  printf( "Invalid polarity %c; valid values are -, 0, +\n",
		  polarity_symbol );
	}
    }
  else
    {
      printf ("Invalid axis for --set-polarity; valid values are [0..5]\n" );
    }
  return 3;
}

int
parse_set_master_sensitivity( int arg_count,
			      char** arg_vector,
			      int arg_index,
			      orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-master-sensitivity" ) == 0 );
  REQUIRE( arg_count > arg_index + 1 );

  int new_sensitivity = atoi( arg_vector[ arg_index + 1 ]);
  int i;

  if ( control.is_valid_sensitivity( new_sensitivity ) )
    {
      printf( "Setting master sensitivity to curve %d\n", new_sensitivity );
      for ( i = 0; i < 6; ++i )
	{
	  control.set_sensitivity( i, new_sensitivity );
	}
    }
  else
    {
      printf( "Invalid sensitivity; valid values are [0..5]\n" );
    }
  return 2;
}

int
parse_set_gain( int arg_count,
		char** arg_vector,
		int arg_index,
		orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-gain" ) == 0 );
  REQUIRE( arg_count > arg_index + 2 );
  
  int axis = atoi( arg_vector[ arg_index + 1 ] );
  int new_gain = atoi( arg_vector[ arg_index + 2 ] );

  if ( control.is_valid_axis( axis ) )
    {
      if ( control.is_valid_gain( new_gain ) )
	{
	  printf( "Setting gain for axis %d to %d\n",
		  axis, new_gain );
	  control.set_gain( axis, new_gain );
	}
      else
	{
	  printf( "Invalid gain %d; valid values are [0..100]\n", new_gain );
	}
    }
  else
    {
      printf( "Invalid axis for --set-gain; valid values are [0..5]\n" );
    }
  return 3;
}

int
parse_set_master_gain( int arg_count,
		       char** arg_vector,
		       int arg_index,
		       orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-master-gain" ) == 0 );
  REQUIRE( arg_count > arg_index + 1 );

  int new_gain = atoi( arg_vector[ arg_index + 1 ] );
  int i;

  if ( control.is_valid_gain( new_gain ) )
    {
      printf( "Setting master gain to %d\n", new_gain );
      for ( i = 0; i < 6; ++i )
	{
	  control.set_gain( i, new_gain );
	}
    }
  else
    {
      printf( "Invalid gain; valid values are [0..100]\n" );
    }
  return 2;
}

int
parse_set_precision_sensitivity( int arg_count, 
				 char** arg_vector,
				 int arg_index,
				 orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-precision-sensitivity") == 0 );
  REQUIRE( arg_count > arg_index + 1 );

  int new_sensitivity = atoi( arg_vector[ arg_index + 1 ] );
  if ( control.is_valid_sensitivity( new_sensitivity ) ) 
    {
      printf( "Setting precision sensitivity to %d\n", new_sensitivity );
      control.set_precision_sensitivity( new_sensitivity );
    }
  else
    {
      printf( "Invalid sensitivity; valid values are [0..5]\n" );
    }
  return 2;
}

int
parse_set_precision_gain( int arg_count, 
				 char** arg_vector,
				 int arg_index,
				 orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-precision-gain") == 0 );
  REQUIRE( arg_count > arg_index + 1 );

  int new_gain = atoi( arg_vector[ arg_index + 1 ] );
  if ( control.is_valid_gain( new_gain ) ) 
    {
      printf( "Setting precision gain to %d\n", new_gain );
      control.set_precision_gain( new_gain );
    }
  else
    {
      printf( "Invalid gain; valid values are [0..100]\n" );
    }
  return 2;
}

int
button_type_from_string( char* str )
{
  if ( strcmp( str, "none" ) == 0 )
    {
      return 0;
    }
  else if ( strcmp( str, "physical" ) == 0 )
    {
      return 1;
    }
  else if ( strcmp( str, "logical" ) == 0 )
    {
      return 2;
    }
  else
    {
      return -1;
    }
}
  
int
parse_set_precision_button( int arg_count, 
				 char** arg_vector,
				 int arg_index,
				 orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-precision-button") == 0 );
  REQUIRE( arg_count > arg_index + 1 );

  int new_button_type = button_type_from_string( arg_vector[ arg_index + 1 ] );
  int new_button_index = atoi( arg_vector[ arg_index + 2 ] );
  int args_used = 0;

  if ( control.is_valid_button_type( new_button_type ) )
    {
      switch ( new_button_type ) 
	{
	case 0 :
	  printf( "Setting precision button to NONE\n" );
	  control.set_precision_button( 0, 0 );
	  args_used = 2;
	  break;
	case 1 :
	  if ( control.is_valid_physical_button_index( new_button_index ) )
	    {
	      printf( "Setting precision button to physical button %d\n",
		      new_button_index );
	      control.set_precision_button( 1, new_button_index );
	    }
	  else
	    {
	      printf( "Invalid physical button index; valid values are [0..5]\n" );
	    }
	  args_used = 3;
	  break;
	case 2:
	  if ( control.is_valid_logical_button_index( new_button_index ) )
	    {
	      printf( "Setting precision button to logical button %d\n",
		      new_button_index );
	      control.set_precision_button( 2, new_button_index );
	    }
	  else
	    {
	      printf( "Invalid logical button index; valid values are [0..15]\n" );
	    }
	  args_used = 3;
	  break;
	default :
	  printf( "You have found a program error; please report this to the hidsporb maintainers." );
	  args_used = 1;
	}
    }
  else
    {
      printf( "invalid button type; valid types are 'none', 'logical', 'physical'\n" );
      args_used = 2;
    }
  return args_used;
}

int
parse_set_defaults( int arg_count,
		    char** arg_vector,
		    int arg_index,
		    orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--set-defaults" ) == 0 );
  control.set_defaults();
  for ( int i = 0; i < 6; ++i )
    {
      control.upload_sensitivity_curve( i, charts[ i ] );
    }
      
  return 1;
}

int 
read_curve_from_file( char* curve_filename,
		      unsigned short* buffer ) 
{
  int result = 1;
  int temp;
  FILE* f;
  f = fopen( curve_filename, "r" );
  if ( f != NULL )
    {
      for ( int i = 0; i < 1024; ++i )
	{
	  //use a temp for the read because %d reads an int, not a short
  	  fscanf( f, "%d", &temp );
	  buffer[ i ] = (unsigned short)temp;
	}
      fclose( f );
      result = 1;
    }
  else 
    {
      result = 0;
    }
  return result;
}

int
parse_upload_sensitivity_curve( int arg_count, 
				char** arg_vector,
				int arg_index,
				orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--upload-sensitivity-curve" ) == 0 );
  REQUIRE( arg_count > arg_index + 2 );
  int curve_id = atoi( arg_vector[ arg_index + 1 ] );
  char* curve_filename = arg_vector[ arg_index + 2 ];
  if ( control.is_valid_curve_id( curve_id ) )
    {
      unsigned short buffer[ 1024 ];
      int buffer_read_ok = read_curve_from_file( curve_filename, buffer );
      if ( buffer_read_ok != 0 )
	{
	  if ( control.is_valid_curve( buffer ))
	    {
	      printf( "Uploading curve %s to sensitivity position %d\n",
		      curve_filename, curve_id );
	      control.upload_sensitivity_curve( curve_id, buffer );
	    }
	  else
	    {
	      printf( "Curve %s contained invalid values; proper values are [0..1023]\n" );
	    }
	}
      else
	{
	  printf( "Unable to read curve from file %s\n", curve_filename );
	}
    }
  else
    {
      printf( "Invalid curve ID for upload; valid values are [0..5]\n" );
    }
  return 3;
}


int
write_curve_to_file( char* filename, unsigned short* buffer )
{
  int result = 1;
  FILE* f;
  f = fopen( filename, "w" );

  if ( f != NULL )
    {
      for ( int i = 0; i < 1024; ++i ) 
	{
	  fprintf( f, "%d\n", buffer[ i ] );
	}
      fclose( f );
      result = 1;
    }
  else
    {
      result = 0;
    }
  return result;
}

//forward declaration
int
parse_read_args_from_file( int arg_count, 
			   char** arg_vector, 
			   int arg_index,
			   orb_control& control );

int
parse_download_sensitivity_curve( int arg_count, 
				char** arg_vector,
				int arg_index,
				orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--download-sensitivity-curve" ) == 0 );
  REQUIRE( arg_count > arg_index + 2 );
  int curve_id = atoi( arg_vector[ arg_index + 1 ] );
  char* curve_filename = arg_vector[ arg_index + 2 ];
  if ( control.is_valid_curve_id( curve_id ) )
    {
      unsigned short buffer[ 1024 ];
      control.download_sensitivity_curve( curve_id, buffer );
      
      int buffer_written_ok = write_curve_to_file( curve_filename, buffer );
      if ( buffer_written_ok != 0 )
	{
	  printf( "Downloaded curve %d to file %s\n", 
		  curve_id, curve_filename );
	}
      else
	{
	  printf( "Unable to write curve %d to file %s\n", curve_id, curve_filename );
	}
    }
  else
    {
      printf( "Invalid curve ID for download; valid values are [0..5]\n" );
    }
  return 3;
}

int
parse_reset_sensitivity_curve( int arg_count, 
			       char** arg_vector,
			       int arg_index,
			       orb_control& control )
{
  REQUIRE( strcmp( arg_vector[ arg_index ], "--reset-sensitivity-curve" ) == 0 );
  REQUIRE( arg_count > arg_index + 1 );
  int curve_id = atoi( arg_vector[ arg_index + 1 ] );
  if ( control.is_valid_curve_id( curve_id ) )
    {
      printf( "Resetting sensitivity curve %d\n", curve_id );
      control.upload_sensitivity_curve( curve_id, charts[ curve_id ] );
    }
  else
    {
      printf( "Invalid curve ID for --reset-sensitivity-curve; valid values are [0..5]\n" );
    }
  return 2;
}

int
parse_args( int arg_count,
	    char** arg_vector,
	    orb_control& control )
{
  int result = 0;
  int index = 1;
  while (index < arg_count)
    {
      if ( strcmp( arg_vector[ index ], "--set-axis" ) == 0 )
	{
	  index += parse_set_axis( arg_count, arg_vector, index, control );
	}
      
      else if ( strcmp( arg_vector[ index ], "--set-facing" ) == 0 )
	{
	  index += parse_set_facing( arg_count, arg_vector, index, control );
	}

      else if ( strcmp( arg_vector[ index ], "--set-chording" ) == 0 )
	{
	  index += parse_set_chording( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-null-region" ) == 0 )
	{
	  index += parse_set_null_region( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-sensitivity" ) == 0 )
	{
	  index += parse_set_sensitivity( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-polarity" ) == 0 )
	{
	  index += parse_set_polarity( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-master-sensitivity" ) == 0)
	{
	  index += parse_set_master_sensitivity( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-gain" ) == 0)
	{
	  index += parse_set_gain( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-master-gain" ) == 0)
	{
	  index += parse_set_master_gain( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-precision-sensitivity" ) == 0)
	{
	  index += parse_set_precision_sensitivity( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-precision-gain" ) == 0)
	{
	  index += parse_set_precision_gain( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-precision-button" ) == 0)
	{
	  index += parse_set_precision_button( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--set-defaults" ) == 0 )
	{
	  index += parse_set_defaults( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--upload-sensitivity-curve" ) == 0 )
	{
	  index += parse_upload_sensitivity_curve( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--download-sensitivity-curve" ) == 0 )
	{
	  index += parse_download_sensitivity_curve( arg_count, arg_vector, index, control );
	}
      else if ( strcmp( arg_vector[ index ], "--reset-sensitivity-curve" ) == 0 )
	{
	  index += parse_reset_sensitivity_curve( arg_count, arg_vector, index, control );
	}
      else if ( (strcmp( arg_vector[ index ], "--from-file" ) == 0 ) ||
		(strcmp( arg_vector[ index ], "-f" ) == 0 ))
	{
	  index += parse_read_args_from_file( arg_count, arg_vector, index, control );
	}

      else
	{
	  printf( "orbcontrol does not understand arg: %s\n", arg_vector[index] );
	  ++index;
	  result = -1;
	}
    }
  return result;
}

void
print_usage( void )
{
  printf( "\n\
Usage: \n\
  orbcontrol [commands...] \n\
where [commands] is one or more of \n\
  --set-axis logical-axis physical-axis \n\
  --set-facing ('horizontal' | 'vertical') \n\
  --set-chording ('on' | 'off')\n \
  --set-null-region (0-127)\n\
  --set-sensitivity (axis) (0-5)\n\
  --set-master-sensitivity (0-5)\n\
  --set-polarity (axis) (-|0|+)\n\
  --set-gain (axis) (0-100)\n\
  --set-master-gain (0-100)\n\
  --set-precision-sensitivity (0-5)\n\
  --set-precision-gain (0-100)\n\
  --set-precision-button (none|physical|logical) (index)\n\
  --upload-sensitivity-curve (0-5) filename\n\
  --download-sensitivity-curve (0-5) filename\n\
\
  --set-defaults\n\
  -f filename, --from-file filename\n\
" );
}

int string_count( char* filename )
{
  int result = 0;
  FILE* f;
  f = fopen( filename, "r" );
  while (!feof( f))
    {
      char command[ 256 ];
      fscanf( f, "%s", command );
      if ( !feof( f ) )
	{
	  ++result;
	}
    }
  return result;
}

void
make_arg_vector( char* filename, char*** pbuffer, int* pcount )
{
  int cstrings = string_count( filename );
  FILE* f;
  f = fopen( filename, "r" );

  if ( (cstrings > 0) && (f != NULL) )
    {
      (*pbuffer) = new char*[ cstrings ];
      for ( int i = 0; i < cstrings; ++i )
	{
	  (*pbuffer)[ i ] = new char[ 256 ];
	  fscanf( f, "%s", (*pbuffer)[i] );
	}
      (*pcount) = cstrings;
      fclose( f );
    }
  else
    {
      (*pbuffer) = NULL;
      (*pcount) = 0;
    }
}

void
free_arg_vector( char** buffer, int count )
{
  for( int i = 0; i < count; ++i )
    {
      delete[] buffer[ i ];
    }
  delete[] buffer;
}

void
parse_args_from_file( char* filename, 
		      orb_control& control )
{
  char** buffer = NULL;
  int count = 0;
  make_arg_vector( filename, &buffer, &count );
  if ( (buffer != NULL) )
    {
      printf( "Reading commands from file %s\n", filename );
      parse_args( count, buffer, control );
      free_arg_vector( buffer, count );
    }
  else
    {
      printf( "Unable to read commands from file %s\n", filename );
    }
      
}
      
int
parse_read_args_from_file( int arg_count,
			   char** arg_vector,
			   int arg_index,
			   orb_control& control )
{
  REQUIRE( ( strcmp( arg_vector[ arg_index ], "--from-file" ) == 0 ) ||
	   ( strcmp( arg_vector[ arg_index ], "-f" ) == 0 ));
  REQUIRE( arg_count > arg_index + 1 );
  parse_args_from_file( arg_vector[ arg_index + 1 ], control );
  return 2;
}


void __cdecl
main( int arg_count, char** arg_vector )
{

  orb_control control;
  if ( control.is_initialized() )
    {
      if ( arg_count < 2 )
	{
	  show_orb_status( control );
	}
      else
	{
	  if ( parse_args( arg_count, arg_vector, control ) == -1 )
	    {
	      print_usage();
	    }
	  printf( "\n\nCommands complete.\n" );
	  show_orb_status(control);
	}
    }
  else
    {
      printf( "Could not initialize control; either the orb is not installed\nor the driver is not operating correctly\n\tError: %s\n", control.last_error_string() );
    }
}
	
