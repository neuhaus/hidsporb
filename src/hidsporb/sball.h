
#ifndef SBALL_H
#define SBALL_H

//
// sorb.h
//
// SpaceOrb 360 detection/parser engine
//

NTSTATUS
SBallDetect(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP detectIrp);

NTSTATUS
SBallInit(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData,
	IN PIRP initIrp, PORB_PARSE_PACKET_FUNC parsePacketFunc,
	IN PVOID parseContext);

NTSTATUS
SBallCleanup(IN PDEVICE_OBJECT serObj, IN PORB_DATA orbData, IN PIRP cleanupIrp);

VOID
SBallParseChar(IN PORB_DATA orbData, IN UCHAR c);

#endif
