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
#ifndef HIDSPORB_H
#define HIDSPORB_H

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif
#include "hidtoken.h"
#include "hidusage.h"
#include "hidport.h"
#ifndef DEBUG_H
#include "debug.h"
#endif





/* global storage area for hidsporb driver */
  typedef struct _HIDSPORB_GLOBAL
  {
    /* A syncronization for access to list */
    FAST_MUTEX          mutex;
    /* Keeps list of all the devices */
    LIST_ENTRY          device_list_head;
    /* Lock so that only one port is accessed */
    KSPIN_LOCK          spin_lock;     

    /* settings strings for registry data */
    UNICODE_STRING registry_base;
    UNICODE_STRING settings_path;
  } HIDSPORB_GLOBAL;

#ifndef HIDSPORB_DEVICE_EXTENSION_H
#include "hidsporb_device_extension.h"
#endif


extern HIDSPORB_GLOBAL Global;

NTSTATUS
    DriverEntry
    (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    );

NTSTATUS
    HSO_create
    (
    IN PDEVICE_OBJECT p_device_object,
    IN PIRP Irp
    );

NTSTATUS
    HSO_close
    (
    IN PDEVICE_OBJECT p_device_object,
    IN PIRP Irp
    );

NTSTATUS
    HSO_add_device
    (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT p_functional_device_object
    );

VOID
    HSO_unload
    (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
HSO_flush_buffers
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );


NTSTATUS
HSO_start_device(
		PDEVICE_OBJECT device_object,
		PDEVICE_EXTENSION device_extension,
		PIRP Irp,
		BOOLEAN should_close_on_failure
		);


#endif 

