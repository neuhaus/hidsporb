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

#include "initguid.h"
#include "hidport.h"
#ifndef PNP_H
#include "pnp.h"
#endif
#ifndef IOCTL_H
#include "ioctl.h"
#endif
#ifndef SETTINGS_H
#include "settings.h"
#endif
#ifndef HID_REPORTS_H
#include "hid_reports.h"
#endif
#ifndef IRP_QUEUE_H
#include "irp_queue.h"
#endif
#ifndef FEATURES_H
#include "features.h"
#endif
#ifndef VALIDITY_H
#include "validity.h"
#endif
#ifndef CHARTS_H
#include "charts.h"
#endif

#ifdef ALLOC_PRAGMA
  /*  #pragma alloc_text (PAGE, HSO_get_device_descriptor) */
  /*  #pragma alloc_text (PAGE, HSO_get_report_descriptor) */
  /*  #pragma alloc_text (PAGE, HSO_get_attributes      ) */
#endif



  NTSTATUS 
    HSO_get_feature( PDEVICE_OBJECT device_object,
		     PIRP irp );
NTSTATUS 
HSO_set_feature( PDEVICE_OBJECT device_object,
		 PIRP irp );


PCHAR manufacturer_string = "Winged Yak Productions\0";
/*  UCHAR manufacterer_string[] = { */
/*    'W',0, 'i',0, 'n',0, 'g',0, 'e',0, 'd',0, ' ',0, 'Y',0, */
/*    'a',0, 'k',0, ' ',0, 'P',0, 'r',0, 'd',0, 'u',0, 'c',0, */
/*    't',0, 'i',0, 'o',0, 'n',0, 's',0 */
/*  }; */

PCHAR product_string = "SpaceOrb 360\0";

/*  UCHAR product_string[] = { */
/*    'S',0, 'p',0, 'a',0, 'c',0, 'e',0, 'o',0, 'r',0, 'b',0, */
/*    ' ',0, '3',0, '6',0, '0',0 */
/*  }; */


PCHAR serial_number_string = "42\0";
/*  UCHAR serial_number_string[] = { */
/*    '4',0, '2',0  */
/*  }; */

PCHAR unsupported_string = "UNSUPPORTED STRING\0";
/*  UCHAR unsupported_string[] = { */
/*    'U',0, 'N',0, 'S',0, 'U',0, 'P',0, 'P',0, 'O',0, 'R',0, */
/*    'T',0, 'E',0, 'D',0, ' ',0, 'S',0, 'T',0, 'R',0, 'I',0, */
/*    'N',0, 'G',0 }; */

/*  UCHAR Raw_string_descriptor[] = { */
/*    4, //length of this string? */
/*    3, // type == STRING */
/*    0x09, 0x00, //language code = ENGLISH */
  
/*    44, 3, */
/*    'W',0, 'i',0, 'n',0, 'g',0, 'e',0, 'd',0, ' ',0, 'Y',0, */
/*    'a',0, 'k',0, ' ',0, 'P',0, 'r',0, 'd',0, 'u',0, 'c',0, */
/*    't',0, 'i',0, 'o',0, 'n',0, 's',0, */

/*    26, 3, */
/*    'S',0, 'p',0, 'a',0, 'c',0, 'e',0, 'o',0, 'r',0, 'b',0, */
/*    ' ',0, '3',0, '6',0, '0',0, */

/*    26, 3, */
/*    'S',0, 'p',0, 'a',0, 'c',0, 'e',0, 'o',0, 'r',0, 'b',0, */
/*    ' ',0, '3',0, '6',0, '0',0 */
/*  }; */

