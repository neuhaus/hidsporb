#ifndef	_ORBPORT_H_
#define	_ORBPORT_H_

// ORB_COMPORT state flags
#define	ORB_DEVICE_ARRIVING	1
#define	ORB_DEVICE_REMOVING	2

#define	OrbIsDeviceArriving(orbPort)	((orbPort)->state & ORB_DEVICE_ARRIVING)
#define	OrbIsDeviceRemoving(orbPort)	((orbPort)->state & ORB_DEVICE_REMOVING)

// This structure is created when callback gets COM device name
// it's used to track outstanding plugins/removals
typedef struct _ORB_PORT {
	// COMPORT list
	LIST_ENTRY	portList;
	// This entry state
	ULONG		state;
	// Reference count
	ULONG		refCount;
	// Link name
	PWCHAR		linkName;
	ULONG		linkLen;
	// synchronization event
	// waiting for arrival/removal completion
	KEVENT		event;
} ORB_PORT, *PORB_PORT;

VOID
OrbInitPortList(IN PDEVICE_EXTENSION devExt);

PORB_PORT
OrbGetPort(IN PDEVICE_EXTENSION devExt, IN PWCHAR linkName, ULONG linkLen, BOOLEAN arrival);

VOID
OrbWakeupPort(IN PDEVICE_EXTENSION devExt, IN PORB_PORT port, IN ULONG state);

#endif
