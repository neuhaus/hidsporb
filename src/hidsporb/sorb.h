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

VOID
SOrbXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length);

VOID
SOrbParsePacket(IN PORB_DATA orbData);

VOID
SOrbParseReset(IN PORB_DATA orbData);

VOID
SOrbParseBallData(IN PORB_DATA orbData);

VOID
SOrbParseButtonData(IN PORB_DATA orbData);

VOID
SOrbParseError(IN PORB_DATA orbData);

VOID
SOrbParseNullRegion(IN PORB_DATA orbData);

VOID
SOrbParseTerm(IN PORB_DATA orbData);
