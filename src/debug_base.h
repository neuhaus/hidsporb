#ifndef DEBUG_BASE_H
#define DEBUG_BASE_H

#if DBG

extern unsigned long OrbDebugLevel;

char*
PnpToString(unsigned char minor);

char*
DbgMajorToStr(unsigned char major);

#define	DbgOut(level, _x_)	\
		if (OrbDebugLevel & (level)) { \
			DbgPrint _x_; \
		}
#else
#define DbgOut(level, _x_)
#endif
#endif
