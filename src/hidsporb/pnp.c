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
#ifndef PNP_H
#include "pnp.h"
#endif
#ifndef SERIAL_ROUTINES_H
#include "serial_routines.h"
#endif
#ifndef IOSYNC_H
#include "iosync.h"
#endif
#ifndef SETTINGS_H
#include "settings.h"
#endif
#ifndef IRP_QUEUE_H
#include "irp_queue.h"
#endif

#ifdef ALLOC_PRAGMA
/*  #pragma alloc_text (PAGE, HSO_remove_device) */
/*  #pragma alloc_text (PAGE, HSO_PnP) */
/*  #pragma alloc_text (PAGE, HSO_pnp_mn_start_device) */
/*  #pragma alloc_text (PAGE, HSO_pnp_mn_remove_device) */
/*  #pragma alloc_text (PAGE, HSO_init_device) */
/*  #pragma alloc_text (PAGE, show_device_property_string) */
/*  #pragma alloc_text (PAGE, HSO_get_resources) */
/*  #pragma alloc_text (PAGE, HSO_power) */
#endif


/* Increment request count of this device */
NTSTATUS  
HSO_increment_request_count
(
 IN PDEVICE_EXTENSION p_device_extension
 )
{
  NTSTATUS    nt_status;

  InterlockedIncrement( &p_device_extension->request_count );
  ASSERT( p_device_extension->request_count > 0 );
    
  if( p_device_extension->is_removed )
    {
      if( 0 == InterlockedDecrement( &p_device_extension->request_count ) ) 
        {
	  KeSetEvent( &p_device_extension->remove_event, IO_NO_INCREMENT, FALSE );
        }
      nt_status = STATUS_DELETE_PENDING;
    }
  else
    {
      nt_status = STATUS_SUCCESS;
    }

  return nt_status;
}
    



/* Decrement request count */
VOID 
HSO_decrement_request_count
(
 IN PDEVICE_EXTENSION p_device_extension
 )
{
  LONG        LocalCount;

  LocalCount = InterlockedDecrement( &p_device_extension->request_count );

  ASSERT( p_device_extension->request_count >= 0 );
    
  if( LocalCount == 0 )
    {
      ASSERT( p_device_extension->is_removed );
      KeSetEvent( &p_device_extension->remove_event, IO_NO_INCREMENT, FALSE );
    }

  return;
}
    

/* removal of the FDO */
VOID 
HSO_remove_device
(
 PDEVICE_OBJECT p_device_object,
 PDEVICE_EXTENSION p_device_extension,
 PIRP p_irp
 )
{
  HSO_LOG( FILE_PNP | HSO_FUNCTION_ENTRY,
		("HSO_remove_device entry; EnableCount == %d", 
		 p_device_extension->enable_count));

  if (!(p_device_extension->is_surprise_removed))
    {
      p_device_extension->is_surprise_removed = TRUE;

      /* Remove device from list, with mutex */
      ExAcquireFastMutex (&Global.mutex);
      RemoveEntryList(&p_device_extension->link);
      ExReleaseFastMutex (&Global.mutex);

      /* complete all pending IRPs */
      HSO_flush_read_report_queue( p_device_extension );

      if (p_device_extension->is_started && p_device_extension->enable_count > 0)
	{
	  HSO_LOG( FILE_PNP | HSO_MESSAGE1,
			("HSO_remove_device: cancelling/stopping detection for removal" ));
	  IoCancelIrp(p_device_extension->read_irp);
	}
    }

  /* bout to tear down everything-- moke sure the serial port is closed! */
  if (p_device_extension->is_removed && (p_device_extension->enable_count > 0))
    {
      HSO_LOG( FILE_PNP | HSO_MESSAGE1,
		    ("Sending fine close; EnableCount == %d",
		     p_device_extension->enable_count ));
      p_device_extension->enable_count = 0;
      HSO_close_port( p_device_object, p_device_extension, p_irp );
      /*save settings to the registry*/
      HSO_save_settings( p_device_extension );
    }

} /* HSO_remove_device */

