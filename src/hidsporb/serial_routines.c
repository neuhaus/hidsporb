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
#ifndef SERIAL_ROUTINES_H
#include "serial_routines.h"
#endif

#ifndef IOSYNC_H
#include "iosync.h"
#endif

#ifndef ORB_COMM_H
#include "orb_comm.h"
#endif

#include "debug.h"
#include "hidport.h"

#ifdef ALLOC_PRAGMA
  /*  #pragma alloc_text (PAGE, HSO_spin_up_read) */
#endif


  // pauses expressed in nanoseconds
#define PAUSE_1_5_SECONDS       (1500 * 1000 * 10)
#define PAUSE_200_MS            (200 * 1000 * 10)
#define PAUSE_150_MS            (150 * 1000 * 10)

#define MS_TO_100_NS            10000



  NTSTATUS
    HSO_get_line_control(
			 PDEVICE_OBJECT p_device_object, /* our fdo */
			 PSERIAL_LINE_CONTROL    serial_line_control
			 )
{
  IO_STATUS_BLOCK     iosb;
  KEVENT              event;
  NTSTATUS            status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY, ("get_line_control" ));

  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);

  status = HSO_sync_ioctl_extra(
				IOCTL_SERIAL_GET_LINE_CONTROL,
				GET_NEXT_DEVICE_OBJECT( p_device_object ),
				&event,
				&iosb,
				NULL,
				0,
				serial_line_control,
				sizeof(SERIAL_LINE_CONTROL));

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	       ("get_line_control failed! (%x)", status));
    }

  HSO_EXITPROC(FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, "get_line_control", status);

  return status;
}


NTSTATUS
HSO_set_line_control(
		     PDEVICE_OBJECT device_object, /* our fdo */
		     PSERIAL_LINE_CONTROL    serial_line_control
		     )
{
  IO_STATUS_BLOCK     iosb;
  KEVENT              event;
  NTSTATUS            status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY, ("set_line_control entry" ));

  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);

  status = HSO_sync_ioctl_extra(
				IOCTL_SERIAL_SET_LINE_CONTROL,
				GET_NEXT_DEVICE_OBJECT( device_object ), 
				&event,
				&iosb,
				serial_line_control,
				sizeof(SERIAL_LINE_CONTROL),
				NULL,
				0);

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "set_line_control exit", status );
    }

  else 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, 
		    "set_line_control_exit", status );
    }

  return status;
}




VOID
HSO_close_port(
	       PDEVICE_OBJECT device_object,
	       PDEVICE_EXTENSION device_extension,
	       PIRP Irp
	       )
{
  PIO_STACK_LOCATION next;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ( "Entering Close_Port" ));
  HSO_detect_spaceorb( device_object, device_extension );
  
  HSO_restore_port( device_object, device_extension );

  next = IoGetNextIrpStackLocation (Irp);
  RtlZeroMemory( next, sizeof( IO_STACK_LOCATION ) );
  next->MajorFunction = IRP_MJ_CLEANUP;
  HSO_send_irp_synchronously( GET_NEXT_DEVICE_OBJECT( device_object ), Irp, FALSE );

  next = IoGetNextIrpStackLocation (Irp);
  RtlZeroMemory( next, sizeof( IO_STACK_LOCATION ) );
  next->MajorFunction = IRP_MJ_CLOSE;
  HSO_send_irp_synchronously( GET_NEXT_DEVICE_OBJECT( device_object ), Irp, FALSE );
  HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, "HSO_Close_Port", STATUS_SUCCESS );
}
  

