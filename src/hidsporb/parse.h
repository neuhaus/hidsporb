//
// parse.h
//

NTSTATUS
OrbReadPacket(IN PDEVICE_EXTENSION devExt);

VOID
OrbClearBuffer(IN PDEVICE_EXTENSION devExt, IN ULONG packetType);

VOID
OrbParseChar(IN PDEVICE_EXTENSION devExt, IN UCHAR c);

VOID
OrbParsePacket(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);

VOID
OrbParseReset(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);

VOID
OrbXorPacket(IN PCHAR buffer, IN USHORT pktType, IN USHORT length);

VOID
OrbPrintAxes(IN PDEVICE_EXTENSION devExt);

VOID
OrbParseBallData(IN PDEVICE_EXTENSION devExt, IN PCHAR pch, IN USHORT length);

VOID
OrbParseButtonData(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);

VOID
OrbParseError(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);

VOID
OrbParseNullRegion(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);

VOID
OrbParseTerm(IN PDEVICE_EXTENSION devExt, IN PCHAR buffer, IN USHORT length);
