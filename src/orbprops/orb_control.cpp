#include "orb_control.h"
#include <windows.h>
extern "C" {
#include <hidsdi.h>
}
#include <setupapi.h>
#ifndef CONTRACT_H
#include "contract.h"
#endif
#ifndef FEATURES_H
#include "features.h"
#endif
extern "C" {
#ifndef VALIDITY_H
#include "validity.h"
#endif
}

#include <stdio.h>
#include <stdlib.h>

class orb_control_implementation
{
 public:
  int is_initialized;
  GUID hid_guid;
  SP_DEVICE_INTERFACE_DATA first_orb;
  HANDLE orb_handle;
  orb_control::error last_error;
  HIDSPORB_FEATURE_PACKET last_report;

  DWORD
  device_interface_details_length( HDEVINFO device_info,
				   PSP_INTERFACE_DEVICE_DATA device_data )
  {
    DWORD result = 0;
    BOOL call_succeeded;
    call_succeeded = SetupDiGetDeviceInterfaceDetail( device_info,
						      device_data,
						      NULL,
						      0,
						      &result,
						      NULL );
    return result;
  }

				    
  PSP_DEVICE_INTERFACE_DETAIL_DATA 
  orphan_get_device_details( HDEVINFO device_info,
			     PSP_INTERFACE_DEVICE_DATA device_data )
  {
    BOOL call_succeeded;
    PSP_DEVICE_INTERFACE_DETAIL_DATA result = NULL;
    DWORD required_length = 0;
    DWORD length = device_interface_details_length( device_info, device_data );
    if ( 0 == length )
      {
	result = NULL;
      }
    else
      {
	result = (PSP_DEVICE_INTERFACE_DETAIL_DATA)(malloc( length ));
	if ( NULL != result )
	  {
	    result->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );
	    call_succeeded = SetupDiGetDeviceInterfaceDetail( device_info,
							      device_data,
							      result,
							      length,
							      &required_length,
							      NULL );
	    if ( !call_succeeded )
	      {
		free( result );
		result = NULL;
	      }
	  }
      }
    return result;
  }

  HANDLE
  device_handle( HDEVINFO device_info, 
		 PSP_INTERFACE_DEVICE_DATA device_data )
  {
    HANDLE result = INVALID_HANDLE_VALUE;
    PSP_DEVICE_INTERFACE_DETAIL_DATA details = orphan_get_device_details( device_info,
									  device_data );
    if ( NULL != details )
      {
	result = CreateFile( details->DevicePath, 
			     GENERIC_READ | GENERIC_WRITE,
			     FILE_SHARE_READ | FILE_SHARE_WRITE,
			     NULL,
			     OPEN_EXISTING,
			     0,
			     NULL );
	free( details );
      }

    return result;
  }

  BOOL
  handle_represents_orb ( HANDLE h )
  {
    BOOL call_succeeded = FALSE;
    HIDD_ATTRIBUTES attributes;
    attributes.Size = sizeof( HIDD_ATTRIBUTES );
    if ( INVALID_HANDLE_VALUE == h )
      {
	return false;
      }
    else
      {
	call_succeeded = HidD_GetAttributes( h, &attributes );
	if ( call_succeeded )
	  {
	    if ( ( attributes.VendorID == HIDSPORB_VENDOR_ID ) 
		 && ( attributes.ProductID == HIDSPORB_PRODUCT_ID )  
		 && ( attributes.VersionNumber == HIDSPORB_VERSION_NUMBER ))
	      
	      {
		return true;
	      }
	    else
	      {
		return false;
	      }
	  }
	else
	  {
	    return false;
	  }
      }
  }


		
  void initialize( void )
  {
    HDEVINFO device_info;
    int i = 0;
    BOOL continue_enum = TRUE;
    BOOL device_found = FALSE;


    is_initialized = FALSE;
    HidD_GetHidGuid( &hid_guid );
    device_info = SetupDiGetClassDevs(&hid_guid,
				      NULL, NULL,
				      DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    first_orb.cbSize = sizeof( first_orb );
    //right now, we'll just get the first orb we see
    while (continue_enum)
      {
	device_found = SetupDiEnumDeviceInterfaces( device_info,
						    NULL,
						    &hid_guid,
						    i,
						    &first_orb );
	if ( device_found )
	  {
	    orb_handle = device_handle( device_info, &first_orb );
	    if ( handle_represents_orb( orb_handle ) )
	      {
		continue_enum = FALSE;
		is_initialized = TRUE;
	      }
	    else
	      {
		continue_enum = TRUE;
		CloseHandle( orb_handle );
		orb_handle = INVALID_HANDLE_VALUE;
	      }
	  }
	else
	  {
	    continue_enum = FALSE;
	  }
	++i;
      }
  }

  void
  get_last_report( void )
  {
    BOOL call_result;
    REQUIRE ( is_initialized );
    last_report.report_id = HIDSPORB_FEATURE_PACKET_ID;
    call_result = HidD_GetFeature( orb_handle, &last_report, HIDSPORB_FEATURE_PACKET_SIZE );
  }

  void
  set_last_report( void )
  {
    BOOL call_result;
    REQUIRE ( is_initialized );
    last_report.report_id = HIDSPORB_FEATURE_PACKET_ID;
    call_result = HidD_SetFeature( orb_handle, &last_report, HIDSPORB_FEATURE_PACKET_SIZE );
  }
    
};