VOID
HSO_restore_port(
		 PDEVICE_OBJECT device_object,
		 PDEVICE_EXTENSION device_extension
		 )
{
  KEVENT          event;
  IO_STATUS_BLOCK iosb;
  NTSTATUS        status;

  PAGED_CODE();
  
  HSO_LOG(FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	  ("HSO_restore_port Enter"));

  KeInitializeEvent(&event, NotificationEvent, FALSE);

  status =
    HSO_sync_internal_ioctl_extra(IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS,
				  GET_NEXT_DEVICE_OBJECT( device_object ),
				  &event,
				  &iosb,
				  &device_extension->serial_port_basic_settings,
				  sizeof(SERIAL_BASIC_SETTINGS),
				  NULL,
				  0);
  //at the time, the serial.sys driver couldn't handle the 
  //IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS irp, so this code has to be here
  //to handle failures; eventually this can probably be discarded.
  if (!NT_SUCCESS(status)) 
    {
      HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_TIMEOUTS, 
			   GET_NEXT_DEVICE_OBJECT( device_object ),
			   &event,
			   &iosb,
			   &device_extension->serial_port_basic_settings.Timeouts,
			   sizeof(SERIAL_TIMEOUTS),
			   NULL,
			   0);

      HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_HANDFLOW, 
			   GET_NEXT_DEVICE_OBJECT( device_object ),
			   &event,
			   &iosb,
			   &device_extension->serial_port_basic_settings.HandFlow,
			   sizeof(SERIAL_HANDFLOW),
			   NULL,
			   0);
    }
  HSO_EXITPROC(FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, 
	       "HSO_restore_port:", STATUS_SUCCESS );

}


NTSTATUS
HSO_initialize_port(
		    PDEVICE_OBJECT device_object,
		    PDEVICE_EXTENSION device_extension
		    )
{
  NTSTATUS        status;
  KEVENT          event;
  IO_STATUS_BLOCK iosb;
  SERIAL_TIMEOUTS serial_timeouts;
  SERIAL_HANDFLOW serial_handshake_flowcontrol;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("Entering HSO_initialize_port" ));

  KeInitializeEvent(&event, NotificationEvent, FALSE);

  status =
    HSO_sync_internal_ioctl_extra(IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS,
				  GET_NEXT_DEVICE_OBJECT( device_object ),
				  &event,
				  &iosb,
				  NULL,
				  0,
				  &device_extension->serial_port_basic_settings,
				  sizeof(SERIAL_BASIC_SETTINGS));

  //
  // In case we are running on a port that does not support basic settings
  //
  if (!NT_SUCCESS(status)) 
    {
      HSO_sync_ioctl_extra(IOCTL_SERIAL_GET_TIMEOUTS,
			   GET_NEXT_DEVICE_OBJECT( device_object ),
			   &event,
			   &iosb,
			   NULL,
			   0,
			   &device_extension->serial_port_basic_settings.Timeouts,
			   sizeof(SERIAL_TIMEOUTS));
  
      RtlZeroMemory(&serial_timeouts, sizeof(SERIAL_TIMEOUTS));
  
      HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_TIMEOUTS,
			   GET_NEXT_DEVICE_OBJECT( device_object ),
			   &event,
			   &iosb,
			   &serial_timeouts,
			   sizeof(SERIAL_TIMEOUTS),
			   NULL,
			   0);

      HSO_sync_ioctl_extra(IOCTL_SERIAL_GET_HANDFLOW,
			   GET_NEXT_DEVICE_OBJECT( device_object ),
			   &event,
			   &iosb,
			   NULL,
			   0,
			   &device_extension->serial_port_basic_settings.HandFlow,
			   sizeof(SERIAL_HANDFLOW));

      serial_handshake_flowcontrol.ControlHandShake = SERIAL_DTR_CONTROL;
      serial_handshake_flowcontrol.FlowReplace = SERIAL_RTS_CONTROL;
      serial_handshake_flowcontrol.XonLimit = 0;
      serial_handshake_flowcontrol.XoffLimit = 0;
     
      status = HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_HANDFLOW, 
				    GET_NEXT_DEVICE_OBJECT( device_object ),
				    &event,
				    &iosb,
				    &serial_handshake_flowcontrol,
				    sizeof(SERIAL_HANDFLOW),
				    NULL,
				    0);
    }
  else
    {
      /*display the old settings */
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	       ("Old Handflow: %x Old replace: %x", 
		device_extension->serial_port_basic_settings.HandFlow.ControlHandShake,
		device_extension->serial_port_basic_settings.HandFlow.FlowReplace ));
    }
  HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, "HSO_initialize_port exit", status );
  return status;
}



