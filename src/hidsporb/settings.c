//
// settings.c
//
//

#include "hidsporb.h"

// Disabled while merging
#if 0
NTSTATUS
HSO_append_unicode_strings( PUNICODE_STRING target,
			    PUNICODE_STRING source,
			    WCHAR* to_append )
{
  NTSTATUS status = STATUS_SUCCESS;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("Entering append_unicode_string" ));
  if ( !source || !to_append )
    {
      status = STATUS_UNSUCCESSFUL;
    }
  else
    {
      RtlInitUnicodeString( target, NULL );
      target->MaximumLength = source->Length +
	(wcslen( to_append ) * sizeof( WCHAR )) + sizeof (UNICODE_NULL );
      target->Buffer = ExAllocatePoolWithTag( PagedPool,
					      target->MaximumLength,
					      HSO_POOL_TAG );
      if ( !target->Buffer )
	{
	  status = STATUS_UNSUCCESSFUL;
	}
      else
	{
	  RtlZeroMemory( target->Buffer, target->MaximumLength );
	  RtlAppendUnicodeToString( target, source->Buffer );
	  RtlAppendUnicodeToString( target, to_append );
	}
    }
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT,
		"exiting append_unicode_string", status );
  return status;
}

void
HSO_store_registry_path( PDRIVER_OBJECT driver_object,
			 PUNICODE_STRING registry_path )
{
  PUNICODE_STRING registry_path_copy;
  NTSTATUS status;
  PWCHAR have_changed_string = L"settings_have_changed";

  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		( "Entering store_registry_path; path= %ws", registry_path->Buffer ));

  status = HSO_append_unicode_strings( &(Global.registry_base), 
				       registry_path, L"" );
  if ( NT_SUCCESS(status) )
    {
      status = HSO_append_unicode_strings( &(Global.settings_path), 
					   &Global.registry_base,
					   L"\\Settings" );
    }

  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, "Exiting store_registry_path", status );
}


