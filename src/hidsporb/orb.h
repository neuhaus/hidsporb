#ifndef ORB_H
#define ORB_H
//
// orb.h
//

#define	ORB_NUM_AXES		6

//TODO: watch areas using ORB_NUM_PHYS_BUTTONS, as this is 
//the max number of buttons on the SpaceOrb--not the spaceball
//(4k has 12)
#define	ORB_NUM_PHYS_BUTTONS	 7
#define ORB_MAX_PHYS_BUTTONS    12
#define ORB_MAX_LOGICAL_BUTTONS 16

//indices into axis array
#define AXIS_TX 0
#define AXIS_TY 1
#define AXIS_TZ 2
#define AXIS_RX 3
#define AXIS_RY 4
#define AXIS_RZ 5

#endif