NTSTATUS
HSO_get_string_descriptor( PDEVICE_OBJECT device_object, 
			   PIRP irp )
{
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION device_extension;
  PIO_STACK_LOCATION irp_stack;
  ULONG bytes_to_copy;
  ULONG string_requested;
  PCHAR pstring_requested;

  HSO_LOG( FILE_IOCTL | HSO_FUNCTION_ENTRY,
	   ("entering get_string_descriptor"));

  device_extension = GET_MINIDRIVER_DEVICE_EXTENSION(  device_object );
  irp_stack = IoGetCurrentIrpStackLocation( irp );
  bytes_to_copy = irp_stack->Parameters.DeviceIoControl.OutputBufferLength;

  //figure out which string we have
  string_requested = (ULONG)(irp_stack->Parameters.DeviceIoControl.Type3InputBuffer ) & 0xffff;
  HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
	   ("requested string 0x%x, userbuffer = 0x%x, bytes_to_copy = 0x%x", string_requested, irp->UserBuffer, bytes_to_copy ));
  switch (string_requested)
    {
    case HID_STRING_ID_IMANUFACTURER :
      pstring_requested = manufacturer_string;
      break;
    case HID_STRING_ID_IPRODUCT :
      pstring_requested = product_string;
      break;
    case HID_STRING_ID_ISERIALNUMBER :
      pstring_requested = serial_number_string;
      break;
    default:
      pstring_requested = unsupported_string;
    }

  //make sure we have a buffer with space
  if ( (irp->UserBuffer != NULL ) && (bytes_to_copy > 0 ) )
    {
      //normalize bytes to copy with our string descriptor
      if ( bytes_to_copy > (strlen( pstring_requested ) * sizeof( CHAR ) + 1))
	{
	  bytes_to_copy = (strlen( pstring_requested ) * sizeof( CHAR ) + 1);
	}
      
      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
	       ("copying %d bytes to string buffer", bytes_to_copy));
      RtlCopyMemory( (PUCHAR)(irp->UserBuffer), 
		     (PUCHAR)(pstring_requested),
		     bytes_to_copy );
      irp->IoStatus.Information = bytes_to_copy;
    }
  else
    {
      status = STATUS_INVALID_USER_BUFFER;
    }
  irp->IoStatus.Status = status;
  
  HSO_EXITPROC( FILE_IOCTL | HSO_FUNCTION_EXIT, 
		"exiting get_string_descriptor",
		status );
  return status;
}

/* Dispatches internal IO control IRPs which have been sent to this device;
   this function cannot be pageable (because reads/writes can be made at 
   dispatch level? */