void
HSO_retrieve_settings_from_registry( PDEVICE_OBJECT device_object, 
				     PDEVICE_EXTENSION device_extension,
				     HANDLE handle )
{
  PRTL_QUERY_REGISTRY_TABLE settings = NULL;
  NTSTATUS status = STATUS_SUCCESS;

  //axes
  LONG axes[] = { 0, 1, 2, 3, 4, 5 };
  LONG default_axes[] = { 0, 1, 2, 3, 4, 5 };
  PWCHAR axes_key_names[] = { L"logical_axis_0",
			     L"logical_axis_1",
			     L"logical_axis_2",
			     L"logical_axis_3",
			     L"logical_axis_4",
			     L"logical_axis_5" };
  

  //sensitivities
  LONG sensitivities[] = { 0, 0, 0, 0, 0, 0 };
  LONG default_sensitivities[] = { 0, 0, 0, 0, 0, 0 };
  PWCHAR sensitivities_key_names[] = { L"sensitivity_0",
				    L"sensitivity_1",    
				    L"sensitivity_2",    
				    L"sensitivity_3",    
				    L"sensitivity_4",   
				    L"sensitivity_5" };   

  //polarities
  LONG polarities[] = { HIDSPORB_POLARITY_POSITIVE,
			HIDSPORB_POLARITY_POSITIVE,
			HIDSPORB_POLARITY_POSITIVE,
			HIDSPORB_POLARITY_POSITIVE,
			HIDSPORB_POLARITY_POSITIVE,
			HIDSPORB_POLARITY_POSITIVE };
			
  LONG default_polarities[] = { HIDSPORB_POLARITY_POSITIVE,
				HIDSPORB_POLARITY_POSITIVE,
				HIDSPORB_POLARITY_POSITIVE,
				HIDSPORB_POLARITY_POSITIVE,
				HIDSPORB_POLARITY_POSITIVE,
				HIDSPORB_POLARITY_POSITIVE };

  PWCHAR polarities_key_names[] = { L"polarity_0",
				 L"polarity_1",
				 L"polarity_2",
				 L"polarity_3",
				 L"polarity_4",
				 L"polarity_5" };

  LONG gains[] = { 50, 50, 50, 50, 50, 50 };
  
  LONG default_gains[] = { 50, 50, 50, 50, 50, 50 };

  PWCHAR gains_key_names[] = { L"gain_0",
			       L"gain_1",
			       L"gain_2",
			       L"gain_3",
			       L"gain_4",
			       L"gain_5" };

  LONG use_chording = 1;
  LONG default_use_chording = 0;

  LONG null_region;
  LONG default_null_region = 65;

  LONG precision_sensitivity = 0;
  LONG default_precision_sensitivity = 0;

  LONG precision_gain = 0;
  LONG default_precision_gain = 0;
  
  LONG precision_button_type = 0;
  LONG default_precision_button_type = 0;

  LONG precision_button_index = 0;
  LONG default_precision_button_index = 0;

  USHORT queries_plus_one = 31;
  int i;

  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		( "Entering retrieve_settings_from_registry" ));

  settings = ExAllocatePoolWithTag( PagedPool,
				    sizeof( RTL_QUERY_REGISTRY_TABLE ) * queries_plus_one,
				    HSO_POOL_TAG );
  if ( !settings )
    {
      status = STATUS_UNSUCCESSFUL;
    }
  else
    {
      /* first let's set up the settings query */
      RtlZeroMemory( settings,
		     sizeof( RTL_QUERY_REGISTRY_TABLE ) * queries_plus_one );

      //now set all the axis queries
      for ( i = 0; i < 6; ++i )
	{
	  settings[ i ].Flags = RTL_QUERY_REGISTRY_DIRECT;
	  settings[ i ].Name = axes_key_names[ i ];
	  settings[ i ].EntryContext = &( axes[ i ] );
	  settings[ i ].DefaultType = REG_DWORD;
	  settings[ i ].DefaultData = &( default_axes[ i ] );
	  settings[ i ].DefaultLength = sizeof( LONG );

	  settings[ i+6 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
	  settings[ i+6 ].Name = sensitivities_key_names[ i ];
	  settings[ i+6 ].EntryContext = &( sensitivities[ i ] );
	  settings[ i+6 ].DefaultType = REG_DWORD;
	  settings[ i+6 ].DefaultData = &( default_sensitivities[ i ] );
	  settings[ i+6 ].DefaultLength = sizeof( LONG );

	  settings[ i+12 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
	  settings[ i+12 ].Name = polarities_key_names[ i ];
	  settings[ i+12 ].EntryContext = &( polarities[ i ] );
	  settings[ i+12 ].DefaultType = REG_DWORD;
	  settings[ i+12 ].DefaultData = &( default_polarities[ i ] );
	  settings[ i+12 ].DefaultLength = sizeof( LONG );

	  settings[ i+18 ].Flags = RTL_QUERY_REGISTRY_DIRECT;
	  settings[ i+18 ].Name = gains_key_names[ i ];
	  settings[ i+18 ].EntryContext = &( gains[ i ] );
	  settings[ i+18 ].DefaultType = REG_DWORD;
	  settings[ i+18 ].DefaultData = &( default_gains[ i ] );
	  settings[ i+18 ].DefaultLength = sizeof( LONG );

	}
      
      settings[24].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[24].Name = L"use_chording";
      settings[24].EntryContext = &use_chording;
      settings[24].DefaultType = REG_DWORD;
      settings[24].DefaultData = &default_use_chording;
      settings[24].DefaultLength = sizeof(LONG);

      settings[25].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[25].Name = L"null_region";
      settings[25].EntryContext = &null_region;
      settings[25].DefaultType = REG_DWORD;
      settings[25].DefaultData = &default_null_region;
      settings[25].DefaultLength = sizeof( LONG );

      settings[26].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[26].Name = L"precision_sensitivity";
      settings[26].EntryContext = &precision_sensitivity;
      settings[26].DefaultType = REG_DWORD;
      settings[26].DefaultData = &default_precision_sensitivity;
      settings[26].DefaultLength = sizeof( LONG );

      settings[27].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[27].Name = L"precision_gain";
      settings[27].EntryContext = &precision_gain;
      settings[27].DefaultType = REG_DWORD;
      settings[27].DefaultData = &default_precision_gain;
      settings[27].DefaultLength = sizeof( LONG );

      settings[28].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[28].Name = L"precision_button_type";
      settings[28].EntryContext = &precision_button_type;
      settings[28].DefaultType = REG_DWORD;
      settings[28].DefaultData = &default_precision_button_type;
      settings[28].DefaultLength = sizeof( LONG );

      settings[29].Flags = RTL_QUERY_REGISTRY_DIRECT;
      settings[29].Name = L"precision_button_index";
      settings[29].EntryContext = &precision_button_index;
      settings[29].DefaultType = REG_DWORD;
      settings[29].DefaultData = &default_precision_button_index;
      settings[29].DefaultLength = sizeof( LONG );

      if ( Global.settings_path.Buffer ) //NT_SUCCESS( status ) )
	{
	  HSO_DBGPRINT( FILE_SETTINGS | HSO_BABBLE,
			("Retrieving settings with path: %ws", Global.settings_path.Buffer ));
	  
	  /* now make the actual query */
	  status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
					  Global.settings_path.Buffer,
					  settings,
					  NULL,
					  NULL );
	}

      if ( !NT_SUCCESS( status ) )
	/* there's been a problem; try querying using the handle */
	{
	  HSO_DBGPRINT( FILE_SETTINGS | HSO_ERROR,
			( "Could not query registry settings settings_path"));
	  if ( handle )
	    {
	      status = RtlQueryRegistryValues( RTL_REGISTRY_HANDLE,
					       (PWSTR) handle,
					       settings,
					       NULL,
					       NULL );
	    }
	}
    }
  /* by now, if we're unsuccessful, something is broken, and we
     should just use our default settings */
  if ( !NT_SUCCESS(status) )
    {
      for ( i = 0; i < 6; ++i )
	{
	  axes[ i ] = default_axes[ i ];
	  sensitivities[ i ] = default_sensitivities[ i ];
	  polarities[ i ] = default_polarities[ i ];
	  gains[ i ] = default_gains[ i ];
	}
      use_chording = default_use_chording;
      null_region = default_null_region;
      precision_sensitivity = default_precision_sensitivity;
      precision_gain = default_precision_gain;
      precision_button_type = default_precision_button_type;
      precision_button_index = default_precision_button_index;
    }
  /* Okay.  Whatever happened, by the time we reach here we should
     have good data in logical_axis_* and use_chording.  Apply that
     data to the device extension */
  for ( i = 0; i < 6; ++i )
    {
      device_extension->axis_map[ i ] = axes[ i ];
      device_extension->sensitivities[ i ] = sensitivities[ i ];
      device_extension->polarities[ i ] = polarities[ i ];
      device_extension->gains[ i ] = gains[ i ];
    }
  device_extension->use_chording = ( use_chording != 0 );
  HSO_set_null_region( device_object, null_region );
  device_extension->precision_sensitivity = precision_sensitivity;
  device_extension->precision_gain = precision_gain;
  device_extension->precision_button_type = precision_button_type;
  device_extension->precision_button_index = precision_button_index;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_BABBLE,
		( "Axis map: %d/%d/%d/%d/%d/%d, use_chording = %d",
		  axes[ 0 ], axes[ 1 ], axes[ 2 ], 
		  axes[ 3 ], axes[ 4 ], axes[ 5 ],
		  use_chording ));
  
  if ( settings )
    {
      ExFreePool( settings );
    }
  // we save settings here so that orbcontrol will work on initial
  // installation
  HSO_save_settings( device_extension );
}

