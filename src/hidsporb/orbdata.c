#include "orbdata.h"

void
OrbDataSetPhysicalAxis( PORB_DATA orb_data,
			int index,
			LONG value )
{
  orb_data->Axes[ index ] = value;
}

void
OrbDataSetPhysicalAxes( PORB_DATA orb_data,
			LONG tx,
			LONG ty,
			LONG tz,
			LONG rx,
			LONG ry,
			LONG rz )
{
  orb_data->Axes[ AXIS_TX ] = tx;
  orb_data->Axes[ AXIS_TY ] = ty;
  orb_data->Axes[ AXIS_TZ ] = tz;
  orb_data->Axes[ AXIS_RX ] = rx;
  orb_data->Axes[ AXIS_RY ] = ry;
  orb_data->Axes[ AXIS_RZ ] = rz;
}

void
OrbDataSetPhysicalButton( PORB_DATA orb_data,
			  int index,
			  LONG value )
{
  orb_data->buttons[ index ] = value;
}


void
OrbDataSetPhysicalButtons7( PORB_DATA orb_data,
			    int button_0,
			    int button_1,
			    int button_2,
			    int button_3,
			    int button_4,
			    int button_5,
			    int button_6)
{
  orb_data->buttons[ 0 ] = button_0;
  orb_data->buttons[ 1 ] = button_1;
  orb_data->buttons[ 2 ] = button_2;
  orb_data->buttons[ 3 ] = button_3;
  orb_data->buttons[ 4 ] = button_4;
  orb_data->buttons[ 5 ] = button_5;
  orb_data->buttons[ 6 ] = button_6;
}


void
OrbDataSetPhysicalButtons8( PORB_DATA orb_data,
			    int button_0,
			    int button_1,
			    int button_2,
			    int button_3,
			    int button_4,
			    int button_5,
			    int button_6,
			    int button_7 )
{
  OrbDataSetPhysicalButtons7( orb_data, 
			      button_0,
			      button_1,
			      button_2,
			      button_3,
			      button_4,
			      button_5,
			      button_6);
  orb_data->buttons[ 7 ] = button_7;
}



void
OrbDataSetPhysicalButtons12( PORB_DATA orb_data,
			     int button_0,
			     int button_1,
			     int button_2,
			     int button_3,
			     int button_4,
			     int button_5,
			     int button_6,
			     int button_7,
			     int button_8, 
			     int button_9,
			     int button_10,
			     int button_11 )
{
  OrbDataSetPhysicalButtons8( orb_data,
			      button_0,
			      button_1,
			      button_2,
			      button_3,
			      button_4,
			      button_5,
			      button_6,
			      button_7 );
  orb_data->buttons[ 8 ] = button_8;
  orb_data->buttons[ 9 ] = button_8;
  orb_data->buttons[ 10 ] = button_8;
  orb_data->buttons[ 11 ] = button_8;
}

