//
// debug.h
//

// debug functions

// global debug level, defined in debug.c
extern ULONG OrbDebugLevel;

#if DBG
PCHAR
PnpToString(UCHAR MinorFunction);

// PNP debugging
#define	ORB_DBG_PNP	1
// Serial I/O debugging
#define	ORB_DBG_SER	2
// I/O debugging
#define	ORB_DBG_IO	4
// Settings debugging
#define	ORB_DBG_SET	8
// Ioctl debugging
#define	ORB_DBG_IOCTL	16
// read_report debugging
#define	ORB_DBG_REPORT	32
// feature debugging
#define	ORB_DBG_FEATURE	64
// ORB parser debugging
#define	ORB_DBG_PARSE	128
// Detection code debugging
#define	ORB_DBG_DETECT	256
// IRP queue debugging
#define	ORB_DBG_IRPQ	512
//
#define	ORB_DBG_DISPATCH	1024
//
#define	ORB_DBG_ALL	(ULONG) (~0L)

#define	DbgOut(level, _x_)	\
		if (OrbDebugLevel & level) { \
			DbgPrint _x_; \
		}
#else
#define	DbgOut(level, _x_)
#endif
