//
// irpq.h
//

typedef struct _ORB_QUEUE_ITEM {
	LIST_ENTRY List;
	PIRP Irp;
} ORB_QUEUE_ITEM, *PORB_QUEUE_ITEM;

VOID
OrbInitReadQueue(IN PDEVICE_EXTENSION devExt);

VOID
OrbCleanupReadQueue(IN PDEVICE_EXTENSION devExt);

NTSTATUS
OrbQueueReadReport(IN PDEVICE_EXTENSION devExt, IN PIRP Irp);

PORB_QUEUE_ITEM
OrbDequeueReadReport(IN PDEVICE_EXTENSION devExt);

VOID
OrbFlushQueue(IN PDEVICE_EXTENSION devExt, NTSTATUS status);

VOID
OrbCompletePacket(IN PORB_DATA orbData, IN PDEVICE_EXTENSION devExt);
