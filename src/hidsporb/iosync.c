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
#ifndef IOSYNC_H
#include "iosync.h"
#endif

#ifndef DEBUG_H
#include "debug.h"
#endif

  NTSTATUS
    HSO_sync_ioctl_func(
		     BOOLEAN          Internal,
		     ULONG            Ioctl,
		     PDEVICE_OBJECT   p_device_object, 
		     PKEVENT          Event,
		     PIO_STATUS_BLOCK Iosb)
{
  return HSO_sync_ioctl_extra_func(Internal,
			    Ioctl,
			    p_device_object, 
			    Event,
			    Iosb,
			    NULL,
			    0,
			    NULL,
			    0);
}                 

NTSTATUS
HSO_sync_ioctl_extra_func(
		   BOOLEAN          Internal,
		   ULONG            Ioctl,                     // io control code
		   PDEVICE_OBJECT   p_device_object,              // object to call
		   PKEVENT          Event,                     // event to wait on
		   PIO_STATUS_BLOCK Iosb,                      // used inside IRP
		   PVOID            InBuffer,      OPTIONAL    // input buffer n
		   ULONG            InBufferLen,   OPTIONAL    // input buffer length
		   PVOID            OutBuffer,     OPTIONAL    // output buffer 
		   ULONG            OutBufferLen)  OPTIONAL    // output buffer length
{
  PIRP                irp;
  NTSTATUS            status;
  
  KeClearEvent(Event);
    
  //
  // Allocate an IRP - No need to release
  // When the next-lower driver completes this IRP, the I/O Manager releases it.
  //
  if (NULL == (irp = IoBuildDeviceIoControlRequest(Ioctl,
						   p_device_object,
						   InBuffer,  
						   InBufferLen,
						   OutBuffer,  
						   OutBufferLen,
						   Internal,
						   Event,
						   Iosb))) {
    
    return STATUS_INSUFFICIENT_RESOURCES;
  }
    
  status = IoCallDriver(p_device_object, irp);
 
  if (STATUS_PENDING == status) {
    //
    // wait for it...
    //
    status = KeWaitForSingleObject(Event,
				   Executive,
				   KernelMode,
				   FALSE, // Not alertable
				   NULL); // No timeout structure
  }
  
  if (NT_SUCCESS(status)) {
    status = Iosb->Status;
  }
  
  return status;
}                 

NTSTATUS
HSO_send_irp_synchronously (
			  IN PDEVICE_OBJECT   p_device_object,
			  IN PIRP             Irp,
			  IN BOOLEAN          CopyToNext
			  )
{
  KEVENT      event;
  NTSTATUS    status;

  PAGED_CODE();

  KeInitializeEvent(&event, SynchronizationEvent, FALSE);

  if (CopyToNext) {
    IoCopyCurrentIrpStackLocationToNext(Irp);
  }

  IoSetCompletionRoutine(Irp,
			 HSO_irp_complete,
			 &event,
			 TRUE,                // on success
			 TRUE,                // on error
			 TRUE                 // on cancel
			 );

  status = IoCallDriver(p_device_object, Irp);

  //
  // Wait for lower drivers to be done with the Irp
  //
  if (status == STATUS_PENDING) {
    KeWaitForSingleObject(&event,
			  Executive,
			  KernelMode,
			  FALSE,
			  NULL
			  );
    status = Irp->IoStatus.Status;
  }

  return status;
}



NTSTATUS 
HSO_irp_complete
(
 IN PDEVICE_OBJECT   p_device_object,
 IN PIRP             Irp,
 IN PVOID            Context
 )
{
  NTSTATUS ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

  PAGED_CODE();

  HSO_LOG(FILE_PNP | HSO_FUNCTION_ENTRY,\
	       ("HSO_irp_complete(p_device_object=0x%x,Irp=0x%x,Context=0x%x)", \
		p_device_object, Irp, Context));

  UNREFERENCED_PARAMETER (p_device_object);
  KeSetEvent ((PKEVENT) Context, 0, FALSE);

  if(Irp->PendingReturned)
    {
      IoMarkIrpPending(Irp);
    }

  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT, "HSO_PnpComplete", ntStatus);

  return ntStatus;
}