NTSTATUS 
HSO_internal_ioctl
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  NTSTATUS            nt_status = STATUS_SUCCESS;
  PDEVICE_EXTENSION   p_device_extension;
  PIO_STACK_LOCATION  p_irp_stack;

  HSO_LOG(FILE_IOCTL | HSO_FUNCTION_ENTRY,   \
	  ("HSO_internal_ioctl(p_device_object=0x%x,p_irp=0x%x)", \
	   p_device_object, p_irp));

  p_irp_stack = IoGetCurrentIrpStackLocation(p_irp);

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  nt_status = HSO_increment_request_count( p_device_extension );
  if (!NT_SUCCESS (nt_status))
    {
      HSO_LOG(FILE_PNP | HSO_ERROR,\
	      ("HSO_internal_ioctl: PnP IRP after device was removed\n"));
      p_irp->IoStatus.Information = 0;
      p_irp->IoStatus.Status = nt_status;
    } 
  else
    {
      switch(p_irp_stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
	  HSO_LOG(FILE_IOCTL | HSO_MESSAGE2, \
		  ("IOCTL_HID_GET_DEVICE_DESCRIPTOR"));
	  nt_status = HSO_get_device_descriptor(p_device_object, p_irp);
	  break;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
	  HSO_LOG(FILE_IOCTL | HSO_MESSAGE2, \
		  ("IOCTL_HID_GET_REPORT_DESCRIPTOR"));
	  nt_status = HSO_get_report_descriptor(p_device_object, p_irp);
	  break;

	case IOCTL_HID_READ_REPORT:
	  HSO_LOG(FILE_IOCTL | HSO_MESSAGE2,\
		  ("IOCTL_HID_READ_REPORT"));
	  nt_status = hso_read_report(p_device_object, p_irp);
	  break;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
	  HSO_LOG(FILE_IOCTL | HSO_MESSAGE2,\
		  ("IOCTL_HID_GET_DEVICE_ATTRIBUTES"));
	  nt_status = HSO_get_attributes(p_device_object, p_irp);
	  break;

	case IOCTL_HID_GET_FEATURE:
	  HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		   ("IOCTL_HID_GET_FEATURE"));
	  nt_status = HSO_get_feature( p_device_object, p_irp );
	  break;

	case IOCTL_HID_SET_FEATURE:
	  HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		   ("IOCTL_HID_SET_FEATURE"));
	  nt_status = HSO_set_feature( p_device_object, p_irp );
	  break;
	    
	case IOCTL_HID_GET_STRING :
	  HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		   ( "IOCTL_HID_GET_STRING" ));
	  nt_status = HSO_get_string_descriptor( p_device_object, p_irp );
	  break;

	default:
	  switch ( p_irp_stack->Parameters.DeviceIoControl.IoControlCode )
	    {
	    case IOCTL_GET_PHYSICAL_DESCRIPTOR :
	      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		       ( "IOCTL_GET_PHYSICAL_DESCRIPTOR" ));
	      break;
	    case IOCTL_HID_ACTIVATE_DEVICE :
	      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		       ( "IOCTL_HID_ACTIVATE_DEVICE" ));
	      break;
	    case IOCTL_HID_DEACTIVATE_DEVICE :
	      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		       ( "IOCTL_HID_DEACTIVATE_DEVICE" ));
	      break;
	    case IOCTL_HID_GET_INDEXED_STRING :
	      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2, 
		       ( "IOCTL_HID_GET_INDEXED_STRING" ));
	      break;
	      /*  	      case IOCTL_HID_GET_INPUT_REPORT : */
	      /*  		HSO_LOG( FILE_IOCTL | HSO_MESSAGE2, */
	      /*  			      ( "IOCTL_HID_GET_INPUT_REPORT" ) ); */
	      /*  		break; */
	      /*  	      case IOCTL_HID_SET_OUTPUT_REPORT : */
	      /*  		HSO_LOG( FILE_IOCTL | HSO_MESSAGE2, */
	      /*  			      ( "IOCTL_HID_SET_OUTPUT_REPORT" ) ); */
	      /*  		break; */
	    case IOCTL_HID_WRITE_REPORT :
	      HSO_LOG( FILE_IOCTL | HSO_MESSAGE2,
		       ( "IOCTL_HID_WRITE_REPORT" ));
	      break;

	    default :
	      HSO_LOG(FILE_IOCTL | HSO_WARNING,\
		      ("Unknown or unsupported IOCTL (%x)",
		       p_irp_stack->Parameters.DeviceIoControl.IoControlCode));
	    }
	  nt_status = STATUS_NOT_SUPPORTED;
	  break;
	}
      
      //return status must be set in the IRP
      p_irp->IoStatus.Status = nt_status;

      HSO_decrement_request_count( p_device_extension );
    }


  if(nt_status != STATUS_PENDING)
    {
      IoCompleteRequest(p_irp, IO_NO_INCREMENT);
      nt_status = STATUS_SUCCESS;
    } 
  else
    {
      //status should never be "pending"
      HSO_LOG(FILE_IOCTL | HSO_MESSAGE1, \
	      ("HSO_internal_ioctl: Pending Status !"));
      IoMarkIrpPending( p_irp );
    }

  HSO_EXITPROC(FILE_IOCTL | HSO_FUNCTION_EXIT_OK, 
	       "HSO_internal_ioctl", nt_status);

  return nt_status;
} 

