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


#ifndef HIDSPORB_DEVICE_EXTENSION_H
#define HIDSPORB_DEVICE_EXTENSION_H

#include "wdm.h"
#include "ntddser.h"

#define ORB_PACKET_BUFFER_LENGTH 60

typedef enum 
{
  HSO_unknown_packet,
  HSO_reset_packet,
  HSO_ball_data_packet,
  HSO_button_data_packet,
  HSO_error_packet,
  HSO_null_region_packet,
  HSO_terminator_packet
} orb_packet_type;

#define NUM_PHYSICAL_AXES 6
#define NUM_PHYSICAL_BUTTONS 7

#define MAX_LOGICAL_AXES 6
#define MAX_LOGICAL_BUTTONS 16

/*
  structure used to store pending IRPs 
*/
typedef struct {
    LIST_ENTRY  list;
    union {
        PIRP    irp;
    };
} NODE, *PNODE;



/*

The hidsporb device extension, intended for use on a per-device basis

*/
typedef struct _DEVICE_EXTENSION
{
  /** some items included from the mouser driver for information **/
  IO_REMOVE_LOCK remove_lock;
  LONG enable_count;
  PIRP read_irp;
  SERIAL_BASIC_SETTINGS serial_port_basic_settings;
  KTIMER delay_timer;

  /* store a pointer to our driver object; this is used in settings.c to 
     retrieve the registry path we started the driver with */
  PDRIVER_OBJECT driver_object;

  /* list of other devices... but not currently used correctly, I think*/
  LIST_ENTRY                  link;

  /* Event used to make sure all other requests have been completed before
     this device is removed in the "remove" plug and play event handler */
  KEVENT                      remove_event;

  /* Number of IRPs currently under way */
  LONG                        request_count;

  /* Flag tracking whether or not device has been removed; if it has,
     all requests should fail */
  BOOLEAN                     is_removed;

  /* Flag tracking whether or not device has been started */
  BOOLEAN                     is_started;

  /* Flag tracking whether or not the device has been "surprise removed" */
  BOOLEAN                     is_surprise_removed;

  /* cache our device object */
  PDEVICE_OBJECT device_object;

  /* fields for parsing orb data */
  unsigned char packet_buffer[ ORB_PACKET_BUFFER_LENGTH ];
  int packet_buffer_cursor; /* NEXT byte that will be written,
			       and CURRENT length of the buffer */
  orb_packet_type current_packet_type;
  /* whether or not we've received a meaningful packet from the orb */
  int orb_has_spoken; 

  /* fields for storing physical orb data */
  int physical_axes [NUM_PHYSICAL_AXES];
  /* buttons: indexed by A-B-C-D-E-F-Reset (0 = A).  Any nonzero value
     means the button is pressed */
  UCHAR physical_buttons [NUM_PHYSICAL_BUTTONS];


  /* Fields for "read thread" management */
  BOOLEAN is_thread_termination_requested; // Request to stop thread
  BOOLEAN is_thread_started; // Is thread started???
  KEVENT  is_thread_terminated; // We wait for thread to terminate
  PVOID   read_thread_object;
  
  /* Fields for physical-to-logical control bindings */
  /* axis map--for each logical axis, an entry in this map contains the
     index of the physical axis to use */
  int axis_map[ MAX_LOGICAL_AXES ];
  /* whether or not to use chording on this device.  If chording
     is used, buttons A/B on the orb set up a context for the remaining
     four buttons */
  BOOLEAN use_chording;
  /* upcoming null region.  Note that the region is not actually
     set in the orb until it's processed during orb_comm, thus the
     two elements for "new_null_region_pending" and "null_region"
  */
  int null_region;
  /* sensitivities -- determines the "response curve" used by each axis */
  int sensitivities[ MAX_LOGICAL_AXES ];
  /* polarities -- determines "which direction is positive" on each axis */
  int polarities[ MAX_LOGICAL_AXES ];
  /* gains -- determines "amplification" of each axis */
  int gains[ MAX_LOGICAL_AXES ];

  /* precision settings--first sensitivity (response curve when 
     precision button is pressed) */
  int precision_sensitivity;
  /* gain to use when button is pressed */
  int precision_gain;
  /* what button type -- logical or physical -- for precision */
  int precision_button_type;
  /* what button index for precision */
  int precision_button_index;
  
  BOOLEAN new_null_region_pending;

  /* some bits used to queue read IRPs */
  LIST_ENTRY read_irp_head;
  KSPIN_LOCK read_irp_lock;
  LONG pending_request_count;
  
}  DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define HIDSPORB_POLARITY_NEGATIVE 0
#define HIDSPORB_POLARITY_ZERO 1
#define HIDSPORB_POLARITY_POSITIVE 2

#define HIDSPORB_BUTTON_TYPE_NONE 0
#define HIDSPORB_BUTTON_TYPE_PHYSICAL 1
#define HIDSPORB_BUTTON_TYPE_LOGICAL 2

#define GET_PDO_FROM_FDO(FDO) \
(((PHID_DEVICE_EXTENSION)(FDO)->DeviceExtension)->PhysicalDeviceObject)

#define GET_MINIDRIVER_DEVICE_EXTENSION(DO)  \
    ((PDEVICE_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_NEXT_DEVICE_OBJECT(DO) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

#endif


#define HSO_START_READ     0x01
#define HSO_END_READ       0x02
#define HSO_IMMEDIATE_READ 0x03