/* Dispatch PNP ioctls */
NTSTATUS  
HSO_dispatch_pnp
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  NTSTATUS nt_status;
  PDEVICE_EXTENSION p_device_extension;
  KEVENT            StartEvent;

  PAGED_CODE();

  HSO_LOG(FILE_PNP | HSO_FUNCTION_ENTRY,\
	       ("HSO_dispatch_pnp(p_device_object=0x%x,p_irp=0x%x)",\
		p_device_object, p_irp ));

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION(p_device_object);

  nt_status = HSO_increment_request_count( p_device_extension );
  if (!NT_SUCCESS (nt_status))
    {
      // plug and play IRP after removed
      HSO_LOG(FILE_PNP | HSO_ERROR,\
		   ("HSO_dispatch_pnp: PnP IRP after device was removed\n"));
      p_irp->IoStatus.Information = 0;
      p_irp->IoStatus.Status = nt_status;
      IoCompleteRequest (p_irp, IO_NO_INCREMENT);
    } 
  else
    {
      PIO_STACK_LOCATION p_irp_stack;

      p_irp_stack = IoGetCurrentIrpStackLocation (p_irp);

      switch(p_irp_stack->MinorFunction)
	{
	case IRP_MN_START_DEVICE:
	  nt_status = HSO_pnp_mn_start_device( p_device_object, p_device_extension, p_irp );
	  break;

	case IRP_MN_STOP_DEVICE:

	  HSO_LOG(FILE_PNP | HSO_MESSAGE1,\
		  ("HSO_Pnp: IRP_MN_STOP_DEVICE"));
	  //after start IRP has been passed down the stack to lower
	  //driver objects, we can't send any more IRPs down the bus
	  //until another START has occurred...?

	  p_device_extension->is_started = FALSE;

	  /* don't need a completion routine here */

	  IoSkipCurrentIrpStackLocation (p_irp);
	  nt_status = IoCallDriver (GET_NEXT_DEVICE_OBJECT(p_device_object), p_irp);
	  break;

	case IRP_MN_SURPRISE_REMOVAL:
	  HSO_LOG(FILE_PNP | HSO_MESSAGE1,\
		  ("HSO_Pnp: IRP_MN_SURPRISE_REMOVAL"));

	  HSO_remove_device(p_device_object, p_device_extension, p_irp);

	  p_irp->IoStatus.Status = STATUS_SUCCESS;
	  IoSkipCurrentIrpStackLocation(p_irp);
	  nt_status = IoCallDriver (GET_NEXT_DEVICE_OBJECT(p_device_object), p_irp);

	  break;

	case IRP_MN_REMOVE_DEVICE:
	  nt_status = HSO_pnp_mn_remove_device( p_device_object, 
					       p_device_extension, p_irp );
	  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT_OK, 
		       "HSO_dispatch_pnp Exit 1", nt_status);

	  return nt_status;

	default:
	  HSO_LOG(FILE_PNP | HSO_WARNING,\
		  ("HSO_dispatch_pnp: IrpStack->MinorFunction Not handled 0x%x", \
		   p_irp_stack->MinorFunction));

	  IoSkipCurrentIrpStackLocation (p_irp);

	  nt_status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(p_device_object), p_irp);
	  break;
	}

      HSO_decrement_request_count( p_device_extension );
    }

  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT, "HSO_dispatch_pnp", nt_status);

  return nt_status;
} /* HSO_dispatch_pnp */