NTSTATUS 
HSO_set_feature( PDEVICE_OBJECT device_object,
		 PIRP irp )
{
  NTSTATUS status = STATUS_SUCCESS;
  PIO_STACK_LOCATION irp_stack;
  PHID_XFER_PACKET packet = ((PHID_XFER_PACKET)(irp->UserBuffer));
  PDEVICE_EXTENSION device_extension = 
    GET_MINIDRIVER_DEVICE_EXTENSION( device_object );
  PHIDSPORB_FEATURE_DATA feature_data = 
    &(((PHIDSPORB_FEATURE_PACKET)(packet->reportBuffer))->feature_data );
  PHIDSPORB_SENSITIVITY_CURVE curve_data = 
    &(((PHIDSPORB_SENSITIVITY_CURVE_PACKET)( packet->reportBuffer ))->curve);
  int i;

  HSO_LOG( FILE_IOCTL | HSO_FUNCTION_ENTRY | HSO_MESSAGE2,
	   ("HSO_set_feature_buffer size = %d; contents: %d/%d/%d/%d/%d/%d/%d/%d", 
	    packet->reportBufferLen,
	    feature_data->axis_map[ 0 ],
	    feature_data->axis_map[ 1 ],
	    feature_data->axis_map[ 2 ],
	    feature_data->axis_map[ 3 ],
	    feature_data->axis_map[ 4 ],
	    feature_data->axis_map[ 5 ],
	    feature_data->use_chording,
	    feature_data->null_region
	    ));
  
  irp_stack = IoGetCurrentIrpStackLocation( irp );
  
  if (packet->reportId == HIDSPORB_FEATURE_PACKET_ID )
    {
      if ( packet->reportBufferLen < HIDSPORB_FEATURE_PACKET_SIZE )
	{
	  irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
	  irp->IoStatus.Information = 0;
	  status = STATUS_BUFFER_TOO_SMALL;
	  HSO_LOG( FILE_IOCTL | HSO_ERROR,
		   ( "set_feature : buffer too small" ));
	}
      else
	{
	  for ( i = 0; i < NUM_PHYSICAL_AXES; ++i )
	    {
	      HSO_set_axis_mapping( device_extension, i, 
				    (LONG)(feature_data->axis_map[ i ] ));
	      HSO_set_sensitivity( device_extension, i, 
				   (LONG)(feature_data->sensitivities[ i ] ));
	      HSO_set_polarity( device_extension, i, 
				(LONG)(feature_data->polarities[ i ]));
	      HSO_set_gain( device_extension, i, 
			    (LONG)(feature_data->gains[ i ] ));
	    }

	  HSO_set_chording( device_extension, 
			    (BOOLEAN)(feature_data->use_chording != 0 ));
	  HSO_set_null_region( device_object,  
			       (int)(feature_data->null_region));
	  HSO_set_precision_sensitivity( device_extension, 
					 (int)(feature_data->precision_sensitivity ));
	  HSO_set_precision_gain( device_extension, 
				  (int)(feature_data->precision_gain));
	  HSO_set_precision_button( device_extension, 
				    (int)(feature_data->precision_button_type ),
				    (int)(feature_data->precision_button_index ) );
	  irp->IoStatus.Status = STATUS_SUCCESS;
	  irp->IoStatus.Information = HIDSPORB_FEATURE_PACKET_SIZE;
	  status = STATUS_SUCCESS;
	}
    }
  else if ( packet->reportId == HIDSPORB_CURVE_PACKET_ID )
    {
      if ( packet->reportBufferLen < HIDSPORB_SENSITIVITY_CURVE_SIZE )
	{
	  irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
	  irp->IoStatus.Information = 0;
	  status = STATUS_BUFFER_TOO_SMALL;
	  HSO_LOG( FILE_IOCTL | HSO_ERROR,
		   ("set_feature : buffer too small"));
	}
      else
	{
	  if ( is_valid_curve_id( curve_data->curve_id ) &&
	       is_valid_curve( curve_data->curve ) )
	    {
	      RtlMoveMemory( charts[ curve_data->curve_id ],
			     curve_data->curve,
			     1024 * sizeof( unsigned short ) );
	      irp->IoStatus.Status = STATUS_SUCCESS;
	      irp->IoStatus.Information = HIDSPORB_SENSITIVITY_CURVE_SIZE;
	      status = STATUS_SUCCESS;
	    }
	  else
	    {
	      irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	      irp->IoStatus.Information = 0;
	      status = STATUS_INVALID_PARAMETER;
	    }
	}
    }
  else
    {
      irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
      irp->IoStatus.Information = 0;
      status = STATUS_INVALID_PARAMETER;
    }
	      
  return status;
}