orb_control::orb_control( void )
{
  m_pimp = new orb_control_implementation;
  m_pimp->is_initialized = 0;
  m_pimp->orb_handle = INVALID_HANDLE_VALUE;
  m_pimp->last_error = orb_control::No_error;

  m_pimp->initialize();
}

orb_control::~orb_control( void )
{
  if ( INVALID_HANDLE_VALUE != m_pimp->orb_handle )
    {
      CloseHandle( m_pimp->orb_handle );
    }
  delete ( m_pimp );
}



void
orb_control::set_axis( int logical_axis, int physical_axis )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_axis( logical_axis ) );
  REQUIRE( is_valid_axis( physical_axis ) );

  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.axis_map[ logical_axis ] = (unsigned char)(physical_axis);
  m_pimp->set_last_report();
}

void
orb_control::set_sensitivity( int logical_axis, int sensitivity )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_axis( logical_axis ) );
  REQUIRE( is_valid_sensitivity( sensitivity ) );

  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.sensitivities[ logical_axis ] = (unsigned char)(sensitivity);
  m_pimp->set_last_report();
}

void
orb_control::set_polarity( int logical_axis, int polarity )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_axis( logical_axis ) );
  REQUIRE( is_valid_polarity( polarity ) );

  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.polarities[ logical_axis ] = (unsigned char)(polarity);
  m_pimp->set_last_report();
}

void
orb_control::set_gain( int logical_axis, int gain )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_axis( logical_axis ) );
  REQUIRE( is_valid_gain( gain ) );
  
  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.gains[ logical_axis ] = (unsigned char)(gain);
  m_pimp->set_last_report();
}

void
orb_control::set_precision_sensitivity( int sensitivity )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_sensitivity( sensitivity ) );
  
  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.precision_sensitivity = (unsigned char)(sensitivity);
  m_pimp->set_last_report();
}

void
orb_control::set_precision_gain( int gain )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_gain( gain ) );
  
  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.precision_gain = (unsigned char)(gain);
  m_pimp->set_last_report();
}
	   
void
orb_control::set_precision_button( int button_type, int button_index )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_button_type( button_type ) );
  REQUIRE( is_valid_logical_button_index( button_index ) );

  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.precision_button_type = (unsigned char)(button_type);
  m_pimp->last_report.feature_data.precision_button_index = (unsigned char)(button_index );
  m_pimp->set_last_report();
}


void
orb_control::set_chording( int new_chording )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.use_chording = (unsigned char)(new_chording != 0);
  m_pimp->set_last_report();
}