NTSTATUS 
HSO_write_registry_long( PWCHAR key, LONG value )
{
  NTSTATUS status;
  if ( Global.settings_path.Buffer )
    {
      status = RtlWriteRegistryValue( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
				      Global.settings_path.Buffer,
				      key,
				      REG_DWORD,
				      &value,
				      sizeof( LONG ) );
    }
  else
    {
      status = STATUS_UNSUCCESSFUL;
    }
  return status;
}

// Todo: add checks for validness
NTSTATUS
OrbSetAxisMapping(PDEVICE_EXTENSION devExt, LONG logical_axis_number, LONG physical_axis_number)
{
	NTSTATUS status = STATUS_SUCCESS;
	WCHAR* axis_setting_names[] = { L"logical_axis_0",
					L"logical_axis_1",
					L"logical_axis_2",
					L"logical_axis_3",
					L"logical_axis_4",
					L"logical_axis_5" };


  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_axis_mapping"));
		       
  /* first things first--set this in the device extension */
  device_extension->axis_map[ logical_axis_number ] = physical_axis_number;
  /* now write it in the registry */
  status = HSO_write_registry_long( axis_setting_names[ logical_axis_number ],
				    physical_axis_number );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		"exiting set_axis_mapping", 
		status );
  return status;
}
		
