//
// detect.c
// ORB detection routines
//

#include "orbenum.h"

extern ULONG OrbEnumNumDevices;

// Two models yet
ORB_MODEL orbModels[2] = {
{ L"SpaceOrb 360",
  L"*SPC0360",
  L"ORBENUM\\*SPC0360" },
{ L"SpaceBall 3003",
  L"*SPC3003",
  L"ORBENUM\\*SPC3003" }
};

//
// Some notes on attaching to serial driver stack
// after we have PDOs we must set their StackSize to
// be correct (ie we must include serial devobj stack too!!!)
// failure to do this leads to bugs!
// if we attach by IoAttachDeviceToDeviceStack(), we don't need to do this
//

// This PNP callback is called with all COM port FDOs interface names
// Note: When we receive this callback, ORB minibus FDO is not yet started,
// thus we can't send ANY Irps to it!!!
NTSTATUS
OrbPnpNotifyCallback(IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION pnp, IN PDEVICE_EXTENSION devExt)
{
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION dev;
  PIO_WORKITEM work;
  PORB_NOTIFY_CONTEXT ctx;
  USHORT len, arrival;
  PWCHAR linkName;

  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): enter\n"));
  //	if (!IsEqualGUID(&(pnp->InterfaceClassGuid),
  //			(LPGUID) &GUID_CLASS_COMPORT)) 
  {
    //		return status;
    //	}
    DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): symlink %ws\n", pnp->SymbolicLinkName->Buffer));
    arrival = 0xc001;
    len = sizeof(pnp->Event);
    // Find out what kind of event it is
    if (RtlCompareMemory(&pnp->Event, (LPGUID) &GUID_DEVICE_INTERFACE_ARRIVAL, len) == len)
      {
	DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): arrival\n"));
	arrival = 1;
      } else
	if (RtlCompareMemory(&pnp->Event, (LPGUID) &GUID_DEVICE_INTERFACE_REMOVAL, len) == len)
	  {
	    DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): removal\n"));
	    arrival = 0;
	  }
    // Allocate buffer for link name
    len = pnp->SymbolicLinkName->Length;
    linkName = ExAllocatePoolWithTag(PagedPool, (len + 2) * sizeof(WCHAR), 'SbrO');
    if (linkName == NULL) 
      {
	DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): failed to alloc linkName buffer\n"));
	goto failed;
      }
    // Copy name into our buffer
    RtlCopyMemory(linkName, pnp->SymbolicLinkName->Buffer, len * sizeof(WCHAR));
    // Add zero to terminate string
    linkName[len] = 0;
    // Allocate work item
    work = IoAllocateWorkItem(devExt->devObj);
    DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): allocated workitem %p", work));
    if (work == NULL) 
      {
	ExFreePool(linkName);
	goto failed;
      }
    ctx = ExAllocatePoolWithTag(PagedPool, sizeof(*ctx), 'CbrO');
    if (ctx == NULL) 
      {
	DbgOut( ORB_DBG_NOTIFY, ("OrbNotify(): failed to alloc context\n"));
	ExFreePool(linkName);
	IoFreeWorkItem(work);
	goto failed;
      }
    // N.B. IoAllocateWorkItem holds reference to FDO device object
    ctx->fdo = devExt->devObj;
    ctx->item = work;
    ctx->flags = arrival;
    ctx->linkName = linkName;
    // Queue work item
    IoQueueWorkItem(work, OrbWorkRoutine, DelayedWorkQueue, ctx);
  failed:
    DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCallback(): exit\n"));

    return status;
  }
}