void
HSO_set_wait_mask_to_zero(
			  PDEVICE_OBJECT device_object
			  )
{
  KEVENT event;
  ULONG wait_mask;
  IO_STATUS_BLOCK iosb;
  /* set the wait mask to zero so that when we send the wait
     request it won't get completed due to init flipping lines.

     I don't know what this means, but mouser did it.  May be removed
     in the future
  */
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("HSO_set_wait_mask_to_zero entry"));
  wait_mask = 0x0;
  KeInitializeEvent( &event, NotificationEvent, FALSE );

  HSO_sync_ioctl_extra( IOCTL_SERIAL_SET_WAIT_MASK,
			GET_NEXT_DEVICE_OBJECT( device_object ),
			&event,
			&iosb,
			&wait_mask,
			sizeof( ULONG ),
			NULL,
			0);
}



NTSTATUS
HSO_serial_wait (
		 PDEVICE_EXTENSION    device_extension,
		 LONG                 timeout
		 )
     //performs a wait for the specified time; negative values for timeout
     //represent relative pause; positive time represents absolute
     //time to wait until
{
  LARGE_INTEGER time = RtlConvertLongToLargeInteger(timeout); 

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("waiting for %d micro secs", timeout));

  if (KeSetTimer(&device_extension->delay_timer,
		 time,
		 NULL)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	       ("Timer already set"));
    }

  return KeWaitForSingleObject(&device_extension->delay_timer,
			       Executive,
			       KernelMode,
			       FALSE,             // Not allertable
			       NULL);             // No timeout structure
}


NTSTATUS
HSO_power_down( 
	       PDEVICE_OBJECT device_object, 
	       PDEVICE_EXTENSION device_extension
	       )
{
  IO_STATUS_BLOCK     iosb;
  SERIAL_HANDFLOW     shf;
  KEVENT              event;
  NTSTATUS            status;
  ULONG               bits;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("entry: HSO_power_down" ));

  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE
		    );

#if 0
  // Set the handflow to default values
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("Setting handflow to default values..."));
  shf.ControlHandShake = SERIAL_DTR_CONTROL;
  shf.FlowReplace = SERIAL_RTS_CONTROL;
  shf.XonLimit = 0;
  shf.XoffLimit = 0;
  status = HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_HANDFLOW,
				GET_NEXT_DEVICE_OBJECT( device_object ),
				&event,
				&iosb,
				&shf,
				sizeof(SERIAL_HANDFLOW),
				NULL,
				0);
  if (!NT_SUCCESS(status)) 
    {
      return status;
    }
#endif

  // Set DTR
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("Setting DTR..."));
  status = HSO_sync_ioctl(IOCTL_SERIAL_SET_DTR,
			  GET_NEXT_DEVICE_OBJECT( device_object ),
			  &event,
			  &iosb);
  if (!NT_SUCCESS(status)) 
    {
      return status; 
    }
        
  // Clear RTS
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("Clearing RTS..."));
  status = HSO_sync_ioctl(IOCTL_SERIAL_CLR_RTS,
			  GET_NEXT_DEVICE_OBJECT( device_object ),
			  &event,
			  &iosb);
  if (!NT_SUCCESS(status)) 
    {
      return status;
    }

  //
  // Set a timer for 200 ms
  //
  status = HSO_serial_wait( device_extension, -PAUSE_200_MS);
  if (!NT_SUCCESS(status)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_ERROR,
	       ("Timer failed with status %x", status));
    }
    
  else
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK,
		    "power_down exit", status );
    }

  return status;
    
}

NTSTATUS
HSO_get_baud_rate(
		  PDEVICE_OBJECT device_object, /* our FDO */
		  PULONG            BaudRate
		  )
{
  SERIAL_BAUD_RATE    sbr;
  IO_STATUS_BLOCK     iosb;
  KEVENT              event;
  NTSTATUS            status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("HSO_get_baud_rate enter"));
    
  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);

  status = HSO_sync_ioctl_extra(
				IOCTL_SERIAL_GET_BAUD_RATE,
				GET_NEXT_DEVICE_OBJECT( device_object ),
				&event,
				&iosb,
				NULL,
				0,
				&sbr,
				sizeof(SERIAL_BAUD_RATE));

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	       ("get_baud_rate failed (%x)", status));
    }
  else 
    {
      *BaudRate = sbr.BaudRate;
    }

  HSO_EXITPROC(FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, 
	       "get_baud_rate", status);

  return status;
}

