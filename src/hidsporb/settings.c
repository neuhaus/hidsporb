//
// settings.c
// SpaceOrb registry settings support
//
//

#include "hidsporb.h"

static
UNICODE_STRING OrbRegistryBase, OrbSettingsPath;

PWCHAR
OrbGetRegistryBase(VOID)
{
	return OrbRegistryBase.Buffer;
}

PWCHAR
OrbGetSettingsPath(VOID)
{
	return OrbSettingsPath.Buffer;
}

// Check feature data
BOOLEAN
OrbCheckFeature(PHIDSPORB_FEATURE_DATA feature)
{
	int i;
	BOOLEAN valid = TRUE;

	// Check everything for validness
	for (i = 0; i < 6; i++) {
		// Check axis map
		if (!OrbIsValidAxis(feature->axis_map[i])) {
			valid = FALSE;
			break;
		}
		// Check sensitivity
		if (!OrbIsValidSensitivity(feature->sensitivities[i])) {
			valid = FALSE;
			break;
		}
		// Check polarity
		if (!OrbIsValidPolarity(feature->polarities[i])) {
			valid = FALSE;
			break;
		}
		// Check gain
		if (!OrbIsValidGain(feature->gains[i])) {
			valid = FALSE;
			break;
		}
		// Check null region
		if (!OrbIsValidNullRegion(feature->null_region)) {
			valid = FALSE;
			break;
		}
		// Check precision button
		if (!OrbIsValidPhysicalButtonIndex(feature->precision_button_index)) {
			valid = FALSE;
			break;
		}
		// Check precision gain
		if (!OrbIsValidGain(feature->precision_gain)) {
			valid = FALSE;
			break;
		}
		// TODO: add other checks?
	}

	return valid;
}

NTSTATUS
OrbAppendUnicodeStrings(PUNICODE_STRING target,
			PUNICODE_STRING source,
			WCHAR* to_append)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	DbgOut(ORB_DBG_SET, ("OrbAppendUnicodeString(): enter\n"));
	// Sanity
	if (!source || !to_append) {
		goto out;
	}
	RtlInitUnicodeString(target, NULL);
	// Calculate target length
	target->MaximumLength = source->Length +
			(wcslen(to_append) * sizeof(WCHAR)) + sizeof(UNICODE_NULL);
	// Allocate string
	target->Buffer = ExAllocatePoolWithTag(PagedPool, target->MaximumLength, HIDSPORB_TAG);
	// Fail if we couldn't
	if (!target->Buffer) {
		goto out;
	}
	// Copy and append
	RtlZeroMemory(target->Buffer, target->MaximumLength);
	RtlAppendUnicodeToString(target, source->Buffer);
	RtlAppendUnicodeToString(target, to_append);
	// OK, we did it
	status = STATUS_SUCCESS;
out:

	return status;
}

NTSTATUS
OrbStoreRegistryPath(PDRIVER_OBJECT driverObj,
		PUNICODE_STRING regPath)
{
	PUNICODE_STRING registry_path_copy;
	NTSTATUS status;
	PWCHAR have_changed_string = L"settings_have_changed";

	DbgOut(ORB_DBG_SET, ("OrbStoreRegPath(): enter, regpath= %ws\n", regPath->Buffer));

	OrbRegistryBase.Buffer = NULL;
	OrbSettingsPath.Buffer = NULL;
	// Save path
	status = OrbAppendUnicodeStrings(&(OrbRegistryBase),
					regPath, L"");
	if (NT_SUCCESS(status)) {
		status = OrbAppendUnicodeStrings(&OrbSettingsPath,
						&OrbRegistryBase,
						L"\\Settings");
	}
	// Free everything if failed
	if (!NT_SUCCESS(status)) {
		OrbFreeRegistryPath();
	}

	return status;
}

void
OrbFreeRegistryPath(void)
{
	if (OrbRegistryBase.Buffer) {
		ExFreePool(OrbRegistryBase.Buffer);
	}
	if (OrbSettingsPath.Buffer) {
		ExFreePool(OrbSettingsPath.Buffer);
	}
	OrbRegistryBase.Buffer = NULL;
	OrbSettingsPath.Buffer = NULL;
}

