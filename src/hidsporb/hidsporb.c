/*

Copyright (c) 2001, Victor B. Putz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the project nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "hidsporb.h"
#ifndef IOSYNC_H
#include "iosync.h"
#endif
#ifndef SERIAL_ROUTINES_H
#include "serial_routines.h"
#endif
#ifndef SETTINGS_H
#include "settings.h"
#endif
#ifndef IOCTL_H
#include "ioctl.h"
#endif
#ifndef IRP_QUEUE_H
#include "irp_queue.h"
#endif
#ifndef PNP_H
#include "pnp.h"
#endif

#ifdef ALLOC_PRAGMA

  /*  #pragma alloc_text( INIT, DriverEntry ) */
  /*  #pragma alloc_text( PAGE, HSO_create) */
  /*  #pragma alloc_text( PAGE, HSO_close ) */
  /*  #pragma alloc_text( PAGE, HSO_add_device) */
  /*  #pragma alloc_text( PAGE, HSO_unload) */
  /*  #pragma alloc_text( PAGE, HSO_restore_port ) */
#endif /* ALLOC_PRAGMA */


  NTSTATUS 
    HSO_cleanup
    (
     IN PDEVICE_OBJECT p_device_object,
     IN PIRP p_irp
     )
{
  HSO_LOG( FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	   ("entering cleanup"));
  p_irp->IoStatus.Information = 0;
  p_irp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest( p_irp, IO_NO_INCREMENT );
  return STATUS_SUCCESS;
}


HIDSPORB_GLOBAL Global;
ULONG          debug_log_level;

/* Entry point for the driver */
NTSTATUS 
DriverEntry
(
 IN PDRIVER_OBJECT  DriverObject,
 IN PUNICODE_STRING RegistryPath
 )
{
  NTSTATUS                        nt_status = STATUS_SUCCESS;
  HID_MINIDRIVER_REGISTRATION     hid_minidriver_registration;

  debug_log_level = 
    /*        FILE_HIDSPORB | */
    /*      FILE_PNP | */
    /*FILE_POLL |*/
    /*        FILE_IOCTL | */
    /*      FILE_HID_REPORTS |*/
    /* FILE_SERIAL_ROUTINES | */
    /*      FILE_ORB_COMM | */
    /*        FILE_SETTINGS |  */
    /*        FILE_CONTROL | */
    /*        FILE_IRP_QUEUE | */
    HSO_ERROR |
    HSO_WARNING |
    /*      HSO_MESSAGE1 |*/
    HSO_MESSAGE2 |
    /*            HSO_FUNCTION_ENTRY |*/
    /*            HSO_FUNCTION_EXIT |*/
    /*      HSO_FUNCTION_EXIT_OK |*/
    HSO_DEFAULT_DEBUGLEVEL;

  HSO_LOG(FILE_HIDSPORB| HSO_WARNING, \
	  ("Hidsporb.sys: Built %s at %s\n", __DATE__, __TIME__));

  HSO_LOG( FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	   ("DriverEntry(DriverObject=0x%x,RegistryPath=0x%x (%ws))",
	    DriverObject, RegistryPath, RegistryPath)
	   );

    
  /* store registry path for future reference */
  HSO_store_registry_path( DriverObject, RegistryPath );

  DriverObject->MajorFunction[IRP_MJ_CREATE]= HSO_create;
  DriverObject->MajorFunction[IRP_MJ_CLOSE] = HSO_close; 
  DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = HSO_flush_buffers;
  DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
    HSO_internal_ioctl;
  DriverObject->MajorFunction[IRP_MJ_PNP]   = HSO_dispatch_pnp;
  DriverObject->MajorFunction[IRP_MJ_POWER] = HSO_power;
  DriverObject->DriverUnload                = HSO_unload; 
  DriverObject->DriverExtension->AddDevice  = HSO_add_device;
  DriverObject->MajorFunction[IRP_MJ_CLEANUP ] = HSO_cleanup;

  //now register as a HID minidriver
  RtlZeroMemory( &hid_minidriver_registration, 
		 sizeof(hid_minidriver_registration));

  hid_minidriver_registration.Revision            = HID_REVISION;
  hid_minidriver_registration.DriverObject        = DriverObject;
  hid_minidriver_registration.RegistryPath        = RegistryPath;
  hid_minidriver_registration.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
  hid_minidriver_registration.DevicesArePolled    = FALSE; 


  HSO_LOG( FILE_HIDSPORB |  HSO_MESSAGE1,
	   ("DeviceExtensionSize = %d",
	    hid_minidriver_registration.DeviceExtensionSize)
	   );

  nt_status = HidRegisterMinidriver( &hid_minidriver_registration );


  HSO_LOG(FILE_HIDSPORB | HSO_MESSAGE1,
	  ("Registered with HID.SYS, returnCode=%x",
	   nt_status)
	  );

  if( NT_SUCCESS(nt_status) )
    {
      /*  Protect the list with a Mutex  */
      ExInitializeFastMutex (&Global.mutex);
      /* Initialize the device list head */
      InitializeListHead(&Global.device_list_head);
      /* Initialize gameport access spinlock */
      KeInitializeSpinLock(&Global.spin_lock);
    }


  HSO_EXITPROC(FILE_HIDSPORB | HSO_FUNCTION_EXIT_OK , "DriverEntry", nt_status);

  return nt_status;
} /* DriverEntry */


