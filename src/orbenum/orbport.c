//
// orbport.c
//
// ORB port tracking code
//

#include "orbenum.h"

#if	DBG
// Dump ORB_PORT structure
VOID
OrbDumpPort(IN PORB_PORT port)
{
	DbgOut(ORB_DBG_DETECT, ("Port %ws, ref %d state %s:%s\n",
				port->linkName,
				port->refCount,
				OrbIsDeviceArriving(port) ? "ARRIVING" : "",
				OrbIsDeviceRemoving(port) ? "REMOVING" : ""));
}
#else
#define	OrbDumpPort(_dummy_)
#endif

VOID
OrbInitPortList(IN PDEVICE_EXTENSION devExt)
{
	InitializeListHead(&devExt->portList);
	ExInitializeFastMutex(&devExt->portMutex);
}

VOID
OrbLockPortList(IN PDEVICE_EXTENSION devExt)
{
	ExAcquireFastMutex(&devExt->portMutex);
}

VOID
OrbUnlockPortList(IN PDEVICE_EXTENSION devExt)
{
	ExReleaseFastMutex(&devExt->portMutex);
}

VOID
OrbInsertPort(IN PDEVICE_EXTENSION devExt, IN PORB_PORT port)
{
	InsertTailList(&devExt->portList, &port->portList);
}

VOID
OrbRemovePort(IN PDEVICE_EXTENSION devExt, IN PORB_PORT port)
{
	RemoveEntryList(&port->portList);
}

// Find ORB_PORT by linkName and increment its refCount
PORB_PORT
OrbFindPort(IN PDEVICE_EXTENSION devExt, IN PWCHAR linkName, IN ULONG linkLen)
{
	PORB_PORT port;

	DbgOut(ORB_DBG_DETECT, ("OrbFindPort(): %ws\n", linkName));
	// Note: this is ARRIVING/REMOVING COM port list
	if (IsListEmpty(&devExt->portList)) {
		return NULL;
	}
	// Get first port structure pointer
	port = CONTAINING_RECORD(devExt->portList.Flink, ORB_PORT, portList);
	// Go thru list
	while (port != (PORB_PORT) &devExt->portList) {
		OrbDumpPort(port);
		// Check if port matches
		if ((port->linkLen == linkLen) && (RtlCompareMemory(port->linkName, linkName, linkLen * 2) == (linkLen * 2))) {
			DbgOut(ORB_DBG_DETECT, ("OrbFindPort(): found %ws\n", linkName));
			// Increment refCount
			InterlockedIncrement(&port->refCount);
			goto out;
		}
		port = CONTAINING_RECORD(port->portList.Flink, ORB_PORT, portList);
	}
	// We didn't find anything
	port = NULL;
out:

	return port;
}

// Allocate new ORB_PORT structure with given state
PORB_PORT
OrbAllocPort(IN PWCHAR linkName, IN ULONG linkLen, ULONG state)
{
	PORB_PORT port;

	DbgOut(ORB_DBG_DETECT, ("OrbAllocPort(): name %ws len %d\n", linkName, linkLen));
	// Allocate port
	port = ExAllocatePoolWithTag(NonPagedPool, sizeof(ORB_PORT), 'PCrO');
	// Fail if we couldn't
	if (port == NULL) {
		return NULL;
	}
	// Allocate name buffer
	port->linkName = ExAllocatePoolWithTag(NonPagedPool, (linkLen * 2) + sizeof(WCHAR), 'nPrO');
	// Fail if we couldn't
	if (port->linkName == NULL) {
		// Free port structure
		ExFreePool(port);

		return NULL;
	}
	// Copy linkName
	RtlCopyMemory(port->linkName, linkName, (linkLen * 2));
	port->linkName[linkLen] = 0;
	//port->linkName[0] = 0;
	// Initialize port
	port->state = state;
	port->refCount = 1;
	port->linkLen = linkLen;
	KeInitializeEvent(&port->event, SynchronizationEvent, FALSE);

	return port;
}

// Free ORB_PORT structure
VOID
OrbFreePort(IN PORB_PORT port)
{
	DbgOut(ORB_DBG_DETECT, ("OrbFreePort(): %ws\n", port->linkName));
	OrbDumpPort(port);
	// Free linkname buffer
	ExFreePool(port->linkName);
	// Free port itself
	ExFreePool(port);
}