// was OrbRetrieve*()
void
OrbGetSettingsFromRegistry(PDEVICE_EXTENSION devExt,
			HANDLE handle)
{
	PRTL_QUERY_REGISTRY_TABLE settings = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	// New way
	HIDSPORB_FEATURE_DATA feature;
	// Axes
//	LONG axes[] = { 0, 1, 2, 3, 4, 5 };
	LONG default_axes[] = { 0, 1, 2, 3, 4, 5 };
	PWCHAR axes_key_names[] = { L"logical_axis_0",
				L"logical_axis_1",
				L"logical_axis_2",
				L"logical_axis_3",
				L"logical_axis_4",
				L"logical_axis_5" };
	// Sensitivities
//	LONG sensitivities[] = { 0, 0, 0, 0, 0, 0 };
	LONG default_sensitivities[] = { 0, 0, 0, 0, 0, 0 };
	PWCHAR sensitivities_key_names[] = { L"sensitivity_0",
					L"sensitivity_1",
					L"sensitivity_2",
					L"sensitivity_3",
					L"sensitivity_4",
					L"sensitivity_5" };
	// Polarities
//	LONG polarities[] = { HIDSPORB_POLARITY_POSITIVE,
//			HIDSPORB_POLARITY_POSITIVE,
//			HIDSPORB_POLARITY_POSITIVE,
//			HIDSPORB_POLARITY_POSITIVE,
//			HIDSPORB_POLARITY_POSITIVE,
//			HIDSPORB_POLARITY_POSITIVE };
			
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

//	LONG gains[] = { 50, 50, 50, 50, 50, 50 };

	LONG default_gains[] = { 50, 50, 50, 50, 50, 50 };

	PWCHAR gains_key_names[] = { L"gain_0",
				L"gain_1",
				L"gain_2",
				L"gain_3",
				L"gain_4",
				L"gain_5" };

//	LONG use_chording = 1;
	LONG default_use_chording = 0;

//	LONG null_region;
	LONG default_null_region = 65;

//	LONG precision_sensitivity = 0;
	LONG default_precision_sensitivity = 0;

//	LONG precision_gain = 0;
	LONG default_precision_gain = 0;
  
//	LONG precision_button_type = 0;
	LONG default_precision_button_type = 0;

//	LONG precision_button_index = 0;
	LONG default_precision_button_index = 0;

	USHORT queries_plus_one = 31;
	PWCHAR keyName;
	BOOLEAN valid = FALSE;
	int i;

	DbgOut(ORB_DBG_SET, ("OrbGetSetFromReg(): enter\n"));
	// Get settings key name
	keyName = OrbGetSettingsPath();
	// Fail if none
	if (keyName == NULL) {
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}
	// Alloc space for settings
	settings = ExAllocatePoolWithTag(PagedPool,
					sizeof(RTL_QUERY_REGISTRY_TABLE) * queries_plus_one,
					HIDSPORB_TAG);
	// Fail if we couldn't
	if (!settings) {
		goto out;
	}
	// first let's set up the settings query
	RtlZeroMemory(settings,
		sizeof(RTL_QUERY_REGISTRY_TABLE) * queries_plus_one);
	// Zero & initialize feature data
	RtlZeroMemory(&feature, sizeof(feature));
	feature.use_chording = 1;
	// now set everything up for query
	for (i = 0; i < 6; i++) {
		// Axes
		feature.axis_map[i] = (UCHAR) i;
		settings[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
		settings[i].Name = axes_key_names[i];
		settings[i].EntryContext = &(feature.axis_map[i]);
		settings[i].DefaultType = REG_DWORD;
		settings[i].DefaultData = &(default_axes[i]);
		settings[i].DefaultLength = sizeof(LONG);
		// Sensitivities
		feature.sensitivities[i] = 0;
		settings[i+6].Flags = RTL_QUERY_REGISTRY_DIRECT;
		settings[i+6].Name = sensitivities_key_names[i];
		settings[i+6].EntryContext = &(feature.sensitivities[i]);
		settings[i+6].DefaultType = REG_DWORD;
		settings[i+6].DefaultData = &(default_sensitivities[i]);
		settings[i+6].DefaultLength = sizeof(LONG);
		// Polarities
		feature.polarities[i] = HIDSPORB_POLARITY_POSITIVE;
		settings[i+12].Flags = RTL_QUERY_REGISTRY_DIRECT;
		settings[i+12].Name = polarities_key_names[i];
		settings[i+12].EntryContext = &(feature.polarities[i]);
		settings[i+12].DefaultType = REG_DWORD;
		settings[i+12].DefaultData = &(default_polarities[i]);
		settings[i+12].DefaultLength = sizeof(LONG);
		// Gains
		feature.gains[i] = 50;
		settings[i+18].Flags = RTL_QUERY_REGISTRY_DIRECT;
		settings[i+18].Name = gains_key_names[i];
		settings[i+18].EntryContext = &(feature.gains[i]);
		settings[i+18].DefaultType = REG_DWORD;
		settings[i+18].DefaultData = &(default_gains[i]);
		settings[i+18].DefaultLength = sizeof(LONG);
	}
	// Set up misc stuff
	settings[24].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[24].Name = L"use_chording";
	settings[24].EntryContext = &feature.use_chording;
	settings[24].DefaultType = REG_DWORD;
	settings[24].DefaultData = &default_use_chording;
	settings[24].DefaultLength = sizeof(LONG);

	settings[25].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[25].Name = L"null_region";
	settings[25].EntryContext = &feature.null_region;
	settings[25].DefaultType = REG_DWORD;
	settings[25].DefaultData = &default_null_region;
	settings[25].DefaultLength = sizeof(LONG);

	settings[26].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[26].Name = L"precision_sensitivity";
	settings[26].EntryContext = &feature.precision_sensitivity;
	settings[26].DefaultType = REG_DWORD;
	settings[26].DefaultData = &default_precision_sensitivity;
	settings[26].DefaultLength = sizeof(LONG);

	settings[27].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[27].Name = L"precision_gain";
	settings[27].EntryContext = &feature.precision_gain;
	settings[27].DefaultType = REG_DWORD;
	settings[27].DefaultData = &default_precision_gain;
	settings[27].DefaultLength = sizeof(LONG);

	settings[28].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[28].Name = L"precision_button_type";
	settings[28].EntryContext = &feature.precision_button_type;
	settings[28].DefaultType = REG_DWORD;
	settings[28].DefaultData = &default_precision_button_type;
	settings[28].DefaultLength = sizeof(LONG);

	settings[29].Flags = RTL_QUERY_REGISTRY_DIRECT;
	settings[29].Name = L"precision_button_index";
	settings[29].EntryContext = &feature.precision_button_index;
	settings[29].DefaultType = REG_DWORD;
	settings[29].DefaultData = &default_precision_button_index;
	settings[29].DefaultLength = sizeof(LONG);

	DbgOut(ORB_DBG_SET, ("OrbGetSetReg(): retrieving settings with path: %ws\n", keyName));
	// now make the actual query
	status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
					keyName,
					settings,
					NULL,
					NULL);
	// Use handle to query if failed
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_SET, ("OrbGetSetReg(): could not query registry settings settings_path\n"));
		if (handle) {
			status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
							(PWSTR) handle,
							settings,
							NULL,
							NULL);
		}
	}
	// Assume everything is valid
	valid = !OrbCheckFeature(&feature);
	// By now, if we're unsuccessful, something is broken, and we
	// should just use our default settings
	if (!NT_SUCCESS(status) || !valid) {
		for (i = 0; i < 6; ++i)	{
			feature.axis_map[i] = (UCHAR) default_axes[i];
			feature.sensitivities[i] = (UCHAR) default_sensitivities[i];
			feature.polarities[i] = (UCHAR) default_polarities[i];
			feature.gains[i] = (UCHAR) default_gains[i];
		}
		feature.use_chording = (UCHAR) default_use_chording;
		feature.null_region = (UCHAR) default_null_region;
		feature.precision_sensitivity = (UCHAR) default_precision_sensitivity;
		feature.precision_gain = (UCHAR) default_precision_gain;
		feature.precision_button_type = (UCHAR) default_precision_button_type;
		feature.precision_button_index = (UCHAR) default_precision_button_index;
	}
	// Okay.  Whatever happened, by the time we reach here we should
	// have good data in logical_axis_* and use_chording.  Apply that
	// data to the device extension
	for (i = 0; i < 6; ++i) {
		devExt->orbData.AxisMap[i] = feature.axis_map[i];
		devExt->orbData.sensitivities[i] = feature.sensitivities[i];
		devExt->orbData.polarities[i] = feature.polarities[i];
		devExt->orbData.gains[i] = feature.gains[i];
	}
	devExt->orbData.use_chording = (feature.use_chording != 0);
	OrbSetNullRegion(devExt, feature.null_region);
	devExt->orbData.precision_sensitivity = feature.precision_sensitivity;
	devExt->orbData.precision_gain = feature.precision_gain;
	devExt->orbData.precision_button_type = feature.precision_button_type;
	devExt->orbData.precision_button_index = feature.precision_button_index;
	DbgOut(ORB_DBG_SET, ("OrbGetSetReg(): Axis map: %d/%d/%d/%d/%d/%d, use_chording = %d\n",
			feature.axis_map[0], feature.axis_map[1], feature.axis_map[2],
			feature.axis_map[3], feature.axis_map[4], feature.axis_map[5],
			feature.use_chording));
