#include <stddef.h>
#include <memory.h>
#include <wdm.h>
#include <hidtoken.h>
#include <hidusage.h>
#include <hidport.h>

#define	DEFINE_GUID
#include <initguid.h>
#include <wdmguid.h>
#include <ntddser.h>

#ifndef	_HIDSPORB_H_
#define _HIDSPORB_H_

#define HIDSPORB_TAG	'brOH'

#include "orb.h"
#include "packet.h"

// Our device extension
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT	devObj; 	// Our device object
	PDRIVER_OBJECT	DriverObject;	// Driver object
	BOOLEAN		Started;	// Is device started?
	BOOLEAN		Removed;	// Is device being removed?
	BOOLEAN		Filler[2];	// bug fix
	PDEVICE_OBJECT	nextDevObj;	// PDO that BUS gave to us
	IO_REMOVE_LOCK	RemoveLock;	// Remove lock
	// Irp used for I/O
	PIRP		readIrp;	// Irp used for I/O
	ORB_DATA	orbData;	// packet parser engine stuff
	// Thread stuff
	BOOLEAN		threadStarted;	//
	KEVENT		threadTermEvent; //
	KEVENT		threadTerminated; //
	PVOID		threadObj;	// Thread object
	// queue stuff
	KSPIN_LOCK	readQueueLock;	// Spin lock
	LIST_ENTRY	readQueueList;	// List
	ULONG		readsPending;	// Reads pending
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define	GET_ORB_DATA(devExt)	(&(((PDEVICE_EXTENSION) (devExt))->orbData))

#define HIDSPORB_POLARITY_NEGATIVE	0
#define HIDSPORB_POLARITY_ZERO		1
#define HIDSPORB_POLARITY_POSITIVE	2

#define HIDSPORB_BUTTON_TYPE_NONE	0
#define HIDSPORB_BUTTON_TYPE_PHYSICAL	1
#define HIDSPORB_BUTTON_TYPE_LOGICAL	2

// initunlo.c
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

VOID
OrbUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS
OrbAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT fdo);

NTSTATUS
OrbDispatch(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbPnp(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS 
OrbSystemControl(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbPnpComplete(IN PDEVICE_OBJECT, IN PIRP, IN PKEVENT);

NTSTATUS
OrbPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS
OrbStartDevice(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS
OrbRemoveDevice(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

#include "serial.h"
#include "orbio.h"
#include "misc.h"
#include "debug.h"
#include "power.h"
#include "dispatch.h"
#include "report.h"
#include "ioctl.h"
#include "io.h"
#include "irpq.h"
#include "sorb.h"
#include "parse.h"
#include "translat.h"
#include "charts.h"
#include "feature.h"
#include "validity.h"
#include "settings.h"
#include "detect.h"

#define HIDSPORB_VENDOR_ID 0x5a8
#define HIDSPORB_PRODUCT_ID 0x360

//#define	GET_DEV_EXT(fdo)	(((PHID_DEVICE_EXTENSION) (fdo)->DeviceExtension)->MiniDeviceExtension)
#define GET_DEV_EXT(DO) ((PDEVICE_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))
#define GET_PDO_FROM_EXT(DO) \
(((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->PhysicalDeviceObject)

#endif