NTSTATUS
HSO_set_baud_rate(
		  PDEVICE_OBJECT device_object, /* our hidsporb fdo */
		  ULONG               BaudRate
		  )
{
  SERIAL_BAUD_RATE    sbr;
  IO_STATUS_BLOCK     iosb;
  KEVENT              event;
  NTSTATUS            status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("HSO_set_baud_rate entry"));

  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);

  sbr.BaudRate = BaudRate;
  status = HSO_sync_ioctl_extra(
				IOCTL_SERIAL_SET_BAUD_RATE,
				GET_NEXT_DEVICE_OBJECT( device_object ), 
				&event,
				&iosb,
				&sbr,
				sizeof(SERIAL_BAUD_RATE),
				NULL,
				0);

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	       ( "set_baud_rate failed 0x%x", status ));

    }
  else 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, 
		    "set_baud_rate exit", status );

    }

  return status;
}


NTSTATUS
HSO_flush_read_buffer(
		      PDEVICE_OBJECT device_object /* our fdo */
		      )
{
  ULONG           bits = SERIAL_PURGE_RXCLEAR;
  NTSTATUS 		status;
  KEVENT			event;
  IO_STATUS_BLOCK iosb;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY, 
	   ("flush_read_buffer entry"));


  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);

  status = HSO_sync_ioctl_extra(
				IOCTL_SERIAL_PURGE, 
				GET_NEXT_DEVICE_OBJECT( device_object ), 
				&event,
				&iosb,
				&bits,
				sizeof(ULONG),
				NULL,
				0);

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "flush_read_buffer exit", status );
    }

  else
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK,
		    "flush_read_buffer exit", status );
    }

  return status;
}




NTSTATUS
HSO_power_up(
	     PDEVICE_OBJECT device_object, /* our fdo */
	     PDEVICE_EXTENSION device_extension
	     )
{
  IO_STATUS_BLOCK     iosb;
  NTSTATUS            status;
  KEVENT              event;


  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("power_up entry"));


  KeInitializeEvent(&event, NotificationEvent, FALSE);

  // Clear DTR
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("clearing DTR..."));
  status = HSO_sync_ioctl(IOCTL_SERIAL_CLR_DTR,
			  GET_NEXT_DEVICE_OBJECT( device_object ),
			  &event,
			  &iosb
			  );

  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;  
    }
                
  // Clear RTS
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("clearing RTS..."));
  status = HSO_sync_ioctl(IOCTL_SERIAL_CLR_RTS,
			  GET_NEXT_DEVICE_OBJECT( device_object ), 
			  &event,
			  &iosb
			  );
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;
    }
                
  // Set a timer for 200 ms
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("waiting 200 ms..."));
  status = HSO_serial_wait(device_extension, -PAUSE_200_MS);
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;      
    }

  // set DTR
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("setting DTR..."));
  status = HSO_sync_ioctl(IOCTL_SERIAL_SET_DTR,
			  GET_NEXT_DEVICE_OBJECT( device_object ),
			  &event,
			  &iosb
			  );
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;
    }
        
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("waiting 200ms..." ));
  status = HSO_serial_wait(device_extension, -PAUSE_200_MS);
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;
    }                                 

  // set RTS      
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("Setting RTS..." ));
  status = HSO_sync_ioctl(IOCTL_SERIAL_SET_RTS,
			  GET_NEXT_DEVICE_OBJECT( device_object ),
			  &event,
			  &iosb
			  );
  if (!NT_SUCCESS( status ))
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;
    }

  //wait a few more milliseconds
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("waiting 175 ms"));
  status = HSO_serial_wait(device_extension, -175 * MS_TO_100_NS);
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES, "power_up exit", status );
      return status;
    }                                 
  
  HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK, 
		"power_up exit", status );

  return status;
}