NTSTATUS
HSO_pnp_mn_remove_device(
			 PDEVICE_OBJECT p_device_object,
			 PDEVICE_EXTENSION p_device_extension,
			 PIRP p_irp
			 )
{
  NTSTATUS nt_status;

  PAGED_CODE();

  HSO_LOG(FILE_PNP | HSO_MESSAGE1,\
	       ("HSO_Pnp: IRP_MN_REMOVE_DEVICE"));

  /* Detach and delete the device object */
  p_device_extension->is_removed = TRUE;

  /* stop the read thread */
  p_device_extension->is_thread_termination_requested = TRUE;
  KeWaitForSingleObject( &(p_device_extension->is_thread_terminated),
			 Executive, KernelMode, FALSE, NULL );
    
  /* stop device */
  HSO_remove_device(p_device_object, p_device_extension, p_irp);

  /* pass on the "remove" irp */
  IoSkipCurrentIrpStackLocation (p_irp);
  nt_status = IoCallDriver (GET_NEXT_DEVICE_OBJECT(p_device_object), p_irp);


  /* Decrement our request count for this irp.  State should now have
     initial 1 irp plus any other irp holds */
  {
    LONG request_count = InterlockedDecrement( &p_device_extension->request_count );
    ASSERT( request_count > 0 );
  }

  /* Finally, wait for any pending IRP/requests to finish */
  if( InterlockedDecrement( &p_device_extension->request_count ) > 0 )
    {
      KeWaitForSingleObject( &p_device_extension->remove_event,
			     Executive, KernelMode, FALSE, NULL );
    }

  nt_status = STATUS_SUCCESS;
  return nt_status;
}

NTSTATUS
HSO_pnp_mn_start_device( 
			PDEVICE_OBJECT p_device_object, 
			PDEVICE_EXTENSION p_device_extension,
			PIRP p_irp 
			)
{
  NTSTATUS nt_status;
  NTSTATUS registry_status;
  HANDLE key_handle;
  PAGED_CODE();

  HSO_LOG(FILE_PNP | HSO_MESSAGE1,\
	       ("HSO_Pnp: IRP_MN_START_DEVICE"));
  
  nt_status = HSO_send_irp_synchronously( GET_NEXT_DEVICE_OBJECT(p_device_object),
				       p_irp,
				       TRUE );
		
  if (NT_SUCCESS(nt_status) && NT_SUCCESS( p_irp->IoStatus.Status))
    {
      PIO_STACK_LOCATION nextStack;
      /* if a create has not been sent down the stack yet,
	 then send one now.  Serial port driver requires a
	 create before any reads or IOCTLs are to be sent */
      if (InterlockedIncrement(&(p_device_extension->enable_count)) == 1 )
	{
	  NTSTATUS prevStatus;
	  ULONG_PTR prevInformation;
	  /* no previous create has been sent.  Do it now! */
	  prevStatus = p_irp->IoStatus.Status;
	  prevInformation = p_irp->IoStatus.Information;
			
	  nextStack = IoGetNextIrpStackLocation( p_irp );
	  RtlZeroMemory( nextStack, sizeof( IO_STACK_LOCATION ) );
	  nextStack->MajorFunction = IRP_MJ_CREATE;

	  nt_status = HSO_send_irp_synchronously( GET_NEXT_DEVICE_OBJECT( p_device_object ),
					       p_irp,
					       FALSE );

	  HSO_LOG( FILE_PNP | HSO_MESSAGE1,
			( "Create for start, status = 0x%x", nt_status ));

	  if (NT_SUCCESS(nt_status) && NT_SUCCESS(p_irp->IoStatus.Status) )
	    {
	      p_irp->IoStatus.Status = prevStatus;
	      p_irp->IoStatus.Information = prevInformation;
	    }
	}
    }

  if ( NT_SUCCESS( nt_status ) && (NT_SUCCESS( p_irp->IoStatus.Status ) ) )
    {
      registry_status = IoOpenDeviceRegistryKey( GET_PDO_FROM_FDO( p_device_object ),
						  PLUGPLAY_REGKEY_DEVICE,
						  STANDARD_RIGHTS_READ,
						  &key_handle );
      HSO_retrieve_settings_from_registry( p_device_object, p_device_extension, key_handle );
      ZwClose( key_handle );
    }
			
  if(NT_SUCCESS(nt_status) && NT_SUCCESS(p_irp->IoStatus.Status))
    {
      nt_status = HSO_init_device (p_device_object, p_irp);
    } 
  else
    {
      HSO_LOG(FILE_PNP | HSO_ERROR,\
		   ("HSO_Pnp: IRP_MN_START_DEVICE nt_status =0x%x",\
		    nt_status));
    }


  p_device_extension->is_started = TRUE;

  HSO_LOG( FILE_PNP | HSO_MESSAGE1, \
		( "spinning up read in start, started = 0x%x", p_device_extension->is_started ));
  /* this was "spin_up_read"; thread architecture suggested by Yuri */
  nt_status = HSO_start_read_thread( p_device_object, p_device_extension );

  p_irp->IoStatus.Information = 0;
  p_irp->IoStatus.Status = nt_status;
  IoCompleteRequest (p_irp, IO_NO_INCREMENT);
  HSO_EXITPROC( FILE_PNP | HSO_FUNCTION_EXIT,
		"mn_start exit", nt_status );
  return nt_status;
}