// Get or allocate new OBR_COMPORT structure
PORB_PORT
OrbGetPort(IN PDEVICE_EXTENSION devExt, IN PWCHAR linkName, ULONG linkLen, BOOLEAN arrival)
{
	PORB_PORT port;

	DbgOut(ORB_DBG_DETECT, ("OrbGetPort(): port %ws len %d\n", linkName, linkLen));
	// Lock ORB_PORT list
	OrbLockPortList(devExt);
	// Allocate if port not found
	if ((port = OrbFindPort(devExt, linkName, linkLen)) == NULL) {
		// Allocate new port
		port = OrbAllocPort(linkName, linkLen, arrival ? ORB_DEVICE_ARRIVING : ORB_DEVICE_REMOVING);
		// Fail if we couldn't alloc
		if (port == NULL) {
			goto out;
		}
		// Insert into list
		OrbInsertPort(devExt, port);
	} else {
		// OrbFindPort() increments refCount
		// so don't forget to release it in some cases
		port->state |= (arrival ? ORB_DEVICE_ARRIVING : 0);
		// We don't mark it ORB_DEVICE_REMOVING
		DbgOut(ORB_DBG_DETECT, ("OrbGetPort(): found %ws\n", linkName));
	}
out:
#if	DBG
	// Dump port, if any
	if (port) {
		OrbDumpPort(port);
	}
#endif
	// Unlock ORB_PORT list
	OrbUnlockPortList(devExt);

	return port;
}

// Wake up people waiting for some event: ie arrival or removal
VOID
OrbWakeupPort(IN PDEVICE_EXTENSION devExt, IN PORB_PORT port, IN ULONG state)
{
	// Lock port list
	OrbLockPortList(devExt);
	// Clear state
	port->state &= ~state;
	// Now decrement refcount
	if (InterlockedDecrement(&port->refCount) == 0) {
		// Don't care about it anymore
		// Remove it from list and free it
		OrbRemovePort(devExt, port);
		OrbFreePort(port);
		OrbUnlockPortList(devExt);

		return;
	}
	// Set and reset event
	// This will wakeup pending _REMOVING waiter
	KeSetEvent(&port->event, FALSE, 0);
	KeResetEvent(&port->event);
	// Unlock port list
	OrbUnlockPortList(devExt);
}

#if 0
...
possible order of things:
ARRIVAL first, OK we allocate ORB_PORT and set state to _ARRIVING
then REMOVAL comes in, it sees that ARRIVAL is in progress, waits for it
and does things it want
if another ARRIVAL comes in after we set _REMOVING, arrival waits for
pending REMOVAL and does what it what... and so on
...
REMOVAL first, we allocate ORB_PORT and set state to _REMOVING
then ARRIVAL comes in, it sees that REMOVAL is in progress, waits for it
and does things it want
If another REMOVAL comes in later it sees device is _ARRIVING, waits for it
, sets _REMOVING and does what it want... and so on
...

new arrival code
	// Get or alloc port
	port = OrbGetPort(devExt, linkName, linkLen, TRUE);
	// Fail if couldn't
	if (port == NULL) {
		DbgOut(ORB_DBG_PNP, ("OrbArriv(): no port\n"));
		goto failure;
	}
	// OK, port is here
	// Wait for pending removal
	// OrbRemove() doesn't set ORB_DEVICE_REMOVING if it sees it's arriving
	if (OrbIsDeviceRemoving(port)) {
		KeWaitForSingleObject(&port->event, Executive, KernelMode, FALSE, NULL);
	}
	... perform detection
	// OK, detection is finished, PDO has (or hasn't been) allocated
	// Wakeup removal waiters, if any
	OrbWakeupPort(devExt, port, ORB_DEVICE_ARRIVING);
	...

...
new removal code
	// Get or alloc port
	port = OrbGetPort(devExt, linkName, linkLen, FALSE);
	// Fail if couldn't
	if (port == NULL) {
		DbgOut(ORB_DBG_PNP, ("OrbRemove(): no port\n"));
		goto failure;
	}
	// OK, port is here
	// Wait for pending arrival
	if (OrbIsDeviceArriving(port)) {
		port->state |= ORB_DEVICE_IS_REMOVING;
		KeWaitForSingleObject(&port->event, Executive, KernelMode, FALSE, NULL);
	}
	// OK, now do what we need
	...
	// Wakeup pending ARRIVAL
	OrbWakeupPort(devExt, port, ORB_DEVICE_REMOVING);
	...
#endif
