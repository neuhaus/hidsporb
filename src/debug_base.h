#ifndef DEBUG_BASE_H
#define DEBUG_BASE_H

extern unsigned long OrbDebugLevel;

#if DBG

char*
PnpToString(unsigned char MinorFunction);


#define	DbgOut(level, _x_)	\
		if (OrbDebugLevel & level) { \
			DbgPrint _x_; \
		}
#else
#define DbgOut( level, _x_ )
#endif

#endif
