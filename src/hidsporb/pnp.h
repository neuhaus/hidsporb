#ifndef PNP_H
#define PNP_H

NTSTATUS 
HSO_increment_request_count
(
 PDEVICE_EXTENSION p_device_extension
 );

VOID 
HSO_decrement_request_count
(
 PDEVICE_EXTENSION p_device_extension
 );

VOID 
HSO_RemoveDevice
(
 PDEVICE_OBJECT p_device_object,
 PDEVICE_EXTENSION p_device_extension,
 PIRP Irp
 );

NTSTATUS  
HSO_dispatch_pnp
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );

NTSTATUS 
HSO_pnp_mn_remove_device(
			 PDEVICE_OBJECT p_device_object,
			 PDEVICE_EXTENSION p_device_extension,
			 PIRP Irp
			 );


NTSTATUS 
HSO_pnp_mn_start_device(
			IN PDEVICE_OBJECT p_device_object,
			PDEVICE_EXTENSION p_device_extension,
			IN PIRP Irp
			);

NTSTATUS 
HSO_init_device
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP           Irp
 );

NTSTATUS 
HSO_power
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP pIrp
 );

VOID 
show_device_property_string(PDEVICE_OBJECT pdo,
			    char* property_desc,
			    DEVICE_REGISTRY_PROPERTY property
			    );

NTSTATUS 
HSO_get_resources
(
 IN PDEVICE_OBJECT p_device_object,
 IN PIRP Irp
 );



#endif
