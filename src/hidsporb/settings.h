#if 0
#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef HIDSPORB_H
#include "hidsporb.h"
#endif

#define DRIVER_OBJECT_EXTENSION_REGPATH 1

/* #define HSO_get_registry_path( driver_object ) \ */
/*    (PUNICODE_STRING)IoGetDriverObjectExtension(driver_object, (PVOID) DRIVER_OBJECT_EXTENSION_REGPATH ) */


void
OrbStoreRegistryPath( PDRIVER_OBJECT driver_object,
			 PUNICODE_STRING registry_path );

void
OrbRetrieveSettingsFromRegistry( PDEVICE_OBJECT device_object,
				     PDEVICE_EXTENSION device_extension,
				     HANDLE handle );

NTSTATUS
OrbSetAxisMapping( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
OrbSetSensitivity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
OrbSetPolarity( PDEVICE_EXTENSION device_extension,
		      LONG logical_axis_number,
		      LONG physical_axis_number );

NTSTATUS
OrbSetGain( PDEVICE_EXTENSION device_extension,
	      LONG logical_axis_number,
	      LONG gain );

NTSTATUS
OrbSetPrecisionSensitivity( PDEVICE_EXTENSION device_extension,
			       LONG sensitivity );

NTSTATUS
OrbSetPrecisionGain( PDEVICE_EXTENSION device_extension,
			LONG gain );

NTSTATUS
OrbSetPrecisionButton( PDEVICE_EXTENSION device_extension,
			  LONG button_type,
			  LONG button_index );


NTSTATUS
OrbSetChording( PDEVICE_EXTENSION device_extension,
		  BOOLEAN use_chording );

NTSTATUS
OrbSetNullRegion( PDEVICE_OBJECT device_object,
		     int null_region );

NTSTATUS
OrbSaveSettingsToRegistry( PDEVICE_EXTENSION device_extension );


#endif
#endif