out:
	// Sanity
	if (settings) {
		ExFreePool(settings);
	}
	// we save settings here so that orbcontrol will work on initial
	// installation
	OrbSaveSettingsToRegistry(devExt);
}

NTSTATUS 
OrbWriteRegistryLong(PWCHAR key, LONG value)
{
	NTSTATUS status;
	PWCHAR keyName;

	// Get settings key name
	keyName = OrbGetSettingsPath();
	// Fail if none
	if (keyName == NULL) {
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}
	// Write value
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
					keyName,
					key,
					REG_DWORD,
					&value,
					sizeof(LONG));
out:

	return status;
}

// Todo: add checks for validness
NTSTATUS
OrbSetAxisMapping(PDEVICE_EXTENSION devExt, ULONG logical_axis_number, ULONG physical_axis_number)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR* axis_setting_names[] = { L"logical_axis_0",
					L"logical_axis_1",
					L"logical_axis_2",
					L"logical_axis_3",
					L"logical_axis_4",
					L"logical_axis_5" };

	DbgOut(ORB_DBG_SET, ("OrbSetAxisMapping(): enter\n"));
	// Sanity check
	if (!OrbIsValidAxis(logical_axis_number) || !OrbIsValidAxis(physical_axis_number)) {
		goto out;
	}
	// first things first--set this in the device extension */
	devExt->orbData.AxisMap[logical_axis_number] = physical_axis_number;
	// now write it in the registry
	status = OrbWriteRegistryLong(axis_setting_names[logical_axis_number],
				    physical_axis_number);
