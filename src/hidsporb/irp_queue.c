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

#include "irp_queue.h"

#ifndef DEBUG_H
#include "debug.h"
#endif
#ifndef HID_REPORTS_H
#include "hid_reports.h"
#endif

void
HSO_increment_pending_request_count( PDEVICE_EXTENSION device_extension )
{
  InterlockedIncrement( &( device_extension->pending_request_count ) );
/*    ++( device_extension->pending_request_count ); */
  HSO_LOG( FILE_IRP_QUEUE | HSO_MESSAGE1,
		("Incrementing pending request count to %d", 
		 device_extension->pending_request_count ) );
}

void
HSO_decrement_pending_request_count( PDEVICE_EXTENSION device_extension )
{
  InterlockedDecrement( &( device_extension->pending_request_count ) );
/*    --( device_extension->pending_request_count ); */
  HSO_LOG( FILE_IRP_QUEUE | HSO_MESSAGE1,
		("decrementing pending request count to %d",
		 device_extension->pending_request_count ) );
}


/*
  Initialize the "read irp" queue
*/
void
HSO_initialize_read_irp_queue( PDEVICE_EXTENSION device_extension )
{
  KeInitializeSpinLock( &(device_extension->read_irp_lock ) );
  InitializeListHead( &( device_extension->read_irp_head ) );
  device_extension->pending_request_count = 0;
}

/*
  queue a read IRP for later and mark it as "pending"; we'll fill
  in when we have data to send
*/
NTSTATUS HSO_queue_read_report( PDEVICE_EXTENSION device_extension, PIRP irp )
{
  NTSTATUS ntStatus;
  PIO_STACK_LOCATION irp_stack;
  PVOID report_buffer;
  ULONG total_report_size;
  PNODE node;

  HSO_LOG( FILE_IRP_QUEUE | HSO_FUNCTION_ENTRY,
		("enter queue_read_report"));
  irp_stack = IoGetCurrentIrpStackLocation(irp);

  //  Get the buffer and its size, make sure they're valid.
  report_buffer = irp->UserBuffer;
  total_report_size = irp_stack->Parameters.DeviceIoControl.OutputBufferLength;

  HSO_LOG( FILE_IRP_QUEUE | HSO_MESSAGE2,
		( "Report buffer = 0x%x, total size = 0x%x",
		  report_buffer, total_report_size ) );

  if (device_extension->is_started)
    {
      if (total_report_size && report_buffer)
        {
	  node = (PNODE)ExAllocatePool(NonPagedPool, sizeof(NODE));
	  if (node)
            {

	      //  Increase the count of outstanding IOs, mark the Irp pending.
	      HSO_increment_pending_request_count( device_extension );

	      //  Hook the Irp onto the pending IO list
	      node->irp = irp;
	      IoMarkIrpPending( irp );
	      ExInterlockedInsertTailList(&( device_extension->read_irp_head ), &node->list, &( device_extension->read_irp_lock ));
	      ntStatus = STATUS_PENDING;
            }
	  else
            {
	      ntStatus = STATUS_NO_MEMORY;
            }
        }
      else
        {
	  // No buffer, or buffer of zero size
	  ntStatus = STATUS_INVALID_PARAMETER;
        }
    }
  else
    {
      //  We're shutting down
      ntStatus = STATUS_NO_SUCH_DEVICE;
    }

  HSO_EXITPROC( FILE_IRP_QUEUE | HSO_FUNCTION_EXIT, 
	     "exiting queue_read_report", ntStatus );

  return ntStatus;
}

void
HSO_flush_read_report_queue( PDEVICE_EXTENSION device_extension )
{
  PNODE node;
  //  Dispose of any Irps, free up all the NODEs waiting in the queues.

  while ((node = (PNODE)ExInterlockedRemoveHeadList(&(device_extension->read_irp_head), &( device_extension->read_irp_lock ) ) ) )
    {
      node->irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
      IoCompleteRequest(node->irp, IO_NO_INCREMENT);
      ExFreePool(node);
    }

}


NTSTATUS
HSO_complete_read_irp( PDEVICE_EXTENSION device_extension,
		       PIRP irp )
{
  PIO_STACK_LOCATION irp_stack;
  NTSTATUS status = STATUS_SUCCESS;

  irp_stack = IoGetCurrentIrpStackLocation( irp );
  HSO_decrement_pending_request_count( device_extension );
      
  /* now make sure this IRP is appropriate */
  HSO_LOG( FILE_IOCTL | HSO_MESSAGE1,
		("Size of read IRP buffer: out - %d, report length: %d", 
		 irp_stack->Parameters.DeviceIoControl.OutputBufferLength,
		 sizeof( HIDSPORB_INPUT_DATA )));
  if ( irp_stack->Parameters.DeviceIoControl.OutputBufferLength < 
       sizeof( HIDSPORB_INPUT_DATA ) )
    {
      HSO_LOG( FILE_IOCTL | HSO_WARNING,
		    ( "complete_queue_head: buffer to small; output= 0x%x; need 0x%x",
		      irp_stack->Parameters.DeviceIoControl.OutputBufferLength,
		      sizeof( HIDSPORB_INPUT_DATA ) ) );
      status = STATUS_BUFFER_TOO_SMALL;
    }
  if ( device_extension->is_started == FALSE )
    {
      status = STATUS_DEVICE_NOT_READY;
    }

  /* if everything's good, translate device-specific data to the HID
	 report */
  if (NT_SUCCESS( status ))
    {
      HSO_generate_hid_data( device_extension, 
			     (PHIDSPORB_INPUT_DATA)(irp->UserBuffer));
      irp->IoStatus.Information = sizeof( HIDSPORB_INPUT_DATA );
    }
  else
    {
      irp->IoStatus.Information = 0x0;
    }
  irp->IoStatus.Status = status;
  HSO_LOG( FILE_IRP_QUEUE | HSO_MESSAGE1,
		( "completing read request" ));
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return status;
}
/* 
   completes the first IRP in the read queue
*/
NTSTATUS
HSO_complete_read_queue_head( PDEVICE_EXTENSION device_extension )
{
  PNODE irp_node;
  NTSTATUS status = STATUS_SUCCESS;

  HSO_LOG( FILE_IRP_QUEUE | HSO_FUNCTION_ENTRY,
		( "Entering complete_queue_head" ) );

  irp_node = (PNODE) ExInterlockedRemoveHeadList( &(device_extension->read_irp_head), 
						  &(device_extension->read_irp_lock) );
  if ( irp_node )
    {
      status = HSO_complete_read_irp( device_extension, irp_node->irp );
      ExFreePool( irp_node );
    }

  
  HSO_EXITPROC( FILE_IRP_QUEUE | HSO_FUNCTION_EXIT, "complete_queue_head", status );

  return status;
}

