//
// lock.c
//
// All locking code belongs to here
//

#include "orbenum.h"

VOID
OrbLockPdos(IN PDEVICE_EXTENSION devExt)
{
	DbgOut(("OrbLockPdos(): getting mutex\n"));
	ExAcquireFastMutex(&devExt->devArrayMutex);
}

VOID
OrbUnlockPdos(IN PDEVICE_EXTENSION devExt)
{
	DbgOut(("OrbLockPdos(): releasing mutex\n"));
	ExReleaseFastMutex(&devExt->devArrayMutex);
}
