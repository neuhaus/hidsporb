//
// FDO functions
//

#include "orbenum.h"

//
// This file contains handlers for our bus FDO
//

// Handle start/stop device like functional driver does
NTSTATUS
OrbFdoPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  UCHAR func;

  PAGED_CODE();
  devExt = (PDEVICE_EXTENSION) fdo->DeviceExtension;
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  func = irpSp->MinorFunction;
  DbgOut(("OrbFdoPnp(): enter %s\n", PnpToString(func)));
  // Try to acquire remove lock
  // it will fail if device is being removed
  status = IoAcquireRemoveLock(&devExt->RemoveLock, Irp);
  // Fail if we don't get remove lock
  if (!NT_SUCCESS(status)) 
    {
      DbgOut(("OrbFdoPnp(): remove lock failed %x\n", status));

      return CompleteIrp(Irp, status, 0);
    }
  switch (func) 
    {
    case IRP_MN_QUERY_DEVICE_RELATIONS:
      // Device relations
      // PNP manager sends this IRP to enumerate our bus
      // We return list of PDOs
      status = OrbFdoQueryRelations(fdo, Irp);
      break;
    case IRP_MN_START_DEVICE:
      // Start ORB minibus FDO
      status = OrbFdoStart(fdo, Irp);
      break;
    case IRP_MN_REMOVE_DEVICE:
      // Remove ORB minibus FDO
      status = OrbFdoRemove(fdo, Irp);
      break;
    case IRP_MN_SURPRISE_REMOVAL:
      // I doubt that will ever happen, but we must handle it anyway
      Irp->IoStatus.Status = STATUS_SUCCESS;
      status = CallNextDriver(devExt->nextDevObj, Irp);
      break;
    case IRP_MN_QUERY_REMOVE_DEVICE:
      // Query remove is sent when PNP is about to remove our device
      // We simply send it to lower bus driver
      Irp->IoStatus.Status = STATUS_SUCCESS;
      status = CallNextDriver(devExt->nextDevObj, Irp);
      break;
    case IRP_MN_QUERY_CAPABILITIES:
      // We don't handle it
      status = CallNextDriverWait(devExt->nextDevObj, Irp);
      CompleteIrp(Irp, status, Irp->IoStatus.Information);
      break;
    default:
      // Call lower bus driver for all other cases
      status = CallNextDriver(devExt->nextDevObj, Irp);
    }
  // In case of IRP_MN_REMOVE_DEVICE, device is already gone,
  // so there is no device extension!!!
  if (func != IRP_MN_REMOVE_DEVICE) 
    {
      IoReleaseRemoveLock(&devExt->RemoveLock, Irp);
    }
  DbgOut(("OrbFdoPnp(): exit %s, status %x\n", PnpToString(func), status));

  return status;
}

// Start FDO device
// Nothing special happens here
NTSTATUS
OrbFdoStart(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
  PDEVICE_OBJECT pdo;
  KEVENT event;
  NTSTATUS status;

  DbgOut(("OrbFdoStart(): enter\n"));
  devExt = (PDEVICE_EXTENSION) fdo->DeviceExtension;
  // Call root bus driver
  Irp->IoStatus.Status = STATUS_SUCCESS;
  status = CallNextDriverWait(devExt->nextDevObj, Irp);
  if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) 
    {
      if (NT_SUCCESS(status)) 
	{
	  //
	  // As we are successfully now back from our start device
	  // we can do work.
	  //
	  devExt->Started = TRUE;
	  devExt->Removed = FALSE;
	  status = STATUS_SUCCESS;
	}
    }
  //
  // We must now complete the IRP, since we stopped it in the
  // completion routine with STATUS_MORE_PROCESSING_REQUIRED.
  //
  DbgOut(("OrbFdoStart(): exit %x\n", status));

  return CompleteIrp(Irp, status, 0);
}