/* Process the "create" irp to create the device object */
NTSTATUS 
HSO_create
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  PIO_STACK_LOCATION   p_irp_stack;
  NTSTATUS             nt_status = STATUS_SUCCESS;
  PDEVICE_EXTENSION device_extension;

  PAGED_CODE ();


  HSO_LOG(FILE_HIDSPORB | HSO_FUNCTION_ENTRY | HSO_MESSAGE1,
	  ("HSO_create(p_device_object=0x%x,p_irp=0x%x)",
	   p_device_object, p_irp) );

  /* currently just a stub.  In the debug log, it looks like
     HSO_create is never in fact called, since the HID class
     driver overwrites my entry points. */
  p_irp->IoStatus.Status = STATUS_SUCCESS;
  p_irp->IoStatus.Information = 0;
  IoCompleteRequest( p_irp, IO_NO_INCREMENT );
  return STATUS_SUCCESS;
} /* HSO_create */

/* Processes the "close" irp sent to this device */
NTSTATUS 
HSO_close
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  PIO_STACK_LOCATION   p_irp_stack;
  PDEVICE_EXTENSION    device_extension;
  NTSTATUS             nt_status = STATUS_SUCCESS;

  PAGED_CODE ();


  /* currently just a stub.  After checking the debug logs,
     it looks like this function is never called, presumably because
     the HID class driver overwrites our handling for these IRPs. */
  HSO_LOG(FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	  ("HSO_close(p_device_object=0x%x,p_irp=0x%x",
	   p_device_object, p_irp ) );
  p_irp->IoStatus.Status = STATUS_SUCCESS;
  p_irp->IoStatus.Information = 0;
  IoCompleteRequest( p_irp, IO_NO_INCREMENT );
  return STATUS_SUCCESS;
} /* HSO_close */


/* Called by the hid class driver; this basically allows us to initialize our
   device extensions. */
