//
// Axis/buttons translation stuff
//

#include "hidsporb.h"

ULONG
OrbLogicalAxisValue(IN PDEVICE_EXTENSION devExt,
		IN ULONG index,
		IN ULONG use_precision)
{
	LONG looked_up;
	LONG base;
	LONG this_axis_gain;
	LONG sensitivity;
	LONG with_gain;
	LONG dividend;
	LONG multiplicand;
	const LONG Scale = 1000;

	// Get Axis value
	base = devExt->Axes[devExt->AxisMap[index]];
	// Get gain value
	this_axis_gain = use_precision ? devExt->precision_gain : devExt->gains[index];
	// Get sensitivity value
	sensitivity = use_precision ? devExt->precision_sensitivity : devExt->sensitivities[index];
	// figure the gain multiplier.  This is an odd beastie.  Basically,
	//at a gain of 50, the scale is even.  At higher levels of gain, the signal
	//is "amplified" so that at gain = 100, values are multiplied by 5.  At
	//gain = 75, values are multiplied by 3.
	if (this_axis_gain < 50) {
		dividend = ((50 - this_axis_gain ) * Scale * 10 / 125) + (1 * Scale);
		with_gain = (base * Scale) / dividend;
	} else
	if (this_axis_gain == 50) {
		with_gain = base;
	} else {
		multiplicand = ((this_axis_gain - 50) * Scale * 10 / 125) + (1 * Scale);
		with_gain = base * multiplicand / Scale;
	}
	// add 512 to get an array index into the sensitivity curves
	with_gain = with_gain + 512;
	// clip to get a valid index
	if (with_gain < 0) {
		with_gain = 0;
	}
	if (with_gain > 1023) {
		with_gain = 1023;
	}
	// flop according to polarity
	switch (devExt->polarities[index]) {
	case HIDSPORB_POLARITY_NEGATIVE:
		with_gain = (1023 - with_gain);
		break;
	case HIDSPORB_POLARITY_ZERO:
		with_gain = 512;
		break;
	case HIDSPORB_POLARITY_POSITIVE:
		break;
	}
	//finally, look up the chart.
	looked_up = charts[ sensitivity ][ with_gain ];

	return looked_up;
}

USHORT
OrbMapButtons(IN PDEVICE_EXTENSION devExt)
{
	int chord_page;
	USHORT result = 0;
	int i;

	if (devExt->use_chording) {
		chord_page = ((devExt->buttons[1] != 0) ? 2 : 0) +
				((devExt->buttons[0] != 0) ? 1 : 0);

	for (i = ORB_NUM_PHYS_BUTTONS - 2; i > 1; --i) {
		result <<= 1;
		result |= (devExt->buttons[i] != 0) ? 1 : 0;
	}

	result <<= (chord_page * 4);
    	} else {
	//loops seemed to work very strangely, here, so we'll go the brute-force
	//way...
#define PHYSICAL_BUTTON( x ) ((devExt->buttons[x] != 0 ) ? 1 : 0)
	result = PHYSICAL_BUTTON( 0 ) |
	( PHYSICAL_BUTTON( 1 ) << 1 ) |
	( PHYSICAL_BUTTON( 2 ) << 2 ) |
	( PHYSICAL_BUTTON( 3 ) << 3 ) |
	( PHYSICAL_BUTTON( 4 ) << 4 ) |
	( PHYSICAL_BUTTON( 5 ) << 5 );
	}

	return result;
}