NTSTATUS
HSO_set_read_timeouts(
		      PDEVICE_OBJECT device_object, /* our fdo */
		      ULONG               Timeout
		      )
{
  NTSTATUS        status;
  SERIAL_TIMEOUTS serial_timeouts;
  KEVENT          event;
  IO_STATUS_BLOCK iosb;

  KeInitializeEvent(&event, NotificationEvent, FALSE);
  RtlZeroMemory(&serial_timeouts, sizeof(SERIAL_TIMEOUTS));

  if (Timeout != 0) 
    {
      serial_timeouts.ReadIntervalTimeout = MAXULONG;
      serial_timeouts.ReadTotalTimeoutMultiplier = MAXULONG;
      serial_timeouts.ReadTotalTimeoutConstant = Timeout;
    }
    
  status =  HSO_sync_ioctl_extra(IOCTL_SERIAL_SET_TIMEOUTS, 
				 GET_NEXT_DEVICE_OBJECT( device_object ),
				 &event,
				 &iosb,
				 &serial_timeouts,
				 sizeof(SERIAL_TIMEOUTS),
				 NULL,
				 0);

  return status;
}

NTSTATUS
HSO_read_serial_port_complete(
			      IN PDEVICE_OBJECT       p_device_object,
			      IN PIRP                 Irp,
			      IN PKEVENT              Event
			      )
{
  UNREFERENCED_PARAMETER(p_device_object);

  KeSetEvent(Event, 0, FALSE);

  //mark status in the IRP?  
  //Set IRP information?
  //evidently not necessary; see definition of IoMarkIrpPending on
  //the MSDN site: "Note, however, that a driver that passes down
  //the IRP and then waits on an event should not mark the IRP
  //pending. Instead, its IoCompletion routine should signal 
  //the event and return STATUS_MORE_PROCESSING_REQUIRED. 


  return STATUS_MORE_PROCESSING_REQUIRED;
}


/*
  strangeness.  If we include both the ntddk.h and wdm.h files,
  we get all kinds of errors for redefinition--but wdm.h does not
  contain the definition for IoReuseIrp.  So we duplicated it here.
*/
NTKERNELAPI
VOID
IoReuseIrp(
	   IN OUT PIRP Irp,
	   IN NTSTATUS Iostatus
	   );


/* performs simple synchronous read from the serial port */
NTSTATUS
HSO_read_serial_port (
		      PDEVICE_OBJECT device_object,
		      PDEVICE_EXTENSION   p_device_extension,
		      PCHAR               read_buffer,
		      USHORT              Buflen,
		      PUSHORT             ActualBytesRead
		      )
{
  NTSTATUS            status = STATUS_SUCCESS;
  PIRP                irp;
  KEVENT              event;
  IO_STATUS_BLOCK     iosb;
  PIO_STACK_LOCATION  stack;
  SERIAL_TIMEOUTS     serial_timeouts;
  int                 i, numReads;

  KeInitializeEvent(&event, NotificationEvent, FALSE);

  if (!NT_SUCCESS(status)) 
    {
      return status;
    }

  irp = p_device_extension->read_irp;

  *ActualBytesRead = 0;
  while (*ActualBytesRead < Buflen) 
    {

      KeClearEvent(&event);
      IoReuseIrp( irp, STATUS_SUCCESS );
    
      irp->AssociatedIrp.SystemBuffer = read_buffer;
    
      stack = IoGetNextIrpStackLocation(irp);
      stack->Parameters.Read.Length = 1;          
      stack->Parameters.Read.ByteOffset.QuadPart = 0;
      stack->MajorFunction = IRP_MJ_READ;

      // Hook a completion routine for when the device completes.
      IoSetCompletionRoutine(irp,
			     HSO_read_serial_port_complete,
			     &event,
			     TRUE,
			     TRUE,
			     TRUE);


      status = IoCallDriver(GET_NEXT_DEVICE_OBJECT( device_object), irp);

      if (status == STATUS_PENDING) 
	{
	  // Wait for the IRP
	  status = KeWaitForSingleObject(&event,
					 Executive,
					 KernelMode,
					 FALSE,
					 NULL);

	  if (status == STATUS_SUCCESS) 
	    {
	      status = irp->IoStatus.Status;
	    }
	}

      if (!NT_SUCCESS(status) || status == STATUS_TIMEOUT) 
	{
	  return status;
	}

      ASSERT(irp->IoStatus.Information >= 0);
      ASSERT(irp->IoStatus.Information <= 1);

      *ActualBytesRead += (USHORT) irp->IoStatus.Information;
      read_buffer += (USHORT) irp->IoStatus.Information;
    }

  return status;
}