/*
  this spits out the internal feature report.  Currently this consists 
  of six bytes of axis map and one byte of "chording on"
*/
NTSTATUS 
HSO_get_feature( PDEVICE_OBJECT device_object,
		 PIRP irp )
{
  NTSTATUS status = STATUS_SUCCESS;
  PIO_STACK_LOCATION irp_stack;
  PHID_XFER_PACKET packet = ((PHID_XFER_PACKET)(irp->UserBuffer));
  PDEVICE_EXTENSION device_extension = 
    GET_MINIDRIVER_DEVICE_EXTENSION( device_object );
  PHIDSPORB_FEATURE_DATA feature_data = 
    &(((PHIDSPORB_FEATURE_PACKET)(packet->reportBuffer))->feature_data);
  PHIDSPORB_SENSITIVITY_CURVE curve_data = 
    &(((PHIDSPORB_SENSITIVITY_CURVE_PACKET)( packet->reportBuffer ))->curve);

  int i;

  HSO_LOG( FILE_IOCTL | HSO_FUNCTION_ENTRY | HSO_MESSAGE1,
	   ("HSO_get_feature buffer size = %d", packet->reportBufferLen ));
  
  //get a pointer to the current stack location
  irp_stack = IoGetCurrentIrpStackLocation( irp );

  if ( packet->reportId == HIDSPORB_FEATURE_PACKET_ID )
    {
      if ( packet->reportBufferLen < HIDSPORB_FEATURE_PACKET_SIZE ) 
	{
	  irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
	  irp->IoStatus.Information = 0;
	  status = STATUS_BUFFER_TOO_SMALL;
	  HSO_LOG( FILE_IOCTL | HSO_ERROR,
		   ("get_feature : Buffer too small" ));
	}
      else
	{
	  for ( i = 0; i < NUM_PHYSICAL_AXES; ++i )
	    {
	      feature_data->axis_map[ i ] = (char)(device_extension->axis_map[ i ]);
	      feature_data->sensitivities[ i ] = (char)(device_extension->sensitivities[ i ]);
	      feature_data->polarities[ i ] = (char)( device_extension->polarities[ i ]);
	      feature_data->gains[ i ] = (char)(device_extension->gains[ i ]);
	    }
	  feature_data->use_chording = (char)(device_extension->use_chording );
	  feature_data->null_region = (char)(device_extension->null_region );
	  feature_data->precision_sensitivity = (char)(device_extension->precision_sensitivity );
	  feature_data->precision_gain = (char)(device_extension->precision_gain);
	  feature_data->precision_button_type = (char)(device_extension->precision_button_type);
	  feature_data->precision_button_index = (char)(device_extension->precision_button_index );
      
	  irp->IoStatus.Status = STATUS_SUCCESS;
	  irp->IoStatus.Information = HIDSPORB_FEATURE_PACKET_SIZE;
	  status = STATUS_SUCCESS;
	}
    }
  else if ( packet->reportId == HIDSPORB_CURVE_PACKET_ID )
    {
      if ( packet->reportBufferLen < HIDSPORB_SENSITIVITY_CURVE_SIZE )
	{
	  irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
	  irp->IoStatus.Information = 0;
	  status = STATUS_BUFFER_TOO_SMALL;
	  HSO_LOG( FILE_IOCTL | HSO_ERROR,
		   ("get_feature : buffer too small" ));
	}
      else
	{
	  if ( is_valid_curve_id( curve_data->curve_id ) )
	    {
	      RtlMoveMemory( curve_data->curve,
			     charts[ curve_data->curve_id ],
			     1024 * sizeof( unsigned short ) );
	      irp->IoStatus.Status = STATUS_SUCCESS;
	      irp->IoStatus.Information = HIDSPORB_SENSITIVITY_CURVE_SIZE;
	      status = STATUS_SUCCESS;
	    }
	  else
	    {
	      irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	      irp->IoStatus.Information = 0;
	      status = STATUS_INVALID_PARAMETER;
	    }
	}
    }
  else
    {
      irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
      irp->IoStatus.Information = 0;
      status = STATUS_INVALID_PARAMETER;
    }

  return status;
}