out:

	return status;
}
		
NTSTATUS
OrbSetSensitivity(PDEVICE_EXTENSION devExt,
		ULONG logical_axis_number,
		ULONG sensitivity)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR* axis_setting_names[] = { L"sensitivity_0",
					L"sensitivity_1",
					L"sensitivity_2",
					L"sensitivity_3",
					L"sensitivity_4",
					L"sensitivity_5" };

	DbgOut(ORB_DBG_SET, ("OrbSetSens(): enter\n"));
	if (!OrbIsValidAxis(logical_axis_number) || !OrbIsValidSensitivity(sensitivity)) {
		goto out;
	}
	// first things first--set this in the device extension
  	devExt->orbData.sensitivities[logical_axis_number] = sensitivity;
	status = OrbWriteRegistryLong(axis_setting_names[logical_axis_number],
					sensitivity);
out:

	return status;
}

NTSTATUS
OrbSetPolarity(PDEVICE_EXTENSION devExt,
		ULONG logical_axis_number,
		ULONG polarity)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR* axis_setting_names[] = { L"polarity_0",
					L"polarity_1",
					L"polarity_2",
					L"polarity_3",
					L"polarity_4",
					L"polarity_5" };

	DbgOut(ORB_DBG_SET, ("OrbSetPolarity(): enter\n"));
	// Sanity check
	if (!OrbIsValidAxis(logical_axis_number) || !OrbIsValidPolarity(polarity)) {
		goto out;
	}
	// first things first--set this in the device extension
	devExt->orbData.polarities[logical_axis_number] = polarity;
	status = OrbWriteRegistryLong(axis_setting_names[logical_axis_number],
					polarity);
out:

	return status;
}

