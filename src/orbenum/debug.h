//
// debug.h
//

// debug functions

#if DBG
PCHAR
PnpToString(UCHAR MinorFunction);

#define	DbgOut(_x_)	\
		DbgPrint _x_;
#else
#define	DbgOut(_x_)
#endif
