//
// detect.c
// ORB detection routines
//

#include "detinc.h"

// List of supported models
HIDSPORB_MODEL orbModels[ORB_NUM_MODELS] = {
{ "SpaceOrb 360", SOrbDetect, SOrbInit, SOrbCleanup }
};

// This function is used to detect if there is Orb present
// Note, port must be already opened
NTSTATUS
OrbDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp)
{
	PIRP Irp = NULL;
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	ULONG i;

	DbgOut(ORB_DBG_DETECT, ("OrbDetect(): enter\n"));
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
