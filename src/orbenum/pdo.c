//
// PDO handles device specific (functional device objects)
//

#include "orbenum.h"
#include "stdio.h"

//
// This file contains handlers for PDO objects (devices themself) that
// our BUS creates
//

NTSTATUS
OrbPdoCallComplete(IN PDEVICE_OBJECT fdo, IN PIRP fdoIrp, PVOID nothing)
{
  PIO_STACK_LOCATION irpSp;
  PIRP Irp;

  ObDereferenceObject(fdo);
  irpSp = IoGetCurrentIrpStackLocation(fdoIrp);
  Irp = (PIRP) irpSp->Parameters.Others.Argument1;
  DbgOut( ORB_DBG_PDO, ("OrbPdoCallComplete(): status %x\n", fdoIrp->IoStatus.Status));
  Irp->IoStatus.Status = fdoIrp->IoStatus.Status;
  Irp->IoStatus.Information = fdoIrp->IoStatus.Information;
  IoFreeIrp(fdoIrp);
  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
OrbPdoCallFdo(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PDEVICE_OBJECT fdo;
  PIO_STACK_LOCATION irpSp, fdoSp;
  PIRP fdoIrp;

  DbgOut( ORB_DBG_PDO, ("OrbPdoCallFdo(): enter\n"));
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  fdo = pdevExt->fdo;
  fdo = IoGetAttachedDeviceReference(fdo);
  fdoIrp = IoAllocateIrp(fdo->StackSize + 1, FALSE);
  if (fdoIrp == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoCallFdo(): no IRP\n"));

      ObDereferenceObject(fdo);
      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, Irp->IoStatus.Information);
    }
  fdoSp = IoGetNextIrpStackLocation(fdoIrp);
  fdoSp->DeviceObject = fdo;
  fdoSp->Parameters.Others.Argument1 = (PVOID) Irp;
  // Set next IRP stack location
  IoSetNextIrpStackLocation(fdoIrp);
  fdoSp = IoGetNextIrpStackLocation(fdoIrp);
  RtlCopyMemory(fdoSp, irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine));
  fdoSp->Control = 0;
  IoSetCompletionRoutine(fdoIrp, (PIO_COMPLETION_ROUTINE) OrbPdoCallComplete, NULL, TRUE, TRUE, TRUE);
  fdoIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
  IoMarkIrpPending(Irp);
  IoCallDriver(fdo, fdoIrp);
  DbgOut( ORB_DBG_PDO, ("OrbPdoCallFdo(): exit"));

  return STATUS_PENDING;
}

// Handle start/stop device like functional driver does
NTSTATUS
OrbPdoPnp(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  UCHAR func;
  PAGED_CODE();

  // Get current I/O stack location and stuff 
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  func = irpSp->MinorFunction;
  DbgOut( ORB_DBG_PDO, ("OrbPdoPnp(): enter %s\n", PnpToString(func)));
  status = IoAcquireRemoveLock(&pdevExt->RemoveLock, Irp);
  // Fail if we don't get remove lock
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoPnp(): remove lock failed %x\n", status));
      return CompleteIrp(Irp, status, 0);
    }
  switch (func) 
    {
      // These requests always succeed
    case IRP_MN_REMOVE_DEVICE:
      // dbg dbg special handling
      //status = OrbPdoRemove(pdo, Irp);
      //break;
      // Note, maybe we should pass these
      // Requests to lower driver?
    case IRP_MN_START_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_SURPRISE_REMOVAL:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
      status = CompleteIrp(Irp, STATUS_SUCCESS, Irp->IoStatus.Information);
      break;
      //
    case IRP_MN_QUERY_DEVICE_RELATIONS:
      status = OrbPdoQueryRelations(pdevExt, Irp);
      break;
      // These always go to orbenum bus FDO
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    case IRP_MN_QUERY_BUS_INFORMATION:
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
    case IRP_MN_SURPRISE_REMOVAL+1:
      status = OrbPdoCallFdo(pdevExt, Irp);
      break;
      // PDOs handle QueryId calls
    case IRP_MN_QUERY_ID:
      status = OrbPdoQueryId(pdevExt, Irp);
      break;
      // This is used to get device description text
    case IRP_MN_QUERY_DEVICE_TEXT:
      status = OrbPdoQueryDeviceText(pdevExt, Irp);
      break;
    default:
      status = CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
      //status = CallNextDriver(pdevExt->nextDevObj, Irp);
    }
  // In case of IRP_MN_REMOVE_DEVICE, device is already gone,
  // so no device extension!
  if (func != IRP_MN_REMOVE_DEVICE) 
    {
      IoReleaseRemoveLock(&pdevExt->RemoveLock, Irp);
    }
  DbgOut( ORB_DBG_PDO, ("OrbPdoPnp(): exit %s, status %x\n", PnpToString(func), status));

  return status;
}

