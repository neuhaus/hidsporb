#include <stddef.h>
#include <memory.h>
#include <wdm.h>
#define	DEFINE_GUID
#include <initguid.h>
#include <wdmguid.h>
#include <ntddser.h>

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
	FAST_MUTEX	devArrayMutex;	// Mutex for protecting array
	ULONG		numDevices;	// Number of PDOs
	PDEVICE_OBJECT	devArray[ORB_MAX_DEVICES];	// Orbs array
	PVOID		notifyEntry;	// IoRegPnpNfy() stuff
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
	PFILE_OBJECT	file;		// Pointer to file object from IoGetDevObP
	ULONG		numDevice;	// Device number for PNP (instance Id)
	PWCHAR		hardwareId;	// Hardware ID (e.g. *SPC0360)
	PWCHAR		deviceId;	// Device ID (e.g. ORBENUM\*FOOBAR)
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
OrbFdoPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS
OrbFdoStart(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS
OrbFdoRemove(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS
OrbFdoQueryRelations(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

// PDO functions
NTSTATUS
OrbPdoPnp(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoStart(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoRemove(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryRelations(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryId(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryInstanceId(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryDeviceId(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryHardwareId(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoQueryDeviceText(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

NTSTATUS
OrbPdoCallComplete(IN PDEVICE_OBJECT fdo, IN PIRP fdoIrp, PVOID nothing);

NTSTATUS
OrbPdoCallFdo(IN PDEVICE_OBJECT pdo, IN PIRP Irp);

#include "detect.h"
#include "notify.h"
#include "serial.h"
#include "orbio.h"
#include "misc.h"
#include "lock.h"
#include "debug.h"
#include "power.h"
#include "dispatch.h"

#endif
