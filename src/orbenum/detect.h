//
// detect.h
//

// ORB models
typedef struct _ORB_MODEL {
	PWCHAR model;
	PWCHAR hardwareId;
	PWCHAR deviceId;
} ORB_MODEL, *PORB_MODEL;

// Detect functions
ULONG
OrbDetect(IN PDEVICE_OBJECT serObj);

NTSTATUS
OrbPortArrival(IN PDEVICE_EXTENSION devExt, IN PORB_NOTIFY_CONTEXT ctx);

NTSTATUS
OrbPortRemoval(IN PDEVICE_EXTENSION devExt, IN PORB_NOTIFY_CONTEXT ctx);

VOID
OrbDeletePdo(IN PPDO_EXTENSION pdevExt);

VOID
OrbMarkPdoAsRemoved(IN PPDO_EXTENSION pdevExt);

ULONG
OrbGetNextPdoNumber(IN PDEVICE_EXTENSION devExt);

VOID
OrbFreePdoNumber(IN PDEVICE_EXTENSION devExt, IN ULONG instanceId);
