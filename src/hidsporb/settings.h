#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef HIDSPORB_H
#include "hidsporb.h"
#endif

NTSTATUS
OrbStoreRegistryPath(PDRIVER_OBJECT driverObj,
		PUNICODE_STRING regPath);

// Don't forget to call this!
void
OrbFreeRegistryPath(void);

BOOLEAN
OrbCheckFeature(PHIDSPORB_FEATURE_DATA feature);

void
OrbGetSettingsFromRegistry(PDEVICE_EXTENSION devExt, HANDLE handle);

NTSTATUS
OrbSetAxisMapping(PDEVICE_EXTENSION devExt,
		ULONG logical_axis_number,
		ULONG physical_axis_number);

NTSTATUS
OrbSetSensitivity(PDEVICE_EXTENSION devExt,
		ULONG logical_axis_number,
		ULONG physical_axis_number);

NTSTATUS
OrbSetPolarity(PDEVICE_EXTENSION devExt,
		ULONG logical_axis_number,
		ULONG physical_axis_number);

NTSTATUS
OrbSetGain(PDEVICE_EXTENSION devExt,
	ULONG logical_axis_number,
	ULONG gain);

NTSTATUS
OrbSetPrecisionSensitivity(PDEVICE_EXTENSION devExt, ULONG sensitivity );

NTSTATUS
OrbSetPrecisionGain(PDEVICE_EXTENSION devExt, ULONG gain);

NTSTATUS
OrbSetPrecisionButton(PDEVICE_EXTENSION devExt, ULONG button_type, ULONG button_index);

NTSTATUS
OrbSetChording(PDEVICE_EXTENSION devExt, BOOLEAN use_chording);

NTSTATUS
OrbSetNullRegion(PDEVICE_EXTENSION devExt, int null_region);

NTSTATUS
OrbSaveSettingsToRegistry(PDEVICE_EXTENSION devExt);

#endif