VOID
OrbWorkRoutine(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx)
{
  DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): enter, device %ws\n", ctx->linkName));
  // Well, device object isn't going to go away, IoAllocWorkItem
  // makes sure this won't happen
  // NOTE! IoFreeWorkItem dereferences FDO object
  // so it makes sense to reference it here
  // or in PDOs IRP_MN_START_DEVICE (i think not there!)
  if (ctx->flags == 1) 
    {
      DbgOut(ORB_DBG_NOTIFY, ("OrbWorkRoutine(): arrival\n"));
      // Note: notifyarrival frees ctx->linkName if needed
      //OrbNotifyArrival(fdo, ctx);
      OrbPortArrival((PDEVICE_EXTENSION) fdo->DeviceExtension, ctx);
    } else
      if (ctx->flags == 0) 
	{
	  DbgOut(ORB_DBG_NOTIFY, ("OrbWorkRoutine(): removal\n"));
	  //OrbNotifyRemoval(fdo, ctx);
          OrbPortRemoval((PDEVICE_EXTENSION) fdo->DeviceExtension, ctx);
          // Free linkName buffer and context structure
          ExFreePool(ctx->linkName);
	} else {
	  DbgOut(ORB_DBG_NOTIFY, ("OrbWorkRoutine(): not arrival, nor removal?\n"));
          ExFreePool(ctx->linkName);
	}
  IoFreeWorkItem(ctx->item);
  ExFreePool(ctx);
  DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): exit\n"));
}

NTSTATUS
OrbGetDeviceProperty(IN PDEVICE_OBJECT devObj,
		     IN DEVICE_REGISTRY_PROPERTY devProperty, OUT PVOID *pBuffer, OUT PULONG pLength)
{
  NTSTATUS status;
  PVOID buffer;
  ULONG length;

  DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): enter\n"));
  // Check if we have correct parameters
  if ((pBuffer == NULL) || (pLength == NULL)) 
    {
      return STATUS_INVALID_PARAMETER;
    }
  *pBuffer = buffer = NULL;
  *pLength = 0;
  // Make first call to determine needed buffer size
  status = IoGetDeviceProperty(devObj, devProperty, 0, buffer, &length);
  DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): needed buffer %d, status %x\n", length, status));
  // If something is wrong, exit
  if (status != STATUS_BUFFER_TOO_SMALL) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): call failed %x\n", status));
      goto failed;
    }
  // Allocate buffer
  buffer = ExAllocatePoolWithTag(PagedPool, length, 'BbrO');
  if (buffer == NULL) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): cant alloc buffer\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto failed;
    }
  status = IoGetDeviceProperty(devObj, devProperty, length, buffer, &length);
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): call1 failed %x\n", status));
      ExFreePool(buffer);
      goto failed;
    }
  *pBuffer = buffer;
  *pLength = length;
 failed:
  DbgOut( ORB_DBG_NOTIFY, ("OrbGetDeviceProperty(): exit, status %x\n", status));

  return status;
}

NTSTATUS
OrbSimComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PKEVENT event)
{
  // We don't care about Irp->PendingReturned!
  KeSetEvent(event, FALSE, 0);

  return STATUS_MORE_PROCESSING_REQUIRED;
}

// Old cruft, to be removed soon -Yuri
#if OLD_CRUFT
// XXX magic hackery, don't mess with it too much!!!
VOID
OrbSimulatePnp(IN PDEVICE_OBJECT devObj)
{
  PIRP Irp;
  PIO_STACK_LOCATION irpSp;
  NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
  KEVENT event;
  ULONG stackSize;

  DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): enter\n"));
  // Allocate IRP
  stackSize = devObj->StackSize;
  DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): stacksize %d\n", stackSize));
  Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
  if (Irp == NULL) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): no IRP\n"));
      goto failed;
    }
  // Skip our IRP stack location
  IoSetNextIrpStackLocation(Irp);
  // Note, IRP stack location points to next SL
  irpSp = IoGetCurrentIrpStackLocation(Irp);
  // Set up major/minor stuff
  irpSp->MajorFunction = IRP_MJ_PNP;
  irpSp->MinorFunction = IRP_MN_QUERY_ID;
  // Set up parameters
  irpSp->Parameters.QueryId.IdType = BusQueryHardwareIDs;
  // Don't forget to set this!!!
  IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) OrbSimComplete, &event, TRUE, TRUE, TRUE);
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  status = IoCallDriver(devObj, Irp);
  // Isn't supposed to happen, but who knows???
  if (status == STATUS_PENDING) 
    {
      KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): call not successful, status %x\n", status));
      goto freeirp;
    }
  //DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): returned ID: %ws\n", (PVOID) Irp->IoStatus.Information));
  DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): returned ID: %p\n", (PVOID) Irp->IoStatus.Information));
  ExFreePool((PVOID) Irp->IoStatus.Information);
 freeirp:
  IoFreeIrp(Irp);
 failed:
  DbgOut( ORB_DBG_NOTIFY, ("OrbSimulatePnp(): exit, status %x\n", status));
}

