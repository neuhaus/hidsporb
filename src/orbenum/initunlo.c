//
// initunlo.c
//
// Initialization & unload functions
//
#define	DEFINE_GUID
#include <initguid.h>
#include "orbenum.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, OrbEnumUnload)
#endif

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
  ULONG i;

  UNREFERENCED_PARAMETER (RegistryPath);

  DbgOut( ORB_DBG_INITUNLO, ("OrbEnumDriverEntry()\n"));

  //
  // Create dispatch points for the IRPs.
  //

  for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) 
    {
      DriverObject->MajorFunction[i] = OrbEnumDispatch;
    }
  DriverObject->DriverUnload			= OrbEnumUnload;
  DriverObject->MajorFunction[IRP_MJ_PNP]		= OrbEnumDispatchPnp;
  DriverObject->MajorFunction[IRP_MJ_POWER]	= OrbEnumDispatchPower;
  DriverObject->DriverExtension->AddDevice	= OrbEnumAddDevice;

  return STATUS_SUCCESS;
}

VOID
OrbEnumUnload(IN PDRIVER_OBJECT DriverObject)
{
  PAGED_CODE ();

  //
  // The device object(s) should be NULL now
  // (since we unload, all the devices objects associated with this
  // driver must have been deleted.
  //
  ASSERT(DriverObject->DeviceObject == NULL);

  DbgOut( ORB_DBG_INITUNLO, ("OrbEnumUnload()\n"));

  return;
}