NTSTATUS  
HSO_add_device
(
 IN PDRIVER_OBJECT p_driver_object,
 IN PDEVICE_OBJECT p_functional_device_object
 )
{
  NTSTATUS                nt_status = STATUS_SUCCESS;
  PDEVICE_OBJECT          p_physical_device_object;
  PDEVICE_EXTENSION       p_device_extension;

  int i;

  PAGED_CODE ();

  HSO_LOG( FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	   ("HSO_add_device(p_driver_object=0x%x,p_functional_device_object=0x%x)",
	    p_driver_object, p_functional_device_object) );

  ASSERTMSG("HSO_add_device:", p_functional_device_object != NULL);
  // Initialize the device extension.
  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION( p_functional_device_object );
  p_physical_device_object = GET_PDO_FROM_FDO( p_functional_device_object );

  /* store a pointer to our driver object so that we can retrieve
     the registry path */
  p_device_extension->driver_object = p_driver_object;

  //init list
  InitializeListHead(&p_device_extension->link);

  //add this device to global list of devices
  ExAcquireFastMutex (&Global.mutex);
  InsertTailList(&Global.device_list_head, &p_device_extension->link);
  ExReleaseFastMutex (&Global.mutex);

  //init remove lock
  p_device_extension->request_count = 1;
  KeInitializeEvent(&p_device_extension->remove_event,
		    SynchronizationEvent,
		    FALSE);

  HSO_LOG( FILE_HIDSPORB | HSO_MESSAGE1, 
	   ("HSO_add_device: entering serial setup; pdo type 0x%x", p_physical_device_object->DeviceType));

    
  p_device_extension->is_removed = FALSE;
  p_device_extension->is_started = FALSE;
  /* initialize comm buffer */
  p_device_extension->packet_buffer_cursor = 0;
  p_device_extension->current_packet_type = HSO_unknown_packet;

  /* initialize thread fields */
  p_device_extension->is_thread_termination_requested = FALSE;
  p_device_extension->is_thread_started = FALSE;
  KeInitializeEvent(&(p_device_extension->is_thread_terminated), SynchronizationEvent, FALSE);

  /* initialize device mapping fields */
  for ( i = 0; i < MAX_LOGICAL_AXES; ++i )
    {
      p_device_extension->axis_map[ i ] = i;
      p_device_extension->sensitivities[ i ] = 0;
      p_device_extension->polarities[ i ] = HIDSPORB_POLARITY_POSITIVE;
      p_device_extension->gains[ i ] = 50;
    }
    
  p_device_extension->use_chording = TRUE;

  p_device_extension->orb_has_spoken = 0;
  p_device_extension->new_null_region_pending = FALSE;
  p_device_extension->null_region = 0;

  /* precision settings */
  p_device_extension->precision_sensitivity = 0;
  p_device_extension->precision_gain = 50;
  p_device_extension->precision_button_type = HIDSPORB_BUTTON_TYPE_NONE;
  p_device_extension->precision_button_index = 0;

  /* cache our FDO for writes to the orb  */
  p_device_extension->device_object = p_functional_device_object;


  IoInitializeRemoveLock(&p_device_extension->remove_lock, HSO_POOL_TAG, 0, 10 );
  p_device_extension->read_irp = IoAllocateIrp( p_physical_device_object->StackSize, FALSE );
  if (!p_device_extension->read_irp) 
    {
      HSO_LOG( FILE_HIDSPORB | HSO_MESSAGE1,
	       ( "HSO_add_device--couldn't allocate read IRP" ) );
      /* read IRP is critical to driver; if we can't get one, abort */
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  /* initialize our delay timer */
  KeInitializeTimer(&p_device_extension->delay_timer);

  /* initialize IRP queue and spinlock */
  HSO_initialize_read_irp_queue( p_device_extension );

  p_functional_device_object->Flags &= ~DO_DEVICE_INITIALIZING;
  p_physical_device_object->Flags &= ~DO_DEVICE_INITIALIZING;
  p_physical_device_object->Flags |= DO_BUFFERED_IO;
  p_physical_device_object->Flags |= DO_POWER_PAGABLE;

  HSO_EXITPROC(FILE_HIDSPORB | HSO_FUNCTION_EXIT_OK, "HSO_add_device", nt_status);

  return nt_status;
} /* HSO_add_device */



/* Unloads all the allocated resources... */
VOID 
HSO_unload
(
 IN PDRIVER_OBJECT p_driver_object
 )
{
  PAGED_CODE();
  HSO_LOG(FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	  ("HSO_unload Enter"));


  ASSERT ( NULL == p_driver_object->DeviceObject);

  /*unload our "driver extension" which held our registry path*/
  if ( Global.registry_base.Buffer )
    {
      ExFreePool( Global.registry_base.Buffer );
    }
  if ( Global.settings_path.Buffer )
    {
      ExFreePool( Global.settings_path.Buffer );
    }

  HSO_EXITPROC(FILE_HIDSPORB | HSO_FUNCTION_EXIT_OK, "HSO_unload:", STATUS_SUCCESS );
  return;
} /* HSO_unload */


NTSTATUS
HSO_flush_buffers(
		  IN PDEVICE_OBJECT p_device_object,
		  IN PIRP p_irp
		  )
{
  PDEVICE_EXTENSION  deviceExtension;
  NTSTATUS           status;

  /*  Get a pointer to the device extension. */
  deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  HSO_LOG( FILE_HIDSPORB | HSO_MESSAGE1, ("Flush enter") );

  status = IoAcquireRemoveLock(&deviceExtension->remove_lock, p_irp);
  if (!NT_SUCCESS(status)) 
    {
      return status;
    }

  IoSkipCurrentIrpStackLocation(p_irp);
  status = IoCallDriver(GET_NEXT_DEVICE_OBJECT( p_device_object ), p_irp);

  IoReleaseRemoveLock(&deviceExtension->remove_lock, p_irp);

  return status;
}



NTSTATUS
HSO_start_device(
		 PDEVICE_OBJECT device_object,
		 PDEVICE_EXTENSION device_extension,
		 PIRP p_irp,
		 BOOLEAN should_close_on_failure
		 )
{
  PIO_STACK_LOCATION  next;
  NTSTATUS            status;
  
  PAGED_CODE();

  HSO_LOG( FILE_HIDSPORB | HSO_FUNCTION_ENTRY,
	   ( "Entering StartDevice" ));

  status = HSO_initialize_serial_device( device_object, device_extension );

  if (NT_SUCCESS(status)) 
    {
      /*    status = HSO_spin_up_read(device_object, device_extension);*/
    }

  if (!NT_SUCCESS(status) && should_close_on_failure) 
    {
    
      HSO_LOG( FILE_HIDSPORB | HSO_ERROR,
	       ("sending close due to failure, 0x%x\n", status));

      // The start failed and we sent the create as part of the start
      // Send the matching cleanup/close so the port is accessible again.
      HSO_close_port(device_object, device_extension, p_irp);
    
      InterlockedDecrement(&device_extension->enable_count);
    }
  
  return status;
}