/* Reads a single character from the serial port */
NTSTATUS
HSO_read_character(
		   PDEVICE_OBJECT device_object,
		   PDEVICE_EXTENSION p_device_extension, 
		   PUCHAR value
		   )
{
  NTSTATUS            status;
  USHORT              actual;

  /*  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
      ("read_character entry"));*/

  status =
    HSO_read_serial_port(device_object, p_device_extension, value, 1, &actual);

  if (!NT_SUCCESS(status)) 
    {
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_ERROR, 
	       ("read_char failed; 0x%x", status ));
    }
  else if (actual != 1) 
    {
      status  = STATUS_UNSUCCESSFUL;
    }
  else 
    {
      /*      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	      ("read %x (actual = %d)", (ULONG)*value, actual));*/
    }

  /*  HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK,
      "read_char exit", status);*/

  return status;
}


#define DETECT_BUFFER_SIZE 2048
/* Reads the first 100 characters output by the spaceorb.  This usually
   will include our detection buffer/startup message */
NTSTATUS
HSO_detect_spaceorb(
		    PDEVICE_OBJECT device_object, /* our fdo */
		    PDEVICE_EXTENSION device_extension
		    )
{
  ULONG           count = 0;
  NTSTATUS        status;
  ULONG           i;
  UCHAR            receiveBuffer[DETECT_BUFFER_SIZE];

  PAGED_CODE();

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("detect enter"));

  status = HSO_set_read_timeouts(device_object, 200);


  HSO_write_character( device_object, '\0d' );
  /*  #if 1 */
  /*    for (i = 0; i < 3; i++) */
  /*      { */
  /*        status = HSO_write_char( device_object, device_extension, &cmd[i]); */
  /*        if (!NT_SUCCESS( status )) */
  /*  	{ */
  /*  	  return status; */
  /*  	} */
  /*      } */
  /*  #endif */

  if (NT_SUCCESS(HSO_read_character(device_object,
				    device_extension,
				    &receiveBuffer[count])))
    {

      // Clear high bit -Yuri
      // removed by vputz--some packets *may* expect this?
      /*      receiveBuffer[count] &= 0x7f; */
      count++;
      status = HSO_set_read_timeouts(device_object, 100);

      while (count < (DETECT_BUFFER_SIZE - 1)) 
	{ 
	  if (NT_SUCCESS(HSO_read_character(device_object,
					    device_extension,
					    &receiveBuffer[count]))) 
	    {
              // Clear high bit -Yuri
              receiveBuffer[count] &= 0x7f;
	      count++;
	    } 
	  else 
	    {
	      break;
	    }
	} 
    }

  //terminate buffer
  *(receiveBuffer + count) = 0;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("Parsing %d chars of orb output...", count));
  for (i = 0; i < count; i++) 
    {
      parse_orb_character( device_extension, receiveBuffer[ i ] );
    }

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ("end transmission"));
  /*      DbgPrint("%c (%c?) (d%d) (0x%x)\n", receiveBuffer[i], receiveBuffer[i] & 0x7f, receiveBuffer[i], receiveBuffer[i]);*/

  // analyze answer... (TBD)

  // Make sure that all subsequent reads are blocking and do not timeout
  if (count > 0) 
    {
      status = HSO_set_read_timeouts(device_object, 0);
      HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	       ("setting timeouts to 0"));
    }

  HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
		"detect_spaceorb exit", status );

  return status;
}


/*copy these defines from serial.h...?*/
#define SERIAL_8_DATA       ((UCHAR)0x08)
#define SERIAL_NONE_PARITY  ((UCHAR)0x00)
#define SERIAL_1_STOP       ((UCHAR)0x00)