NTSTATUS
HSO_set_sensitivity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG sensitivity )
{
/*   UNICODE_STRING settings_path; */
  NTSTATUS status = STATUS_SUCCESS;
  WCHAR* axis_setting_names[] = { L"sensitivity_0",
				 L"sensitivity_1",
				 L"sensitivity_2",
				 L"sensitivity_3",
				 L"sensitivity_4",
				 L"sensitivity_5" };


  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_sensitivity"));
		       
  /* first things first--set this in the device extension */
  device_extension->sensitivities[ logical_axis_number ] = sensitivity;
  status = HSO_write_registry_long( axis_setting_names[ logical_axis_number ],
				    sensitivity );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		"exiting set_sensitivity", 
		status );
  return status;
}

NTSTATUS
HSO_set_polarity( PDEVICE_EXTENSION device_extension,
		  LONG logical_axis_number,
		  LONG polarity )
{
/*   UNICODE_STRING settings_path; */
  NTSTATUS status = STATUS_SUCCESS;
  WCHAR* axis_setting_names[] = { L"polarity_0",
				 L"polarity_1",
				 L"polarity_2",
				 L"polarity_3",
				 L"polarity_4",
				 L"polarity_5" };


  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_polarity"));
		       
  /* first things first--set this in the device extension */
  device_extension->polarities[ logical_axis_number ] = polarity;
  status = HSO_write_registry_long( axis_setting_names[ logical_axis_number ],
				    polarity );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		"exiting set_polarity", 
		status );
  return status;
}

NTSTATUS
HSO_set_gain( PDEVICE_EXTENSION device_extension,
		  LONG logical_axis_number,
		  LONG gain )
{
/*   UNICODE_STRING settings_path; */
  NTSTATUS status = STATUS_SUCCESS;
  WCHAR* axis_setting_names[] = { L"gain_0",
				 L"gain_1",
				 L"gain_2",
				 L"gain_3",
				 L"gain_4",
				 L"gain_5" };


  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_gain"));
		       
  /* first things first--set this in the device extension */
  device_extension->gains[ logical_axis_number ] = gain;
  /* now write it in the registry */
  status = HSO_write_registry_long( axis_setting_names[ logical_axis_number ],
				    gain );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		"exiting set_gain", 
		status );
  return status;
}

NTSTATUS
HSO_set_precision_sensitivity( PDEVICE_EXTENSION device_extension,
			       LONG sensitivity )
{
  NTSTATUS status = STATUS_SUCCESS;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_precision_sensitivity"));
  device_extension->precision_sensitivity = sensitivity;
  status = HSO_write_registry_long( L"precision_sensitivity", sensitivity );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT,
		"exiting set_precision_sensitivity",
		status );
  return status;
}

NTSTATUS
HSO_set_precision_gain( PDEVICE_EXTENSION device_extension,
			LONG gain )
{
  NTSTATUS status = STATUS_SUCCESS;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_precision_gain"));
  device_extension->precision_gain = gain;
  status = HSO_write_registry_long( L"precision_gain", gain );
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT,
		"exiting set_precision_gain",
		status );
  return status;
}

NTSTATUS
HSO_set_precision_button( PDEVICE_EXTENSION device_extension,
			  LONG button_type,
			  LONG button_index )
{
  NTSTATUS status = STATUS_SUCCESS;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_precision_button"));
  device_extension->precision_button_type = button_type;
  device_extension->precision_button_index = button_index;
  status = HSO_write_registry_long( L"precision_button_type", button_type );
  if ( NT_SUCCESS( status ) )
    {
      status = HSO_write_registry_long( L"precision_button_index", button_index );
    }
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT,
		"exiting set_precision_button",
		status );
  return status;
}