// Just debugging for now
ULONG
OrbNotifyCheck(IN PDEVICE_OBJECT devObj)
{
  NTSTATUS status;
  ULONG len;
  ULONG model;
  CHAR buffer[256];

  model = 0xffff;
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCheck(): enter\n"));
#if 0
  status = OrbGetDeviceProperty(devObj, DevicePropertyManufacturer, &buffer, &len);
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCheck(): failed, %x\n", status));
      goto failed;
    }
  ExFreePool(buffer);
#endif
  //OrbSimulatePnp(devObj);
  // Initialize com port and power up ORB
  status = OrbInitComPort(devObj);
  // Power down & restore settings
  if (!NT_SUCCESS(status)) 
    {
      OrbPowerDown(devObj);
      goto failed;
    }
  //OrbReadSomething(devObj, buffer);
  //now look in "buffer" and see if we find our strings
  if ( OrbBufferContainsOrbStartupString( buffer , 256 ) )
    {
      DbgOut( ORB_DBG_NOTIFY, ( "OrbNotifyCheck(): ****DETECTED SPACEORB****" ));
      model = 0;
    }
  if ( OrbBufferContainsBallStartupString( buffer, 256 ) )
    {
      DbgOut( ORB_DBG_NOTIFY, ( "OrbNotifyCheck(): ****DETECTED SPACEBALL****" ));
      model = 1; // disabled to further test hidsporb driver
    }
  OrbPowerDown(devObj);
failed:
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyCheck(): exit\n"));
  return model;
}

//VERY simplistic "find a string in a string" function; if the 
//string of bytes p2 exists in p1 returns TRUE, otherwise
//returns FALSE
BOOLEAN
OrbBlockContains( IN CONST PVOID p1, 
		  IN SIZE_T p1_size,
		  IN CONST PVOID p2,
		  IN SIZE_T p2_size )
{
  int search_counter = 0;
  SIZE_T search_length = p1_size - p2_size;
  SIZE_T compare_result;
  BOOLEAN Result = FALSE;

  for ( search_counter = 0; 
	( search_counter < search_length )
	  && ( Result == FALSE );
	++search_counter )
    {
      compare_result = RtlCompareMemory( (PVOID)( (PCHAR)p1 + search_counter ),
					 p2,
					 p2_size );
      if ( compare_result == p2_size )
	{
	  Result = TRUE;
	}
    }
  return Result;
}

//simple search to see if the given buffer contains the SpaceOrb startup string
BOOLEAN
OrbBufferContainsOrbStartupString( IN PCHAR buffer,
				   IN SIZE_T buffer_size )
{
  CHAR search_string[] = "R Spaceball (R)";
  SIZE_T search_string_length = 15;
  return OrbBlockContains( buffer, buffer_size, 
			   (PVOID)(search_string), search_string_length );
}

//simple search to see if the given buffer contains the SpaceBall 
//startup string
BOOLEAN 
OrbBufferContainsBallStartupString( IN PCHAR buffer,
				   IN SIZE_T buffer_size )
{
  CHAR search_string[] = "@1 Spaceball alive and well";
  SIZE_T search_string_length = 27;
  return OrbBlockContains( buffer, buffer_size,
			   (PVOID)(search_string), search_string_length );
}