// Remove ORB minibus FDO device
NTSTATUS
OrbFdoRemove(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
  UNICODE_STRING symName;
  ULONG i;
  PDEVICE_EXTENSION devExt;
  NTSTATUS status;

  DbgOut(("OrbFdoRemove(): enter\n"));
  devExt = (PDEVICE_EXTENSION) fdo->DeviceExtension;
  // Wait for pending I/O to complete
  IoReleaseRemoveLockAndWait(&devExt->RemoveLock, Irp);
  Irp->IoStatus.Status = STATUS_SUCCESS;
  IoSkipCurrentIrpStackLocation(Irp);
  // Call root bus
  status = IoCallDriver(devExt->nextDevObj, Irp);
  // Delete all child devices
  // Note, I believe we get here after all our PDOs are removed
  // PNP deletes PDOs first
  // Lock
  OrbLockPdos(devExt);
  DbgOut(("OrbFdoRemove(): deleting %d PDOs\n", devExt->numDevices));
  for (i = 0; i < devExt->numDevices; i++) 
    {
      DbgOut(("OrbFdoRemove(): deleting %d PDO %p\n", i, devExt->devArray[0]));
      IoDeleteDevice(devExt->devArray[i]);
    }
  // Unregister notification callback
  IoUnregisterPlugPlayNotification(devExt->notifyEntry);
  devExt->notifyEntry = NULL;
  OrbUnlockPdos(devExt);
  // Delete symbolic link
  RtlInitUnicodeString(&symName, DOS_DEVICE_NAME);
  IoDeleteSymbolicLink(&symName);
  // Detach and delete device object
  IoDetachDevice(devExt->nextDevObj);
  IoDeleteDevice(fdo);
  DbgOut(("OrbFdoRemove(): exit %x\n", status));

  return status;
}

//
// This is where we tell PNP about all our devices
//
NTSTATUS
OrbFdoQueryRelations(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  PDEVICE_RELATIONS rel, oldrel;
  ULONG count, size, nCopy, i;

  irpSp = IoGetCurrentIrpStackLocation(Irp);
  devExt = (PDEVICE_EXTENSION) fdo->DeviceExtension;
  DbgOut(("OrbFdoQueryRelations(): enter, old rel %p\n", Irp->IoStatus.Information));
  if (irpSp->Parameters.QueryDeviceRelations.Type != BusRelations) 
    {
      // Simply pass request to root bus device
      DbgOut(("OrbFdoQueryRelations(): not BusRelations\n"));
      return CallNextDriver(devExt->nextDevObj, Irp);
      //status = STATUS_SUCCESS;
      goto failed;
    }
  // Lock FDO array so detect code won't do bad things behind our back
  OrbLockPdos(devExt);
  oldrel = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
  nCopy = 0;
  size = sizeof(DEVICE_RELATIONS);
  // Expand array
  if (oldrel != NULL) 
    {
      //CompleteIrp(Irp, STATUS_SUCCESS, rel);
      OrbUnlockPdos(devExt);
      status = STATUS_SUCCESS;
      goto failed;
    }
#if 0
  if (oldrel) 
    {
      size += (oldrel->Count + 1) * sizeof(PDEVICE_OBJECT);
      nCopy = size;
    } else {
      count = devExt->numDevices;
      if (count > 1)
	size += ((count - 1) * sizeof(PDEVICE_OBJECT));
    }
  // Calculate size
  rel = ExAllocatePoolWithTag(PagedPool, size, 'rBRO');
  if (rel == NULL) 
    {
      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
  if (oldrel == 0) 
    {
      rel->Count = 0;
    } else {
      // Copy oldrel
      RtlCopyMemory(newrel, oldrel, nCopy);
    }
#endif
  count = devExt->numDevices;
  DbgOut(("OrbFdoQueryRelations(): count %d\n", count));
  if (count > 1)
    size += ((count - 1) * sizeof(PDEVICE_OBJECT));
  rel = ExAllocatePoolWithTag(PagedPool, size, 'rBRO');
  if (rel == NULL) 
    {
      OrbUnlockPdos(devExt);
      //return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto failed;
    }
  rel->Count = count;
  // Copy & reference devices only if there are any
  if (count > 0) 
    {
      RtlCopyMemory(&rel->Objects[0], &devExt->devArray[0], count * sizeof(PDEVICE_OBJECT));
      // Reference all device objects
      for (i = 0; i < count; i++) 
	{
	  ObReferenceObject(rel->Objects[i]);
	}
    }
  DbgOut(("OrbFdoQueryRelations(): new rel %p\n", rel));
  // Unlock FDO
  OrbUnlockPdos(devExt);
  // I'm not sure if we should call lower PDO (provided by root enum???)
  // Complete query request
  Irp->IoStatus.Information = (ULONG_PTR) rel;
  status = STATUS_SUCCESS;
 failed:
  Irp->IoStatus.Status = status;

  DbgOut(("OrbFdoQueryRelations(): status %x\n", status));
  //return CompleteIrp(Irp, status, Irp->IoStatus.Information);
  return CallNextDriver(devExt->nextDevObj, Irp);
}