NTSTATUS
HSO_set_chording( PDEVICE_EXTENSION device_extension,
		  BOOLEAN use_chording )
{
/*   UNICODE_STRING settings_path; */
  NTSTATUS status = STATUS_SUCCESS;
  LONG use_chording_long;
  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_use_chording"));
  

  /* first things first--set this in the device extension */
  use_chording_long = use_chording ? 1 : 0;
  device_extension->use_chording = use_chording;

  /* now write it in the registry */
/*   HSO_get_registry_settings_string( device_extension, &settings_path ); */
  if ( Global.settings_path.Buffer )
    {
      status =RtlWriteRegistryValue( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
				     Global.settings_path.Buffer,
				     L"use_chording",
				     REG_DWORD,
				     &use_chording_long,
				     sizeof( LONG ) );
    }
  else
    {
      status = STATUS_UNSUCCESSFUL;
    }
  HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		"exiting set_use_chording", 
		status );
  return status;
  
}
			 
  
NTSTATUS
HSO_save_settings( PDEVICE_EXTENSION device_extension )
{
  NTSTATUS status = STATUS_SUCCESS;
  int i;
  for ( i = 0; i < MAX_LOGICAL_AXES; ++i )
    {
      HSO_set_axis_mapping( device_extension, 
			    i,
			    device_extension->axis_map[ i ] );
      HSO_set_sensitivity( device_extension, i,
			   device_extension->sensitivities[ i ] );
      HSO_set_polarity( device_extension, i, 
			device_extension->polarities[ i ] );
      HSO_set_gain( device_extension, i, 
		    device_extension->gains[ i ] );
    }
  HSO_set_chording( device_extension, device_extension->use_chording );
  HSO_set_precision_sensitivity( device_extension, device_extension->precision_sensitivity );
  HSO_set_precision_gain( device_extension, device_extension->precision_gain );
  HSO_set_precision_button( device_extension, 
			    device_extension->precision_button_type,
			    device_extension->precision_button_index );
  return status;
}
	

#define SETTINGS_HAVE_CHANGED_KEY L"settings_have_changed"
#define MAX_POLLS_BETWEEN_SETTINGS_CHECKS 30

BOOLEAN
HSO_settings_have_changed( void )
{
  //checks the "settings have changed" flag for a nonzero indication
  NTSTATUS status = STATUS_SUCCESS;
  RTL_QUERY_REGISTRY_TABLE query_table[2];
  LONG settings_have_changed;
  LONG default_has_changed = 0;
  RtlZeroMemory( &query_table, sizeof( RTL_QUERY_REGISTRY_TABLE )*2 );
  query_table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
  query_table[0].Name = SETTINGS_HAVE_CHANGED_KEY;
  query_table[0].EntryContext = &settings_have_changed;
  query_table[0].DefaultType = REG_DWORD;
  query_table[0].DefaultData = &default_has_changed;
  query_table[0].DefaultLength = sizeof( LONG );
  status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
				   Global.settings_path.Buffer,
				   query_table,
				   NULL,
				   NULL );
  if ( NT_SUCCESS( status ) )
    {
      return ( settings_have_changed != 0 );
    }
  else
    {
      return FALSE;
    }
}
		 

NTSTATUS
HSO_set_null_region( PDEVICE_OBJECT device_object,
		     int new_null_region )
{
  NTSTATUS status = STATUS_SUCCESS;
  LONG null_long;

  HSO_DBGPRINT( FILE_SETTINGS | HSO_FENTRY,
		("entering set_null_region"));
  //if it's in range (7 bits: 0-128) set it and record.
  if ( ( 0 <= new_null_region ) && ( new_null_region < 129 ) )
    {
      //set a pending null region...
      GET_MINIDRIVER_DEVICE_EXTENSION( device_object )->null_region = new_null_region;
      GET_MINIDRIVER_DEVICE_EXTENSION( device_object )->new_null_region_pending = TRUE;
      //then record the data
      if ( Global.settings_path.Buffer )
	{
	  null_long = new_null_region;
	  status =RtlWriteRegistryValue( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
					 Global.settings_path.Buffer,
					 L"null_region",
					 REG_DWORD,
					 &null_long,
					 sizeof( LONG ) );
	}
      else
	{
	  status = STATUS_UNSUCCESSFUL;
	}
      HSO_EXITPROC( FILE_SETTINGS | HSO_FEXIT, 
		    "exiting set_null_region", 
		    status );
    }
  else
    {
      status = STATUS_INVALID_PARAMETER;
    }
  return status;
     
}

#endif	// Disabled while merging