// This function is called from a worker thread context
VOID
OrbNotifyArrival(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx)
{
  PDEVICE_OBJECT devObj;
  PFILE_OBJECT fileObj;
  PDEVICE_EXTENSION devExt;
  NTSTATUS status;
  UNICODE_STRING linkName;
  ULONG model, numDevice, free;
  PDEVICE_OBJECT pdo;
  PPDO_EXTENSION pdevExt;

  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): enter\n"));
  free = 1;
  devExt = (PDEVICE_EXTENSION) ctx->fdo->DeviceExtension;
  // Acquire FDO removelock
  status = IoAcquireRemoveLock(&devExt->RemoveLock, ctx);
  // Couldn't get lock, means device is removing
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): cant get removelock, status %x\n", status));
      goto failed;
    }
  // Init string
  RtlInitUnicodeString(&linkName, ctx->linkName);
  // Get target device object pointer
  status = IoGetDeviceObjectPointer(&linkName,
				    STANDARD_RIGHTS_ALL,
				    &fileObj,
				    &devObj);
  // Couldn't open, used by other driver or program
  // we don't care about this failure
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): cant open device %ws, status %x\n", ctx->linkName, status));
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
    }
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): got dev obj %p, file obj %p\n", devObj, fileObj));
  // Get ORB model
  model = OrbNotifyCheck(devObj);
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): found orb, model %d", model ));
  // Deref file object if we didn't find ORB
  if (model == 0xffff) {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): ORB not detected\n"));
      ObDereferenceObject(fileObj);
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
  }
  // Well, we found ORB, create PDO for it
  status = OrbCreatePdo(devExt, &pdo);
  // Fail if no success
  if (!NT_SUCCESS(status)) {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): cant alloc PDO, status %x\n", status));
      ObDereferenceObject(fileObj);
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
  }
  // Lock PDOs array
  OrbLockPdos(devExt);
  // Get device number
  numDevice = devExt->numDevices++;
  // We've got PDO, initialize it and insert into global list
  OrbInitPdo(pdo, devObj, fileObj, ctx->linkName, orbModels[model].model,
	     orbModels[model].hardwareId, orbModels[model].deviceId, numDevice);
  // Insert into PDOs array
  devExt->devArray[numDevice] = pdo;
  // Unlock PDOs array
  OrbUnlockPdos(devExt);
  // Don't free ctx->linkName
  free = 0;
  // TODO: issue IoInvalidateDeviceRelations() in some cases
  // We can't always issue it, because PNP notify callback can be called
  // EVEN before orbenum FDO is started
  // Get devobj reference
  ObReferenceObject(devObj);
  // Loose file reference (close COM port)
  ObDereferenceObject(fileObj);
  // Force PNP to rescan our BUS
  if (devExt->Started) {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): invdevrel sent\n"));
      IoInvalidateDeviceRelations(devExt->busPdo, BusRelations);
  }
  // Release remove lock
  IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
failed:
  if (free) {
      ExFreePool(ctx->linkName);
  }
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyArrival(): exit\n"));
}