/* Responds to the mighty IOCTL_HID_GET_DEVICE_DESCRIPTOR irp by
   creating and returning a device descriptor */
NTSTATUS 
HSO_get_device_descriptor
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  PHID_DESCRIPTOR pHidDescriptor;        /* Hid descriptor for this device */
  USHORT   cbReport;
  UCHAR               rgGameReport[MAXBYTES_GAME_REPORT] ;
  NTSTATUS            nt_status = STATUS_SUCCESS;
  PDEVICE_EXTENSION   p_device_extension;
  PIO_STACK_LOCATION  p_irp_stack;

  PAGED_CODE ();

  HSO_LOG(FILE_IOCTL | HSO_FUNCTION_ENTRY,\
	  ("HSO_get_device_descriptor(p_device_object=0x%x,p_irp=0x%x)",
	   p_device_object, p_irp));

  p_irp_stack = IoGetCurrentIrpStackLocation(p_irp);

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  pHidDescriptor =  (PHID_DESCRIPTOR) p_irp->UserBuffer;


  if( p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*pHidDescriptor)  )
    {

      HSO_LOG(FILE_IOCTL | HSO_ERROR,\
	      ("HSO_get_device_descriptor: OutBufferLength(0x%x) < sizeof(HID_DESCRIPTOR)(0x%x)", \
	       p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength, sizeof(*pHidDescriptor)));


      nt_status = STATUS_BUFFER_TOO_SMALL;
    } 
  else
    {
      /*
       * Generate the report
       */
      nt_status =  HSO_generate_report(p_device_object, rgGameReport, &cbReport);
      
      if( NT_SUCCESS(nt_status) )
	{
	  RtlZeroMemory( pHidDescriptor, sizeof(*pHidDescriptor) );
	  /*
	   * Copy device descriptor to HIDCLASS buffer
	   */
	  pHidDescriptor->bLength                         = sizeof(*pHidDescriptor);
	  pHidDescriptor->bDescriptorType                 = HID_HID_DESCRIPTOR_TYPE;
	  pHidDescriptor->bcdHID                          = HID_REVISION;
	  pHidDescriptor->bCountry                        = 0; /*not localized*/
	  pHidDescriptor->bNumDescriptors                 = HSO_NUMBER_DESCRIPTORS;
	  pHidDescriptor->DescriptorList[0].bReportType   = HID_REPORT_DESCRIPTOR_TYPE ;
	  pHidDescriptor->DescriptorList[0].wReportLength = cbReport;
	    
	  //following two feature reports added to attempt to control
	  //orb via hid feature reports

	  p_irp->IoStatus.Information = sizeof(*pHidDescriptor);
	} 
      else
	{
	  p_irp->IoStatus.Information = 0x0;
	}
    }

  HSO_EXITPROC(FILE_IOCTL |HSO_FUNCTION_EXIT_OK, "HSO_get_device_descriptor", nt_status);

  return nt_status;
} /* HSO_get_device_descriptor */


/*
  Responds to the IOCTL_HID_GET_REPORT_DESCRIPTOR irp by creating and
  returning a report descriptor (report descriptor is static for this
  device, not dynamically created--see hidsporb_report_descriptor.h
*/
NTSTATUS 
HSO_get_report_descriptor
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  PDEVICE_EXTENSION     p_device_extension;
  PIO_STACK_LOCATION    p_irp_stack;
  NTSTATUS              nt_status;
  UCHAR                 rgGameReport[MAXBYTES_GAME_REPORT] ;
  USHORT                cbReport;

  PAGED_CODE ();

  HSO_LOG(FILE_IOCTL | HSO_FUNCTION_ENTRY,\
	  ("HSO_get_report_descriptor(p_device_object=0x%x,p_irp=0x%x)",\
	   p_device_object, p_irp));

  p_irp_stack = IoGetCurrentIrpStackLocation(p_irp);

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  nt_status =  HSO_generate_report(p_device_object, rgGameReport, &cbReport);

  if( NT_SUCCESS(nt_status) )
    {
      if( cbReport >  (USHORT) p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength )
        {
	  nt_status = STATUS_BUFFER_TOO_SMALL;

	  HSO_LOG(FILE_IOCTL | HSO_ERROR,\
		  ("HSO_get_report_descriptor: cbReport(0x%x) OutputBufferLength(0x%x)",\
		   cbReport, p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength));

        } 
      else
	{
	  RtlCopyMemory( p_irp->UserBuffer, rgGameReport, cbReport );
	  // Report how many bytes were copied
	  p_irp->IoStatus.Information = cbReport;
	  nt_status = STATUS_SUCCESS;
	}
    }

  HSO_EXITPROC(FILE_IOCTL |HSO_FUNCTION_EXIT_OK, "HSO_get_report_descriptor", nt_status);

  return nt_status;
} /* HSO_get_report_descriptor */