void
orb_control::set_facing( orb_control::facing new_facing )
{
  int i = 0;
  REQUIRE( is_initialized() );
  REQUIRE( (new_facing == Vertical_facing) || 
	   (new_facing == Horizontal_facing ));
  switch ( new_facing )
    {
    case Vertical_facing : 
      for ( i = 0; i < 6; ++ i )
	{
	  set_axis( i, i );
	  set_polarity( i, 2 );
	}
      break;

    case Horizontal_facing: 
      //swap some axes
      set_axis( 0, 0 );
      set_axis( 1, 2 );
      set_axis( 2, 1 );
      set_axis( 3, 3 );
      set_axis( 4, 5 );
      set_axis( 5, 4 );
      //but when we do this, the polarity of the z and rz axes
      //should be reversed
      for ( i = 0; i < 6; ++i )
	{
	  set_polarity( i, 2 );
	}
      set_polarity( 2, 0 );
      set_polarity( 5, 0 );
      break;
      
    default:
      //should never get here
      CHECK( 0 );
    }

}

void
orb_control::set_null_region( int new_null_region )
{
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_null_region ( new_null_region ) );
  
  m_pimp->get_last_report();
  m_pimp->last_report.feature_data.null_region = (unsigned char)(new_null_region);
  m_pimp->set_last_report();
}

void
orb_control::set_defaults( void )
{
  //just set default settings in
  REQUIRE( is_initialized() );
  
  for (int i = 0; i < 6; ++i )
    {
      m_pimp->last_report.feature_data.axis_map[i] = (unsigned char)i;
      m_pimp->last_report.feature_data.sensitivities[ i ] = 4;
      m_pimp->last_report.feature_data.polarities[ i ] = 2;
      m_pimp->last_report.feature_data.gains[ i ] = 50;
    }
  m_pimp->last_report.feature_data.use_chording = 1;
  m_pimp->last_report.feature_data.null_region = 65;
  m_pimp->last_report.feature_data.precision_sensitivity = 0;
  m_pimp->last_report.feature_data.precision_gain = 50;
  m_pimp->last_report.feature_data.precision_button_type = 0;
  m_pimp->last_report.feature_data.precision_button_index = 0;
  
  m_pimp->set_last_report();
}


void
orb_control::upload_sensitivity_curve( int curve_id,
				       unsigned short* buffer )
{
  //buffer should point to a user-allocated array of 1024 unsigned shorts
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_curve_id( curve_id ) );
  REQUIRE( buffer != NULL );
  
  //first fill a curve data field with the curve data
  HIDSPORB_SENSITIVITY_CURVE_PACKET packet;
  packet.report_id = HIDSPORB_CURVE_PACKET_ID;
  packet.curve.curve_id = (unsigned char)(curve_id);
  memmove( &(packet.curve.curve), buffer, 1024 * sizeof( unsigned short ) );
  //now upload it
  HidD_SetFeature( m_pimp->orb_handle, &packet, HIDSPORB_SENSITIVITY_CURVE_PACKET_SIZE );
}

int
orb_control::is_initialized( void )
{
  return m_pimp->is_initialized;
}


int
orb_control::physical_axis_from_logical_axis( int axis )
{
  REQUIRE( is_initialized() );
  REQUIRE( axis >= 0 );
  REQUIRE( axis < 6 );

  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.axis_map[ axis ];
}

int
orb_control::sensitivity( int axis )
{
  REQUIRE( is_initialized() );
  REQUIRE( axis >= 0 );
  REQUIRE( axis < 6 );

  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.sensitivities[ axis ];
}

int
orb_control::polarity( int axis )
{
  REQUIRE( is_initialized() );
  REQUIRE( axis >= 0 );
  REQUIRE( axis < 6 );

  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.polarities[ axis ];
}

int
orb_control::gain( int axis )
{
  REQUIRE( is_initialized() );
  REQUIRE( axis >= 0 );
  REQUIRE( axis < 6 );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.gains[ axis ];
}

int
orb_control::precision_sensitivity( void )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.precision_sensitivity;
}

int 
orb_control::precision_gain( void )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.precision_gain;
}

int
orb_control::precision_button_type( void )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.precision_button_type;
}

int
orb_control::precision_button_index( void )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.precision_button_index;
}

int
orb_control::using_chording( void )
{
  REQUIRE( is_initialized() );
  
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.use_chording;
}

int
orb_control::null_region( void )
{
  REQUIRE( is_initialized() );
  m_pimp->get_last_report();
  return m_pimp->last_report.feature_data.null_region;
}

