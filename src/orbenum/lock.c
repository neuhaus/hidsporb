//
// lock.c
//
// All locking code belongs to here
//

#include "orbenum.h"

VOID
OrbLockPdos(IN PDEVICE_EXTENSION devExt)
{
  DbgOut( ORB_DBG_LOCK, ("OrbLockPdos(): getting mutex\n"));
  ExAcquireFastMutex(&devExt->devArrayMutex);
}

VOID
OrbUnlockPdos(IN PDEVICE_EXTENSION devExt)
{
  DbgOut( ORB_DBG_LOCK, ("OrbUnlockPdos(): releasing mutex\n"));
  ExReleaseFastMutex(&devExt->devArrayMutex);
}