/*
  creates a hid report based on current data from the orb
*/
NTSTATUS  
hso_read_report
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP p_irp
 )
{
  NTSTATUS            nt_status = STATUS_SUCCESS;
  PDEVICE_EXTENSION   p_device_extension;
  PIO_STACK_LOCATION  p_irp_stack;

  HSO_LOG(FILE_IOCTL | HSO_FUNCTION_ENTRY,\
	  ("hso_read_report(p_device_object=0x%x,p_irp=0x%x)", \
	   p_device_object, p_irp));

  /*
   * Get a pointer to the device extension.
   */

  p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION (p_device_object);

  /* now queue this IRP */
  nt_status = HSO_queue_read_report( p_device_extension, p_irp );

  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT,  "hso_read_report", nt_status);

  return nt_status;
} /* hso_read_report */



/*
  respond to IOCTL_HID_GET_ATTRIBUTES
*/
NTSTATUS 
HSO_get_attributes
(
 PDEVICE_OBJECT  p_device_object,
 PIRP            p_irp
 )
{
  NTSTATUS nt_status = STATUS_SUCCESS;
  PIO_STACK_LOCATION  p_irp_stack;

  PAGED_CODE();

  HSO_LOG(FILE_IOCTL | HSO_FUNCTION_ENTRY,\
	  ("HSO_get_attributes(p_device_object=0x%x,p_irp=0x%x)",\
	   p_device_object, p_irp));

  /*
   * Get a pointer to the current location in the p_irp
   */

  p_irp_stack = IoGetCurrentIrpStackLocation(p_irp);

  if( p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof (HID_DEVICE_ATTRIBUTES)   )
    {
      nt_status = STATUS_BUFFER_TOO_SMALL;

      HSO_LOG(FILE_IOCTL | HSO_ERROR,\
	      ("HSO_get_attributes: cbReport(0x%x) OutputBufferLength(0x%x)",\
	       sizeof (HID_DEVICE_ATTRIBUTES), p_irp_stack->Parameters.DeviceIoControl.OutputBufferLength));
    } else
      {
        PDEVICE_EXTENSION       p_device_extension;
        PHID_DEVICE_ATTRIBUTES  DeviceAttributes;

        /*
         * Get a pointer to the device extension
         */
        p_device_extension = GET_MINIDRIVER_DEVICE_EXTENSION(p_device_object);
        DeviceAttributes = (PHID_DEVICE_ATTRIBUTES) p_irp->UserBuffer;

        RtlZeroMemory( DeviceAttributes, sizeof(*DeviceAttributes));

        /*
         * Report how many bytes were copied
         */

        p_irp->IoStatus.Information   = sizeof(*DeviceAttributes);

        DeviceAttributes->Size          = sizeof (*DeviceAttributes);
        DeviceAttributes->VendorID      = HIDSPORB_VENDOR_ID;
        DeviceAttributes->ProductID     = HIDSPORB_PRODUCT_ID;
        DeviceAttributes->VersionNumber = HIDSPORB_VERSION_NUMBER;
	
      }

  HSO_EXITPROC(FILE_IOCTL|HSO_FUNCTION_EXIT_OK, "HSO_get_attributes", nt_status);

  return nt_status;
} /* HSO_get_attributes */