// Note, we can just simply complete this call
// w/o calling serial
NTSTATUS
OrbPdoStart(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  NTSTATUS status;

  DbgOut( ORB_DBG_PDO, ("OrbPdoStart(): enter\n"));
  // status = CallNextDriverWait(pdo, Irp);
  status = STATUS_SUCCESS;
  if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) 
    {
      // Start and do whatever we want
      if (NT_SUCCESS(status)) 
	{
	  //
	  // As we are successfully now back from our start device
	  // we can do work.
	  //
	  pdevExt->Started = TRUE;
	  pdevExt->Removed = FALSE;
	}
    }
  //
  // We must now complete the IRP, since we stopped it in the
  // completion routine with STATUS_MORE_PROCESSING_REQUIRED.
  //
  DbgOut( ORB_DBG_PDO, ("OrbPdoStart(): exit %x\n", status));

  return CompleteIrp(Irp, status, 0);
}

NTSTATUS
OrbPdoRemove(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PDEVICE_EXTENSION devExt;
  NTSTATUS status;
  PIRP Irp1;

  DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): enter\n"));
  // Get root FDO dev ext
  devExt = (PDEVICE_EXTENSION) pdevExt->fdo->DeviceExtension;
  // Lock PDO array
  OrbLockPdos(devExt);
  // Remove PDO from array
  DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): numdevice %d\n", pdevExt->numDevice));
  devExt->devArray[pdevExt->numDevice] = NULL;
  devExt->numDevices--;
  OrbUnlockPdos(devExt);
  // Wait for pending I/O to complete
  IoReleaseRemoveLockAndWait(&pdevExt->RemoveLock, Irp);
#if 0
  Irp->IoStatus.Status = STATUS_SUCCESS;
  IoSkipCurrentIrpStackLocation(Irp);
  // Call serial driver
  status = IoCallDriver(pdevExt->nextDevObj, Irp);
#endif
  status = CompleteIrp(Irp, STATUS_SUCCESS, 0);
  // Detach and delete device object
#if 0
  Irp1 = IoAllocateIrp(pdevExt->nextDevObj->StackSize + 1, FALSE);
  // Send IRP_MJ_CLOSE to serial driver
  if (Irp1 == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): no IRP??? bad!\n"));
    }
  if (Irp1) 
    {
      IoSetNextIrpStackLocation(Irp1);
      irpSp = IoGetNextIrpStackLocation(Irp1);
      irpSp->MajorFunction = IRP_MJ_CLOSE;
      status = CallNextDriverWait(pdevExt->nextDevObj, Irp1);
      DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): sent CLOSE, status %x\n"));
      // Send IRP_MJ_CLEANUP request
      IoSetNextIrpStackLocation(Irp1);
      irpSp = IoGetNextIrpStackLocation(Irp1);
      irpSp->MajorFunction = IRP_MJ_CLEANUP;
      status = CallNextDriverWait(pdevExt->nextDevObj, Irp1);
      DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): sent CLEANUP, status %x\n"));
      IoFreeIrp(Irp1);
    }
#endif
  //IoDetachDevice(pdevExt->nextDevObj);
  // Just close COM port
  //OrbSerClosePort(pdevExt->nextDevObj);
  //ObDereferenceObject(pdevExt->fileObj);
  ObDereferenceObject(pdevExt->nextDevObj);
  // Don't forget to free linkName buffer!!!
  ExFreePool(pdevExt->linkName);
  IoDeleteDevice(pdevExt->devObj);
  DbgOut( ORB_DBG_PDO, ("OrbPdoRemove(): exit %x\n", status));

  return status;
}