// This function is called from a worker thread context
VOID
OrbNotifyRemoval(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx)
{
  PDEVICE_EXTENSION devExt;
  PPDO_EXTENSION pdevExt;
  ULONG i, found = 0, len;

  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyRemoval(): enter\n"));
  // Get FDO device extension
  devExt = (PDEVICE_EXTENSION) ctx->fdo->DeviceExtension;
  // Get symlink len
  len = wcslen(ctx->linkName);
  // Lock PDOs array
  OrbLockPdos(devExt);
  // Find device by linkName and delete it
  for (i = 0; i < devExt->numDevices; i++) {
       pdevExt = (PPDO_EXTENSION) devExt->devArray[i]->DeviceExtension;
       if (RtlCompareMemory(pdevExt->linkName, ctx->linkName, len) == len) {
           DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyRemoval(): found dev %ws\n", ctx->linkName));
           // Found device, remove it from array
           devExt->devArray[i] = NULL;
           found = 1;
           break;
       }
  }
  if (found) {
      // Decrement number of devices
      devExt->numDevices--;
      // Delete device object
      IoDeleteDevice(pdevExt->devObj);
  }
  // Unlock PDOs array
  OrbUnlockPdos(devExt);
  if (found) {
      DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyRemoval(): invdevrel sent\n"));
      IoInvalidateDeviceRelations(devExt->busPdo, BusRelations);
  }
  DbgOut( ORB_DBG_NOTIFY, ("OrbNotifyRemoval(): exit\n"));
}
#endif
// OLD cruft, to be removed soon

// Create PDO device object
NTSTATUS
OrbCreatePdo(IN PDEVICE_EXTENSION devExt, OUT PDEVICE_OBJECT *ppdo)
{
  PDEVICE_OBJECT pdo;
  PPDO_EXTENSION pdevExt;
  NTSTATUS status = STATUS_SUCCESS;

  DbgOut( ORB_DBG_NOTIFY, ("OrbCreatePdo(): enter\n"));
  DbgOut( ORB_DBG_NOTIFY, ("OrbCreatePdo(): devExt %p, drvObj %p\n", devExt, devExt->DriverObject));
  pdo = NULL;
  *ppdo = NULL;
  status = IoCreateDevice(devExt->DriverObject, sizeof(PDO_EXTENSION), NULL,
			  FILE_DEVICE_UNKNOWN, FILE_AUTOGENERATED_DEVICE_NAME, TRUE, &pdo);
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbCreatePdo(): cant create device object, %x\n", status));
      goto failed;
    }
  pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
  // Indicate it's PDO device
  pdevExt->Flags = 1;
  pdevExt->devObj = pdo;
  pdevExt->fdo = devExt->devObj;
  pdevExt->DriverObject = devExt->DriverObject;
  //pdevExt->numDevice = InterlockedIncrement(&OrbEnumNumDevices) - 1;
  pdevExt->hardwareId = NULL;
  pdevExt->deviceId = NULL;
  pdevExt->numDevice = 0;
  pdevExt->instanceId = ORB_MAX_DEVICES;
  // Note, we should copy Flags in OrbPdoStart()
  pdo->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
  pdo->Flags &= ~DO_DEVICE_INITIALIZING;
  *ppdo = pdo;
 failed:
  DbgOut( ORB_DBG_NOTIFY, ("OrbCreatePdo(): exit, PDO %p, %x\n", pdo, status));

  return status;
}

// Note
// we may need this routine if we didn't attach
// to device stack
NTSTATUS
OrbInitPdo(IN PDEVICE_OBJECT pdo, IN PDEVICE_OBJECT targetFdo,
	   IN PFILE_OBJECT fileObj, IN PWCHAR linkName, IN PWCHAR model,
	   IN PWCHAR hardwareId, IN PWCHAR deviceId, IN ULONG instanceId, IN ULONG numDevice)
{
  PPDO_EXTENSION pdevExt;

  DbgOut( ORB_DBG_NOTIFY, ("OrbInitPdo(): enter\n"));
  DbgOut( ORB_DBG_NOTIFY, ("OrbInitPdo(): PDO %p trgFdo %p hardwareId %ws deviceId %ws numDevice %d\n",
	  pdo, targetFdo, hardwareId, deviceId, numDevice));
  pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
  // We cat't attach to stack, don't ever do this!
  // Set next device object
  pdevExt->nextDevObj = targetFdo;
  // Correct stack size
  // Note, might be pdo->StackSize + fdo->StackSize [+ 1]
  pdo->StackSize = targetFdo->StackSize + 1;
  pdevExt->linkName = linkName;
  pdevExt->model = model;
  pdevExt->hardwareId = hardwareId;
  pdevExt->deviceId = deviceId;
  pdevExt->numDevice = numDevice;
  pdevExt->instanceId = instanceId;
  pdevExt->fileObj = fileObj;
  pdevExt->Removed = FALSE;
  IoInitializeRemoveLock(&pdevExt->RemoveLock, ORBENUM_TAG,
			 1,
			 5);
  // Acquire remove lock for deleting
  //IoAcquireRemoveLock(&pdevExt->RemoveLock, 0);
  DbgOut( ORB_DBG_NOTIFY, ("OrbInitPdo(): exit\n"));

  return STATUS_SUCCESS;
}