/*
  Handle the "init device" IRP
*/
NTSTATUS 
HSO_init_device
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  NTSTATUS nt_status = STATUS_SUCCESS;
  PDEVICE_EXTENSION   p_device_extension;
  ULONG  DescriptorLength;

  PAGED_CODE();

  HSO_LOG(FILE_PNP | HSO_FUNCTION_ENTRY,\
	       ("HSO_init_device(p_device_object=0x%x,Irp=0x%x)", \
		p_device_object,p_irp));

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION(p_device_object);

  /* init serial port */
  nt_status = HSO_initialize_serial_device( p_device_object, p_device_extension );
  /* allocate resources etc */
  if ( NT_SUCCESS( nt_status ) )
    {
      nt_status = HSO_get_resources(p_device_object,p_irp);
    }

  if( !NT_SUCCESS(nt_status) )
    {
      ExAcquireFastMutex (&Global.mutex);
      RemoveEntryList(&p_device_extension->link);
      ExReleaseFastMutex (&Global.mutex);
    }

  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT_OK, "HSO_init_device", nt_status);

  return nt_status;
} /* HSO_init_device */


/*debug routine to debug-print information about a device*/
void
show_device_property_string(PDEVICE_OBJECT pdo,
			    char* property_desc,
			    DEVICE_REGISTRY_PROPERTY property
			    )
{
#define show_device_buffer_length 200
  ULONG result_length = 0;
  char buffer[show_device_buffer_length];
  NTSTATUS status = STATUS_SUCCESS;
  status = IoGetDeviceProperty( pdo, property, show_device_buffer_length, buffer, &result_length );
  if NT_SUCCESS(status) 
    {
      HSO_LOG( FILE_PNP | HSO_MESSAGE1,
		    ("Device property %s: %ws", property_desc, buffer ));
    }
  else
    { 
      HSO_LOG( FILE_PNP | HSO_MESSAGE1,
		    ("Error checking device property %s", property_desc ) );
    }
}

/* allocates resources which will be used by the driver.  I try and get
   resource information from Serenum... but unfortunately, I no longer 
   remember if any of this is actually used by the driver in a meaningful
   way.  This function may be superfluous (I'm just nervous about
   deleting it) */
