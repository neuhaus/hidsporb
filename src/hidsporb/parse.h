//
// parse.h
//
// Orb generic packet parser engine
//

NTSTATUS
OrbReadPacket(IN PORB_DATA orbData, IN PDEVICE_OBJECT serObj, IN PIRP readIrp);

VOID
OrbClearBuffer(IN PORB_DATA orbData, IN ULONG packetType);

VOID
OrbParseCallFunc(IN PORB_DATA orbData);

