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
#define IOSYNC_H

#include "wdm.h"

NTSTATUS
HSO_sync_ioctl_func(
		 BOOLEAN          is_internal,
		 ULONG            ioctl,
		 PDEVICE_OBJECT   p_device_object, 
		 PKEVENT          p_event,
		 PIO_STATUS_BLOCK p_iosb);

NTSTATUS
HSO_sync_ioctl_extra_func(
		   BOOLEAN          is_internal,
		   ULONG            ioctl,
		   PDEVICE_OBJECT   p_device_object,
		   PKEVENT          p_event,
		   PIO_STATUS_BLOCK p_iosb, 
		   PVOID            in_buffer,
		   ULONG            in_buffer_length,
		   PVOID            out_buffer,
		   ULONG            out_buffer_length );

#define HSO_sync_ioctl(ioctl, p_device_object, p_event, p_iosb)  \
        HSO_sync_ioctl_func(FALSE, ioctl, p_device_object, p_event, p_iosb)


#define HSO_sync_internal_ioctl(ioctl, p_device_object, p_event, p_iosb) \
        HSO_sync_ioctl_func(TRUE, ioctl, p_device_object, p_event, p_iosb)                                   
#define HSO_sync_ioctl_extra(ioctl, p_device_object, p_event, p_iosb, \
                             in_buffer, in_buffer_length, \
                             out_buffer, out_buffer_length) \
  HSO_sync_ioctl_extra_func(FALSE, ioctl, p_device_object, p_event, p_iosb,   \
                            in_buffer, in_buffer_length, \
                            out_buffer, out_buffer_length ) 
#define HSO_sync_internal_ioctl_extra(ioctl, p_device_object, p_event, p_iosb,  \
                                  in_buffer, in_buffer_length, \
                                  out_buffer, out_buffer_length) \
  HSO_sync_ioctl_extra_func(TRUE, ioctl, p_device_object, p_event, p_iosb,   \
                            in_buffer, in_buffer_length, \
                            out_buffer, out_buffer_length )


NTSTATUS
HSO_send_irp_synchronously (
    IN PDEVICE_OBJECT   p_device_object,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    );

NTSTATUS 
HSO_irp_complete
(
 PDEVICE_OBJECT   p_device_object,
 PIRP             Irp,
 PVOID            Context
 );


#endif