NTSTATUS 
HSO_get_resources
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  NTSTATUS            nt_status = STATUS_SUCCESS;
  SERENUM_PORT_DESC   PortDesc;
  SERIAL_BAUD_RATE    BaudRate;
  SERIAL_LINE_CONTROL LineControl;
  PDEVICE_EXTENSION   p_device_extension;
  KEVENT              IoctlCompleteEvent;
  IO_STATUS_BLOCK     IoStatus;
  PIO_STACK_LOCATION  irpStack, nextStack;
  IO_STATUS_BLOCK     io_status;
  int                 i;
  PIRP test_irp;
  char buffer[100];
  ULONG result_length;
  PAGED_CODE ();

  HSO_LOG(FILE_PNP | HSO_FUNCTION_ENTRY,\
	       ("HSO_get_resources(p_device_object=0x%x,p_irp=0x%x)",\
		p_device_object, p_irp));

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION(p_device_object);
  HSO_LOG(FILE_PNP | HSO_MESSAGE1,\
	       ("calling Serial Now"));
  /*getting information just for fun...*/
  HSO_LOG( FILE_PNP | HSO_MESSAGE1, \
		("NextDevObj DeviceType: 0x%x", GET_NEXT_DEVICE_OBJECT(p_device_object)->DeviceType ) );
  show_device_property_string( GET_NEXT_DEVICE_OBJECT( p_device_object ), "Description", DevicePropertyDeviceDescription );
  show_device_property_string( GET_NEXT_DEVICE_OBJECT( p_device_object ), "Location", DevicePropertyLocationInformation );
  show_device_property_string( GET_NEXT_DEVICE_OBJECT( p_device_object ), "PDO name", DevicePropertyPhysicalDeviceObjectName );
  show_device_property_string( GET_NEXT_DEVICE_OBJECT( p_device_object ), "enumerator name", DevicePropertyEnumeratorName );
    
  /*time to try some more IRP madness*/

  KeInitializeEvent(&IoctlCompleteEvent, NotificationEvent, FALSE);

  PortDesc.Size = sizeof (PortDesc );
  test_irp = IoBuildDeviceIoControlRequest(
					   IOCTL_SERENUM_PORT_DESC,
					   GET_NEXT_DEVICE_OBJECT( p_device_object ),
					   &PortDesc,
					   PortDesc.Size,
					   &PortDesc,
					   PortDesc.Size,
					   FALSE,
					   &IoctlCompleteEvent,
					   &io_status );

  IoSetCompletionRoutine (test_irp, HSO_irp_complete, &IoctlCompleteEvent, TRUE, TRUE, TRUE);    
  nt_status = IoCallDriver(GET_NEXT_DEVICE_OBJECT( p_device_object), test_irp );

  HSO_LOG( FILE_PNP | HSO_MESSAGE1, ("659: Ntstatus= 0x%x, IO status = 0x%x, size=0x%x:0x%x", nt_status, io_status.Status, io_status.Information, sizeof(PortDesc) ) );
  IoCompleteRequest( test_irp, IO_NO_INCREMENT );

  nt_status = HSO_get_line_control( p_device_object, &LineControl );

  HSO_LOG( FILE_PNP | HSO_MESSAGE1, ("558: Ntstatus= 0x%x", nt_status ) );


  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT_OK, "HSO_get_resources", p_irp->IoStatus.Status);

  /* BIG NO-NO */
  return STATUS_SUCCESS; /*p_irp->IoStatus.Status;*/
} /* HSO_get_resources */




/* Handles power IRPs.  We don't actually process any power IRPs, because
   I don't quite understand what's expected, so all we do is pass 'em on to
   the next device on the IRP stack. */
NTSTATUS 
HSO_power
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  PDEVICE_EXTENSION  p_device_extension;
  NTSTATUS           nt_status;

  PAGED_CODE ();

  HSO_LOG(FILE_PNP | HSO_FUNCTION_ENTRY,\
	       ("Enter HSO_power(p_device_object=0x%x,p_irp=0x%x)",p_device_object, p_irp));

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  nt_status = HSO_increment_request_count( p_device_extension );
  if (!NT_SUCCESS (nt_status))
    {
      HSO_LOG(FILE_PNP | HSO_ERROR,\
		   ("HSO_power: PnP IRP after device was removed\n"));
      p_irp->IoStatus.Information = 0;
      p_irp->IoStatus.Status = nt_status;
      IoCompleteRequest (p_irp, IO_NO_INCREMENT);
    } 
  else
    {
      IoSkipCurrentIrpStackLocation (p_irp);

      PoStartNextPowerIrp (p_irp);
      
      nt_status =  PoCallDriver (GET_NEXT_DEVICE_OBJECT (p_device_object), p_irp);
      
      HSO_decrement_request_count( p_device_extension );
    }


  HSO_EXITPROC(FILE_IOCTL | HSO_FUNCTION_EXIT, "HSO_power", nt_status);
  return nt_status;
} /* HSO_power */