#if 0
//
XXX
{
  PDEVICE_OBJECT devObj;
  PDEVICE_EXTENSION devExt;
  PFILE_OBJECT file;
  NTSTATUS status;
  PDEVICE_OBJECT pdo, nextDevObj;
  PPDO_EXTENSION pdevExt;

  devExt = (PDEVICE_EXTENSION) fdo->DeviceExtension;
  // Well, device object isn't going to go away, IoAllocWorkItem
  // makes sure this won't happen
  // NOTE! IoFreeWorkItem dereferences FDO object
  // so it makes sense to reference it here
  // or in PDOs IRP_MN_START_DEVICE (i think not here!)
  // Hack hack hack
  // dbgdbg
  // we don't attach for second time
  if ((ctx->flags != 0) || (notagain)) 
    {
      goto failed;
    }
  notagain = 1;
  // Acquire FDO removelock
  status = IoAcquireRemoveLock(&devExt->RemoveLock, ctx);
  // Couldn't get lock, means device is removing
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): cant get removelock, status %x\n", status));
      goto failed;
    }
  // Get target device object pointer
  status = IoGetDeviceObjectPointer(&ctx->linkName,
				    STANDARD_RIGHTS_ALL,
				    &file,
				    &devObj);
  // Couldn't open, used by other driver or program
  // we don't care about this failure
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): cant open device %ws, status %x\n", ctx->linkName.Buffer, status));
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
    }
  DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): got dev obj %p, file obj %p\n", devObj, file));
  // dbg dbg dbg
  status = OrbCreatePdo(devExt->devObj, &pdo);
  if (!NT_SUCCESS(status)) 
    {
      DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): cant alloc PDO\n"));
      ObDereferenceObject(file);
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
    }
  // We have PDO
  DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): allocated PDO %p\n", pdo));
  nextDevObj = IoAttachDeviceToDeviceStack(pdo, devObj);
  if (nextDevObj == NULL) 
    {
      // Coudn't attach
      DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): couldnt attach\n"));
      // Delete PDO
      IoDeleteDevice(pdo);
      // Deref file object
      ObDereferenceObject(file);
      // Release remove lock
      IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
      goto failed;
    }
  DbgOut( ORB_DBG_NOTIFY, ("OrbWorkRoutine(): attached %p to n %p/d %p\n", pdo, nextDevObj, devObj));
  // Save serial's device object pointer
  // in PDO's device extension
  pdevExt = (PPDO_EXTENSION) pdo->DeviceExtension;
  pdevExt->nextDevObj = nextDevObj;
  pdevExt->file = file;
  // We must copy flags in OrbPdoStart()
  pdo->Flags |= DO_BUFFERED_IO;
  OrbLockPdos(devExt);
  // Insert into array
  devExt->devArray[devExt->numDevices++] = pdo;
  OrbUnlockPdos(devExt);
  // For now, just release our reference
  //ObDereferenceObject(file);
  // Release remove lock
  IoReleaseRemoveLock(&devExt->RemoveLock, ctx);
  // Force PNP to rescan our bus
  //IoInvalidateDeviceRelations(devExt->devObj, BusRelations);
}

#endif
