//
// sorb.h
//
// SpaceOrb 360 detection/parser engine
//


NTSTATUS
SOrbDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp);

NTSTATUS
SOrbInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData,
	IN PIRP initIrp, PORB_PARSE_PACKET_FUNC parsePacketFunc,
	IN PVOID parseContext);

NTSTATUS
SOrbCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp);

VOID
SOrbParseChar(IN PORB_DATA orbData, IN UCHAR c);

