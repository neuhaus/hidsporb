//
// debug.h
//

// debug functions
// Serial I/O debugging
#define	ORB_DBG_SER	(1 << 1)

#if DBG
PCHAR
PnpToString(UCHAR MinorFunction);

#define	DbgOut(_x_)	\
		DbgPrint _x_;
#else
#define	DbgOut(_x_)
#endif
