#if 0 // disabled while merging
#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif

#define DRIVER_OBJECT_EXTENSION_REGPATH 1

/* #define HSO_get_registry_path( driver_object ) \ */
/*    (PUNICODE_STRING)IoGetDriverObjectExtension(driver_object, (PVOID) DRIVER_OBJECT_EXTENSION_REGPATH ) */


void
HSO_store_registry_path( PDRIVER_OBJECT driver_object,
			 PUNICODE_STRING registry_path );

void
HSO_retrieve_settings_from_registry( PDEVICE_OBJECT device_object,
				     PDEVICE_EXTENSION device_extension,
				     HANDLE handle );

NTSTATUS
HSO_set_axis_mapping( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_sensitivity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_polarity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
HSO_set_gain( PDEVICE_EXTENSION device_extension,
	      LONG logical_axis_number,
	      LONG gain );

NTSTATUS
HSO_set_precision_sensitivity( PDEVICE_EXTENSION device_extension,
			       LONG sensitivity );

NTSTATUS
HSO_set_precision_gain( PDEVICE_EXTENSION device_extension,
			LONG gain );

NTSTATUS
HSO_set_precision_button( PDEVICE_EXTENSION device_extension,
			  LONG button_type,
			  LONG button_index );


NTSTATUS
HSO_set_chording( PDEVICE_EXTENSION device_extension,
		  BOOLEAN use_chording );

NTSTATUS
HSO_set_null_region( PDEVICE_OBJECT device_object,
		     int null_region );

NTSTATUS
HSO_save_settings( PDEVICE_EXTENSION device_extension );


#endif
#endif