// This must be 9600 -Yuri
#define HIDSPORB_BAUD_RATE 9600
/*  SERIAL_BAUD_9600 */
static SERIAL_LINE_CONTROL 
hidsporb_line_control =  { SERIAL_1_STOP, SERIAL_NONE_PARITY, SERIAL_8_DATA };
/*{ STOP_BIT_1, NO_PARITY, 8 };*/
/*
  This does all the initialization of the serial port, detection
  of the spaceorb, initialization of the serial port's communications
  parameters, etc.  In other words, very distinct from InitializePort. 
*/
NTSTATUS
HSO_initialize_serial_device(
			     PDEVICE_OBJECT device_object,
			     PDEVICE_EXTENSION device_extension
			     )
{
  NTSTATUS status = STATUS_SUCCESS;
  SERIAL_LINE_CONTROL test_line_control;
  ULONG test_baud;

  HSO_set_wait_mask_to_zero( device_object );
  /* initialize the hardware */
  status = HSO_initialize_port( device_object, device_extension );
  HSO_LOG_IF( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, (!NT_SUCCESS(status)),
	      ("Failed to initialize port: result = 0x%x", status ));

  HSO_get_baud_rate( device_object, &test_baud );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ( "old baud rate: %x", test_baud ));

  status = HSO_power_down( device_object, device_extension );
  HSO_LOG_IF( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, (!NT_SUCCESS(status)),
	      ("Failed to power down port: result = 0x%x", status ));


  HSO_set_baud_rate( device_object, HIDSPORB_BAUD_RATE );
  
  HSO_get_baud_rate( device_object, &test_baud );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ( "new baud rate: %x", test_baud ));

  HSO_get_line_control( device_object, &test_line_control );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, 
	   ("Old line control: stop: 0x%x, parity: 0x%x, data: 0x%x",
	    test_line_control.StopBits,
	    test_line_control.Parity,
	    test_line_control.WordLength ));

  HSO_set_line_control( device_object, &hidsporb_line_control );

  HSO_get_line_control( device_object, &test_line_control );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, 
	   ("New line control: stop: 0x%x, parity: 0x%x, data: 0x%x",
	    test_line_control.StopBits,
	    test_line_control.Parity,
	    test_line_control.WordLength ));


  HSO_flush_read_buffer( device_object );

  status = HSO_power_up( device_object, device_extension );
  HSO_get_baud_rate( device_object, &test_baud );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1,
	   ( "new baud rate after powerup: %x", test_baud ));
  HSO_get_line_control( device_object, &test_line_control );
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, 
	   ("New line control after powerup: stop: 0x%x, parity: 0x%x, data: 0x%x",
	    test_line_control.StopBits,
	    test_line_control.Parity,
	    test_line_control.WordLength ));

  HSO_LOG_IF( FILE_SERIAL_ROUTINES | HSO_MESSAGE1, (!NT_SUCCESS(status)),
	      ("Failed to power up port: result = 0x%x", status ));

  /* start detection */
  /*wait 1.5 seconds*/
  HSO_serial_wait( device_extension, -PAUSE_1_5_SECONDS );
  HSO_detect_spaceorb( device_object, device_extension );
  HSO_detect_spaceorb( device_object, device_extension );

  return status;
}




NTSTATUS
HSO_write_serial_port (
		       PDEVICE_OBJECT device_object, /* our FDO */
		       PCHAR               write_buffer,
		       ULONG               num_bytes,
		       PIO_STATUS_BLOCK    io_status_block
		       )
     //synchronous write to serial port 
     //returns STATUS_SUCCESS if the read was successful, error code otherwise
{
  NTSTATUS        status;
  PIRP            irp;
  LARGE_INTEGER   starting_offset = RtlConvertLongToLargeInteger(0);
  KEVENT          event;

  int             i, num_reads;

  KeInitializeEvent(&event,
		    NotificationEvent,
		    FALSE);
    
  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("write_serial_port entry"));

  if (NULL == (irp = IoBuildSynchronousFsdRequest(
						  IRP_MJ_WRITE,
						  GET_NEXT_DEVICE_OBJECT( device_object ),
						  write_buffer,        
						  num_bytes,   
						  &starting_offset,    
						  &event,
						  io_status_block   
						  ))) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT
		    ,"write_serial_port could not allocate IRP",
		    STATUS_INSUFFICIENT_RESOURCES );

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  status = IoCallDriver(GET_NEXT_DEVICE_OBJECT( device_object ), irp);
        
  if (status == STATUS_PENDING) 
    {
	
      // I don't know at this time if I can wait with the default time of
      // 200 ms as I'm doing.  In the help file for IoBuildSynchronousFsdRequest
      // I think that it says I can't, but I'm not quite sure.
      // Presently I will.  I'll cancel the Irp if it isn't done.
      status = KeWaitForSingleObject(
				     &event,
				     Executive,
				     KernelMode,
				     FALSE, // Not alertable
				     NULL);
    }
    
  status = io_status_block->Status;
                
  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT,
		    "write_serial_port failed", status );
    }

  else
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT,
		    "write_serial_port exit" , status );
    }

  return status;
}

