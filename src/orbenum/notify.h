//
// notify.h
//

typedef struct _ORB_NOTIFY_CONTEXT {
	ULONG		flags;	  // 0 for arrival, 1 for removal
	PWCHAR		linkName; // Link name
	PDEVICE_OBJECT	fdo;	// Our bus FDO
	PIO_WORKITEM	item;	// Work item pointer
} ORB_NOTIFY_CONTEXT, *PORB_NOTIFY_CONTEXT;

// Notify functions
NTSTATUS
OrbPnpNotifyCallback(IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION pnp, IN PDEVICE_EXTENSION devExt);

VOID
OrbWorkRoutine(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx);

NTSTATUS
OrbCreatePdo(IN PDEVICE_OBJECT fdo, OUT PDEVICE_OBJECT *ppdo);

NTSTATUS
OrbInitPdo(IN PDEVICE_OBJECT pdo, IN PDEVICE_OBJECT targetFdo, IN PWCHAR hardwareId, IN PWCHAR deviceId, IN ULONG numDevice);

VOID
OrbNotifyArrival(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx);

VOID
OrbNotifyRemoval(IN PDEVICE_OBJECT fdo, IN PORB_NOTIFY_CONTEXT ctx);