orb_control::facing
orb_control::current_facing( void )
{
  REQUIRE( is_initialized() );
  if ( ( physical_axis_from_logical_axis( 0 ) == 0 ) &&
       ( physical_axis_from_logical_axis( 1 ) == 1 ) &&
       ( physical_axis_from_logical_axis( 2 ) == 2 ) &&
       ( physical_axis_from_logical_axis( 3 ) == 3 ) &&
       ( physical_axis_from_logical_axis( 4 ) == 4 ) &&
       ( physical_axis_from_logical_axis( 5 ) == 5 ) &&
       ( polarity( 0 ) == 2 ) &&
       ( polarity( 1 ) == 2 ) &&
       ( polarity( 2 ) == 2 ) &&
       ( polarity( 3 ) == 2 ) &&
       ( polarity( 4 ) == 2 ) &&
       ( polarity( 5 ) == 2 ))
    {
      return Vertical_facing;
    }
  else if ( ( physical_axis_from_logical_axis( 0 ) == 0 ) &&
	    ( physical_axis_from_logical_axis( 1 ) == 2 ) &&
	    ( physical_axis_from_logical_axis( 2 ) == 1 ) &&
	    ( physical_axis_from_logical_axis( 3 ) == 3 ) &&
	    ( physical_axis_from_logical_axis( 4 ) == 5 ) &&
	    ( physical_axis_from_logical_axis( 5 ) == 4 ) &&
	    ( polarity( 0 ) == 2 ) &&
	    ( polarity( 1 ) == 2 ) &&
	    ( polarity( 2 ) == 0 ) &&
	    ( polarity( 3 ) == 2 ) &&
	    ( polarity( 4 ) == 2 ) &&
	    ( polarity( 5 ) == 0 ) )
    {
      return Horizontal_facing;
    }
  else
    {
      return Custom_facing;
    }
}

void
orb_control::download_sensitivity_curve( int curve_id,
				       unsigned short* buffer )
{
  //buffer should point to a user-allocated array of 1024 unsigned shorts
  REQUIRE( is_initialized() );
  REQUIRE( is_valid_curve_id( curve_id ) );
  REQUIRE( buffer != NULL );
  
  //first fill a curve data field with the curve data
  HIDSPORB_SENSITIVITY_CURVE_PACKET packet;
  packet.report_id = HIDSPORB_CURVE_PACKET_ID;
  packet.curve.curve_id = (unsigned char)(curve_id);
  //now upload it
  HidD_GetFeature( m_pimp->orb_handle, &packet, HIDSPORB_SENSITIVITY_CURVE_PACKET_SIZE );
  memmove( buffer, &(packet.curve.curve), 1024 * sizeof( unsigned short ) );
}


int
orb_control::is_valid_axis( int axis )
{
  return ::is_valid_axis( axis );
}
 
int
orb_control::is_valid_sensitivity( int sensitivity )
{
  return ::is_valid_sensitivity( sensitivity );
}

int 
orb_control::is_valid_polarity( int polarity )
{
  return ::is_valid_polarity( polarity );
}

int
orb_control::is_valid_gain( int gain )
{
  return ::is_valid_gain( gain );
}

int
orb_control::is_valid_button_type( int button_type )
{
  return ::is_valid_button_type( button_type );
}

int
orb_control::is_valid_physical_button_index( int button_index ) 
{
  return ::is_valid_physical_button_index( button_index );
}

int
orb_control::is_valid_logical_button_index( int button_index )
{
  return ::is_valid_logical_button_index( button_index );
}

int 
orb_control::is_valid_null_region( int region )
{
  return ::is_valid_null_region( region );
}

int
orb_control::is_valid_curve_id( int curve_id )
{
  return ::is_valid_curve_id( curve_id );
}

int
orb_control::is_valid_curve( unsigned short* buffer )
{
  return ::is_valid_curve( buffer );
}

orb_control::error
orb_control::last_error( void )
{
  return m_pimp->last_error;
}

const char*
orb_control::last_error_string( void )
{
  const char* result = NULL;
  orb_control::error last = last_error();

  switch (last) 
    {
    case No_error : 
      result = "No Error";
      break;
      
    case Could_not_open_base_key :
      result = "Could not open base key";
      break;
      
    case Could_not_open_subkey :
      result = "Could not open subkey";
      break;

    default :
      result = "Unknown error!";
      break;
    }
  return result;
}