/* writes a single character to the serial port */
NTSTATUS
HSO_write_character(
		    PDEVICE_OBJECT device_object, /* our fdo */
		    UCHAR value
		    )
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS        status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("write_char enter"));

  status = HSO_write_serial_port(
				 device_object,
				 &value,
				 1,
				 &iosb);

  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS(status)) 
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT,
		    "write_char failed", status);
    }

  else
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK,
		    "write_char exit", status );
    }

  return status;
}

/* writes zero-terminated string to serial port, one byte at a time */
NTSTATUS
HSO_write_string(
		 PDEVICE_OBJECT device_object, /* our fdo */
		 PSZ                 Buffer
		 )
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS        status;

  HSO_LOG( FILE_SERIAL_ROUTINES | HSO_FUNCTION_ENTRY,
	   ("write_string enter"));

  status = HSO_write_serial_port(
				 device_object,
				 Buffer,
				 strlen(Buffer),
				 &iosb);
  if (!NT_SUCCESS(iosb.Status)) 
    {
      status = iosb.Status;
    }

  if (!NT_SUCCESS( status ))
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT,
		    "write_string failed", status );
    }
  else
    {
      HSO_EXITPROC( FILE_SERIAL_ROUTINES | HSO_FUNCTION_EXIT_OK,
		    "write_string exit", status );
    }

  return status;
}



// Call HSO_start_read_thread() insead of spin_up_read
// Set is_thread_termination_requested Call KeWaitForSingleObject with is_thread_terminated 
// to wait for thread termination in unload/stop

// Start read thread to poll serial port
NTSTATUS
HSO_start_read_thread(PDEVICE_OBJECT devObj, PDEVICE_EXTENSION devExt)
{
  HANDLE   handle;
  NTSTATUS status;
  ULONG    i;

  // Create thread
  status = PsCreateSystemThread(&handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, HSO_read_thread, devObj);
  if (!NT_SUCCESS(status)) 
    {
      return status;
    }

  // We have to reference it
  status = ObReferenceObjectByHandle(handle, THREAD_ALL_ACCESS, NULL, KernelMode, &devExt->read_thread_object, NULL);

  // Don't forget to call ObDereferenceObject in 
  // unload/stop to avoid memory leak!!!
  if (!NT_SUCCESS(status)) 
    {
      // Tell the thread to terminate
      devExt->is_thread_termination_requested = TRUE;
      return status;
    }
  devExt->is_thread_started = TRUE;
  return status;
}

// This thread is started to read/update values from SpaceOrb
VOID
HSO_read_thread(PVOID Context)
{
  PDEVICE_OBJECT       devObj;
  PDEVICE_EXTENSION devExt;
  NTSTATUS ntStatus;

  devObj = (PDEVICE_OBJECT) Context;
  devExt = GET_MINIDRIVER_DEVICE_EXTENSION(devObj);
  // Should use InterlockedCompareExchange?
  while (devExt->is_thread_termination_requested == FALSE) 
    {
      // Read packet in liborb way (i.e. read the 
      // first byte, then the rest of packet) and parse it
      ntStatus = HSO_read_packet(devObj, devExt);
    }
  // Tell 'em we're dead
  KeSetEvent(&devExt->is_thread_terminated, 0, FALSE);
  devExt->is_thread_started = FALSE;
  PsTerminateSystemThread(0);
}


NTSTATUS
HSO_read_packet( PDEVICE_OBJECT device_object,
		 PDEVICE_EXTENSION device_extension )
{
  NTSTATUS status;
  UCHAR ch;

  status = HSO_read_character( device_object, 
			       device_extension,
			       &ch );
  if (NT_SUCCESS( status ))
    {
      parse_orb_character( device_extension, ch );
    }
  return status;
  
}

