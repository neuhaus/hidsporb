#include <stddef.h>
#include <memory.h>
#include <wdm.h>

#ifndef	_ORBENUM_H_
#define _ORBENUM_H_

// NT device name
#define ORBENUM_DEVICE_NAME	L"\\Device\\Orbenum"
#define DOS_DEVICE_NAME		L"\\DosDevices\\OrbenumDev"

#define ORBENUM_TAG		'erbO'

// There can be 16 Orbs/Spaceballs attached to system
// maybe too many? ;-)))
#define	ORB_MAX_DEVICES		16

typedef struct _COMMON_EXTENSION {
	ULONG		Flags;		// PDO/FDO
} COMMON_EXTENSION, *PCOMMON_EXTENSION;

// Our bus FDO device extension
typedef struct _DEVICE_EXTENSION {
	ULONG		Flags;		// FDO/PDO
	PDEVICE_OBJECT	devObj; 	// Our device object
	PDRIVER_OBJECT	DriverObject;	// Driver object
	BOOLEAN		Started;	// Is device started?
	BOOLEAN		Removed;	// Is device being removed?
	BOOLEAN		Filler[2];	// bug fix
	PDEVICE_OBJECT	nextDevObj;	// PDO that BUS gave to us
	PDEVICE_OBJECT	fdo;		// our bus FDO
	IO_REMOVE_LOCK	RemoveLock;	// Remove lock
	// END OF common extension block
	PDEVICE_OBJECT	busPdo;		// Bus pdo, not attached dev
	FAST_MUTEX	devArrayMutex;	// Mutex for protecting array
	ULONG		numDevices;	// Number of PDOs
	PDEVICE_OBJECT	devArray[ORB_MAX_DEVICES];	// Orbs array
	PVOID		notifyEntry;	// IoRegPnpNfy() stuff
	LIST_ENTRY	portList;	// ORB_PORT list
	FAST_MUTEX	portMutex;	// Mutex for protecting list
	ULONG		pdoMask;	// Used instances numbers
	// NOTE: We can't use the same InstanceId's or PNP will bugcheck!
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// Note that DEVICE/PDO_EXTENSION must be in sync!!!
typedef struct _PDO_EXTENSION {
	ULONG		Flags;		// 1 indicates this is pdo
	PDEVICE_OBJECT	devObj;		// Our device object
	PDRIVER_OBJECT	DriverObject;	// Driver object
	BOOLEAN		Started;	// Is device started?
	BOOLEAN		Removed;	// Is device being removed?
	BOOLEAN		Filler[2];	// bug fix
	PDEVICE_OBJECT	nextDevObj;	// PDO that Orbenum bus gave to us
	PDEVICE_OBJECT	fdo;		// our bus FDO
	IO_REMOVE_LOCK	RemoveLock;	// Remove lock
	// END OF common extension block
	PFILE_OBJECT	fileObj;	// Pointer to file object from IoGetDevObP
	PWCHAR		linkName;	// Link name
	ULONG		numDevice;	// devArray index
	ULONG		instanceId;	// Instance ID for PNP
        PWCHAR		model;		// Model (e.g. 'SpaceOrb 360')
	PWCHAR		hardwareId;	// Hardware ID (e.g. *SPC0360)
	PWCHAR		deviceId;	// Device ID (e.g. ORBENUM\*SPC0360)
} PDO_EXTENSION, *PPDO_EXTENSION;

// initunlo.c
NTSTATUS    
DriverEntry(       
    IN  PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath 
    );

VOID        
OrbEnumUnload(         
    IN  PDRIVER_OBJECT DriverObject
    );

// PNP functions
NTSTATUS
OrbEnumAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo);

NTSTATUS
OrbEnumDispatch(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbEnumDispatchPnp(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS 
OrbEnumDispatchSystemControl(IN PDEVICE_OBJECT devObj, IN PIRP Irp);

NTSTATUS
OrbPnpComplete(IN PDEVICE_OBJECT, IN PIRP, IN PKEVENT);

NTSTATUS
OrbFdoPnp(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

NTSTATUS
OrbFdoStart(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

NTSTATUS
OrbFdoRemove(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

NTSTATUS
OrbFdoQueryRelations(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

NTSTATUS
OrbFdoQueryPnpState(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

// PDO functions
NTSTATUS
OrbPdoPnp(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoStart(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoRemove(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryRelations(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryInstanceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryDeviceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryHardwareId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoQueryDeviceText(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

NTSTATUS
OrbPdoCallComplete(IN PDEVICE_OBJECT fdo, IN PIRP fdoIrp, PVOID nothing);

NTSTATUS
OrbPdoCallFdo(IN PPDO_EXTENSION pdevExt, IN PIRP Irp);

#include "notify.h"
#include "detect.h"
#include "serial.h"
#include "orbio.h"
#include "misc.h"
#include "lock.h"
#include "debug.h"
#include "power.h"
#include "dispatch.h"
#include "orbport.h"

#endif