NTSTATUS
OrbSetGain(PDEVICE_EXTENSION devExt,
	ULONG logical_axis_number,
	ULONG gain)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR* axis_setting_names[] = { L"gain_0",
					L"gain_1",
					L"gain_2",
					L"gain_3",
					L"gain_4",
					L"gain_5" };

	DbgOut(ORB_DBG_SET, ("OrbSetGain(): enter\n"));
	// Sanity check
	if (!OrbIsValidAxis(logical_axis_number) || !OrbIsValidGain(gain)) {
		goto out;
	}
	// first things first--set this in the device extension
	devExt->orbData.gains[logical_axis_number] = gain;
	// now write it in the registry
	status = OrbWriteRegistryLong(axis_setting_names[logical_axis_number],
					gain);
out:

	return status;
}

NTSTATUS
OrbSetPrecisionSensitivity(PDEVICE_EXTENSION devExt,
			ULONG sensitivity)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	DbgOut(ORB_DBG_SET, ("OrbSetPrecSens(): enter\n"));
	// Sanity check
	if (!OrbIsValidSensitivity(sensitivity)) {
		goto out;
	}
	// Set new precision sensitivity
	devExt->orbData.precision_sensitivity = sensitivity;
	status = OrbWriteRegistryLong(L"precision_sensitivity", sensitivity);
out:

	return status;
}

NTSTATUS
OrbSetPrecisionGain(PDEVICE_EXTENSION devExt,
		ULONG gain)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	DbgOut(ORB_DBG_SET, ("OrbSetPrecGain(): enter\n"));
	// Sanity check
	if (!OrbIsValidGain(gain)) {
		goto out;
	}
	// Set new precision gain
	devExt->orbData.precision_gain = gain;
	status = OrbWriteRegistryLong(L"precision_gain", gain);
out:

	return status;
}

NTSTATUS
OrbSetPrecisionButton(PDEVICE_EXTENSION devExt,
			ULONG button_type,
			ULONG button_index)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	DbgOut(ORB_DBG_SET,("OrbSetPrecBtn(): enter\n"));
	// Sanity check
	if (!OrbIsValidPhysicalButtonIndex(button_index)) {
		goto out;
	}
	// Set new button type & index
	devExt->orbData.precision_button_type = button_type;
  	devExt->orbData.precision_button_index = button_index;
	status = OrbWriteRegistryLong(L"precision_button_type", button_type);
	if (NT_SUCCESS(status)) {
		status = OrbWriteRegistryLong(L"precision_button_index", button_index);
	}
out:

	return status;
}


NTSTATUS
OrbSetChording(PDEVICE_EXTENSION devExt,
		BOOLEAN use_chording)
{
	NTSTATUS status;
	LONG use_chording_long;

	DbgOut(ORB_DBG_SET, ("OrbSetChording(): enter\n"));
 	// first things first--set this in the device extension
  	use_chording_long = use_chording ? 1 : 0;
	devExt->orbData.use_chording = use_chording;
	status = OrbWriteRegistryLong(L"use_chording", use_chording_long);

	return status;
}
			 
  
NTSTATUS
OrbSaveSettingsToRegistry(PDEVICE_EXTENSION devExt)
{
	NTSTATUS status = STATUS_SUCCESS;
	int i;

	for (i = 0; i < ORB_NUM_AXES; ++i) {
		OrbSetAxisMapping(devExt, i, devExt->orbData.AxisMap[i]);
		OrbSetSensitivity(devExt, i, devExt->orbData.sensitivities[i]);
		OrbSetPolarity(devExt, i, devExt->orbData.polarities[i]);
		OrbSetGain(devExt, i, devExt->orbData.gains[i]);
	}
	OrbSetChording(devExt, devExt->orbData.use_chording);
	OrbSetPrecisionSensitivity(devExt, devExt->orbData.precision_sensitivity);
	OrbSetPrecisionGain(devExt, devExt->orbData.precision_gain);
	OrbSetPrecisionButton(devExt, devExt->orbData.precision_button_type,
				devExt->orbData.precision_button_index);

	return status;
}
	
NTSTATUS
OrbSetNullRegion(PDEVICE_EXTENSION devExt,
		int new_null_region)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	LONG null_long;

	DbgOut(ORB_DBG_SET, ("OrbSetNullRegion(): enter\n"));
	// Sanity check
	if (!OrbIsValidNullRegion(new_null_region)) {
		goto out;
	}
	// Set a pending null region...
	devExt->orbData.null_region = new_null_region;
	devExt->orbData.new_null_region_pending = TRUE;
	// Then record the data
	null_long = new_null_region;
	status = OrbWriteRegistryLong(L"null_region", null_long);
out:

	return status;
}
