//
// detect.h
//
// Orb detection code
//

// Number of Orb models we support
#define	ORB_NUM_MODELS		2

typedef NTSTATUS
(*PORB_DETECT)(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp);

typedef NTSTATUS
(*PORB_INIT)(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP initIrp,
	PORB_PARSE_PACKET_FUNC parsePacketFunc, IN PVOID parseContext);

typedef NTSTATUS
(*PORB_CLEANUP)(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp);

typedef struct _HISDPORB_MODEL {
	PWCHAR		pnpId;
	PCHAR		modelName;
	PORB_DETECT	orbDetect;
	PORB_INIT	orbInit;
	PORB_CLEANUP	orbCleanup;
} HIDSPORB_MODEL, *PHIDSPORB_MODEL;

NTSTATUS
OrbDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp);

NTSTATUS
OrbInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP initIrp,
	PORB_PARSE_PACKET_FUNC parsePacketFunc,
	IN PVOID parseContext);

NTSTATUS
OrbCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp);
