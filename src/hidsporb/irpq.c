//
// irpq.c
//
// READ_REPORT queue management functions
//

#include "hidsporb.h"

// Initialize queue
VOID
OrbInitReadQueue(IN PDEVICE_EXTENSION devExt)
{
	KeInitializeSpinLock(&devExt->readQueueLock);
	InitializeListHead(&devExt->readQueueList);
}

// Place READ_REPORT on queue and 
NTSTATUS
OrbQueueReadReport(IN PDEVICE_EXTENSION devExt, IN PIRP Irp)
{
	PORB_QUEUE_ITEM item;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	// Allocate queue item
	item = (PORB_QUEUE_ITEM) ExAllocatePoolWithTag(NonPagedPool, sizeof(ORB_QUEUE_ITEM), 'QbrO');
	// Fail if no memory
	if (item == NULL) {
		// Bad luck
		DbgOut(ORB_DBG_IRPQ, ("OrbQueueReadReport(): no item\n"));

		// Complete Irp with error
		return CompleteIrp(Irp, status, 0);
	}
	// Remember to always mark Irp pending
	IoMarkIrpPending(Irp);
	// Save Irp pointer for later processing
	item->Irp = Irp;
	// Insert into queue of pending requests
	ExInterlockedInsertHeadList(&devExt->readQueueList, &item->List, &devExt->readQueueLock);
	// Increment pending I/O count
	InterlockedIncrement(&devExt->readsPending);
	//DbgOut(ORB_DBG_IRPQ, ("OrbQueueReadReport(): inserted\n"));

	return STATUS_PENDING;
}

// Remove first element from READ_REPORT queue
PORB_QUEUE_ITEM
OrbDequeueReadReport(IN PDEVICE_EXTENSION devExt)
{
	PORB_QUEUE_ITEM item;

	item = (PORB_QUEUE_ITEM) ExInterlockedRemoveHeadList(&devExt->readQueueList, &devExt->readQueueLock);
	// See if queue has something
	if (item) {
		// Decrement pending I/O count
		InterlockedDecrement(&devExt->readsPending);
	}

	return item;
}

// Flush all Irps on READ_REPORT queue with specified status
VOID
OrbFlushQueue(IN PDEVICE_EXTENSION devExt, NTSTATUS status)
{
	PORB_QUEUE_ITEM item;

	// Dequeue first element
	item = OrbDequeueReadReport(devExt);
	// While there is something
	while (item) {
		// Complete IR
		CompleteIrp(item->Irp, status, 0);
		// Free Item
		ExFreePool(item);
		// Dequeue next element
		item = OrbDequeueReadReport(devExt);
	}
}

// Dequeue and complete Irp
VOID
OrbCompletePacket(IN PORB_DATA orbData, IN PDEVICE_EXTENSION devExt)
{
	PORB_QUEUE_ITEM item;
	PHIDSPORB_REPORT report;
	HIDSPORB_REPORT localReport;
	ULONG i;

	// Dequeue item
	item = OrbDequeueReadReport(devExt);
	// Just return if there is nothing on queue
	if (item == NULL) {
		return;
	}
	// Initialize report
	report = item->Irp->UserBuffer;
	RtlZeroMemory(&localReport, sizeof(HIDSPORB_REPORT));
	// Note, we can have many reports
	localReport.reportId = 1;
	// Copy Axes
	for (i = 0; i < ORB_NUM_AXES; i++) {
		localReport.Axes[i] = OrbLogicalAxisValue(orbData, i, 0);
	}
	// Copy buttons
	localReport.buttonMap = OrbMapButtons(orbData);
//#if DBG
	if (localReport.buttonMap) {
	DbgOut(ORB_DBG_REPORT, ("OrbCompletePacket(): buttons %x\n", (ULONG) localReport.buttonMap));
	}
//#endif
	// Copy report
	RtlCopyMemory(report, &localReport, sizeof(HIDSPORB_REPORT));
	// Complete IRP
	CompleteIrp(item->Irp, STATUS_SUCCESS, sizeof(HIDSPORB_REPORT));
	// Free queue item
	ExFreePool(item);
}