//
// This is where we tell PNP about all our devices
//
NTSTATUS
OrbPdoQueryRelations(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status;
	PDEVICE_RELATIONS rel, oldrel;
	ULONG count, size, i, relType;

	// Get I/O stack location and stuff
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): enter\n"));
	relType = irpSp->Parameters.QueryDeviceRelations.Type;
	// Determine what to do
	switch (relType) {
	case TargetDeviceRelation:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): target relations\n"));
	break;
	case EjectionRelations:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): ejection relations\n"));
	// Just complete request if device is not removed
	if (!pdevExt->Removed) {
		return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	}
	break;
	case RemovalRelations:
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): removal relations\n"));
	// Simply complete request
	//return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
	break;
	}
	// Return PDO for all relations
	oldrel = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
	// Allocate place for DEVICE_RELATIONS
	rel = ExAllocatePoolWithTag(PagedPool, sizeof(DEVICE_RELATIONS), 'ZbrO');
	status = STATUS_INSUFFICIENT_RESOURCES;
	if (rel) {
		rel->Count = 1;
		rel->Objects[0] = pdevExt->devObj;
		ObReferenceObject(pdevExt->devObj);
		status = STATUS_SUCCESS;
		// TBD: don't free oldrel, add to it?
		if (oldrel)
			ExFreePool(oldrel);
	}
	DbgOut(ORB_DBG_PDO, ("OrbPdoQueryRelations(): exit\n"));

	return CompleteIrp(Irp, status, (ULONG_PTR) rel);
}

// Handle query ID for device
NTSTATUS
OrbPdoQueryId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  NTSTATUS status;
  PIO_STACK_LOCATION irpSp;
  ULONG idType;
  PWCHAR idstring;
  PWCHAR id;
  ULONG len;
  ULONG size;

  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryId(): enter\n"));
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  idType = irpSp->Parameters.QueryId.IdType;
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryId(): query %d\n", idType));

  switch (idType) 
    {
    case BusQueryInstanceID:
      status = OrbPdoQueryInstanceId(pdevExt, Irp);
      break;
    case BusQueryDeviceID:
      status = OrbPdoQueryDeviceId(pdevExt, Irp);
      break;
    case BusQueryHardwareIDs:
      status = OrbPdoQueryHardwareId(pdevExt, Irp);
      break;
    case BusQueryCompatibleIDs:
    default:
      status = CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
      goto ex;
    }
 ex:
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryId(): exit %x\n", status));

  return status;
}

NTSTATUS
OrbPdoQueryInstanceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  PWCHAR id;
  ULONG len, size;

  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryId(): enter\n"));
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  len = 4;
  size = (len + 2) * sizeof(WCHAR);
  id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
  if (id == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoQueInstId(): no memory\n"));

      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
  swprintf(id, L"%02d", pdevExt->instanceId);
  id[len+1] = 0;
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryId(): exit\n"));

  return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  PWCHAR id;
  ULONG len, size;

  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryDevId(): enter\n"));
  // Get current IRP stack location
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  //len = wcslen(L"ORBENUM\\*FOOBAR");
  len = wcslen(pdevExt->deviceId);
  size = (len + 2) * sizeof(WCHAR);
  id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
  if (id == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoQueDevId(): no memory\n"));

      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
  //wcscpy(id, L"ORBENUM\\*FOOBAR");
  wcscpy(id, pdevExt->deviceId);
  id[len+1] = 0;
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryDevId(): exit\n"));

  return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryHardwareId(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  PWCHAR id;
  ULONG len, size;

  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryHardId(): enter\n"));
  // Get current IRP stack location
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  //len = wcslen(L"*FOOBAR");
  len = wcslen(pdevExt->hardwareId);
  size = (len + 2) * sizeof(WCHAR);
  id = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
  if (id == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoQueHardId(): no memory\n"));

      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
  //wcscpy(id, L"*FOOBAR");
  wcscpy(id, pdevExt->hardwareId);
  // Add null terminator
  id[len+1] = 0;
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryHardId(): exit\n"));

  return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) id);
}

NTSTATUS
OrbPdoQueryDeviceText(IN PPDO_EXTENSION pdevExt, IN PIRP Irp)
{
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status;
  PWCHAR text;
  ULONG len, size;

  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): enter\n"));
  // Get current IRP stack location
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  if (irpSp->Parameters.QueryDeviceText.DeviceTextType != DeviceTextDescription) 
    {
      return CompleteIrp(Irp, Irp->IoStatus.Status, Irp->IoStatus.Information);
    }
  if ((text = (PWCHAR) Irp->IoStatus.Information) != NULL) 
    {
      // Somebody provided this information? strange
      status = STATUS_SUCCESS;
      goto complete;
    }
  len = wcslen(pdevExt->model);
  size = (len + 2) * sizeof(WCHAR);
  text = ExAllocatePoolWithTag(PagedPool, size, 'ZbrO');
  if (text == NULL) 
    {
      DbgOut( ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): no memory\n"));

      return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
  swprintf(text, L"%ws", pdevExt->model);
  DbgOut( ORB_DBG_PDO, ("OrbPdoQueryDeviceText(): exit, text %ws\n", text));

 complete:
  return CompleteIrp(Irp, STATUS_SUCCESS, (ULONG_PTR) text);
}
