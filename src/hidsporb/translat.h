//
// translat.h
//
// Phys/log axis, buttons translation functions
//
//

ULONG
OrbLogicalAxisValue(IN PDEVICE_EXTENSION devExt,
		IN ULONG index,
		IN ULONG use_precision);

USHORT
OrbMapButtons(IN PDEVICE_EXTENSION devExt);
