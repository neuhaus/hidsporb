//
// detect.c
// ORB detection routines
//

#include "detinc.h"

// List of supported models
HIDSPORB_MODEL orbModels[ORB_NUM_MODELS] = {
{ L"*SPC0360", "SpaceOrb 360", SOrbDetect, SOrbInit, SOrbCleanup },
{ L"*SPC3003", "SpaceBall 3003", SBallDetect, SBallInit, SBallCleanup }
};
// Note for SpaceBall 3003 functions are null
// add them when they're complete

// These two beasts are static and belong to this file
static NTSTATUS
OrbGetPnpIdComplete(IN PDEVICE_OBJECT devObj, IN PIRP Irp, IN PKEVENT event)
{
	// We don't care about Irp->PendingReturned!
	KeSetEvent(event, FALSE, 0);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

// Call this functions to get ORB PNP ID from lower PDO
// We detect different ORB by PNP IDs
static NTSTATUS
OrbGetPnpId(IN PDEVICE_OBJECT devObj, OUT PWCHAR *pnpId)
{
	PIRP Irp;
	PIO_STACK_LOCATION irpSp;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KEVENT event;
	ULONG stackSize;

	DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): enter\n"));
	// Check if pointer is valid
	if (pnpId == NULL) {
		DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): null buffer\n"));
		goto failed;
	}
	// Allocate IRP
	stackSize = devObj->StackSize;
	DbgOut( ORB_DBG_ALL, ("OrbGetPnpId(): stacksize %d\n", stackSize));
	Irp = IoAllocateIrp(devObj->StackSize + 1, FALSE);
	if (Irp == NULL) {
		DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): no IRP\n"));
		goto failed;
	}
	// Skip our IRP stack location
	//IoSetNextIrpStackLocation(Irp);
	// Note, IRP stack location points to next SL
	// IoSetNextIrpStackLocation() doesn't work in some cases
	// So we get next location by hand
	irpSp = IoGetNextIrpStackLocation(Irp);
	// Set up major/minor stuff
	irpSp->MajorFunction = IRP_MJ_PNP;
	irpSp->MinorFunction = IRP_MN_QUERY_ID;
	// Set up parameters
	irpSp->Parameters.QueryId.IdType = BusQueryHardwareIDs;
	// Don't forget to set this!!!
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) OrbGetPnpIdComplete,
				&event, TRUE, TRUE, TRUE);
	// Initialize event
	KeInitializeEvent(&event, NotificationEvent, FALSE);
	// Call PDO to get PNP ID
	status = IoCallDriver(devObj, Irp);
	// Isn't supposed to happen, but who knows???
	if (status == STATUS_PENDING) {
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
	}
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): call not successful, status %x\n", status));
		goto freeirp;
	}
	DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): returned ID: %ws\n", Irp->IoStatus.Information));
	//ExFreePool((PVOID) Irp->IoStatus.Information);
	*pnpId = Irp->IoStatus.Information;
freeirp:
	// Free Irp
	IoFreeIrp(Irp);
failed:
	DbgOut(ORB_DBG_DETECT, ("OrbGetPnpId(): exit, status %x\n", status));

	return status;
}

// This function is used to detect if there is Orb present
// Note, port must be already opened
NTSTATUS
OrbDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp)
{
	PIRP Irp = NULL;
	PWCHAR pnpId;
	NTSTATUS status = STATUS_NO_SUCH_DEVICE;
	ULONG i, found = 0;

	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): enter\n"));
#if 1
	// Get PNP ID
	status = OrbGetPnpId(serObj, &pnpId);
	// Fail if no success
	if (!NT_SUCCESS(status)) {
		DbgOut(ORB_DBG_DETECT, ("OrbDetect(): PNP ID failed\n"));

		return STATUS_NO_SUCH_DEVICE;
	}
	// Print PNP ID
	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): PNP ID: %ws\n", pnpId));
	// Look and see if there is matching PNP ID
	for (i = 0; i < ORB_NUM_MODELS; i++) {
		if (wcscmp(orbModels[i].pnpId, pnpId) == 0) {
			DbgOut(ORB_DBG_DETECT, ("OrbDetect(): found %s (%ws)\n",
				orbModels[i].modelName, pnpId));
			// Set model
			orbData->orbModel = i;
			// Indicate success
			status = STATUS_SUCCESS;
			found = 1;
			// Detected supported ORB
			// but the code is not there yet
			// Fail detection in such case
			if (orbModels[i].orbInit == NULL) {
				DbgOut(ORB_DBG_DETECT, ("OrbDetect(): no support yet\n"));
				found = 0;
			}
			// Don't forget to free PNP ID buffer!
			ExFreePool(pnpId);
			break;
		}
	}
	if (!found) {
		status = STATUS_NO_SUCH_DEVICE;
		DbgOut(ORB_DBG_DETECT, ("OrbDetect(): unknown model\n"));
	}
#else
	// Old way detection stuff
	// Check & alloc Irp if needed
	if (detectIrp == NULL) {
		Irp = IoAllocateIrp(serObj->StackSize + 1, FALSE);
		if (Irp == NULL) {
			DbgOut(ORB_DBG_DETECT, ("OrbDetect(): no IRP\n"));
			goto failed;
		}
		detectIrp = Irp;
	}
	// Call every detect function
	for (i = 0; i < ORB_NUM_MODELS; i++) {
		status = (*orbModels[i].orbDetect)(serObj, orbData, detectIrp);
		// OK, we detected Orb, exit the loop
		if (status == STATUS_SUCCESS) {
			break;
		}
	}
failed:
	// Free Irp, if any
	if (Irp != NULL) {
		IoFreeIrp(Irp);
	}
#endif
	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): model %d, exit %x\n", orbData->orbModel, status));

	return status;
}

// This function initializes ORB
NTSTATUS
OrbInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData,
	IN PIRP initIrp, PORB_PARSE_PACKET_FUNC parsePacketFunc,
	IN PVOID parseContext)
{
	return (*orbModels[orbData->orbModel].orbInit)(serObj, orbData,
		initIrp, parsePacketFunc, parseContext);
}

// This function cleans up everything
NTSTATUS
OrbCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp)
{
	return (*orbModels[orbData->orbModel].orbCleanup)(serObj, orbData, cleanupIrp);
}
