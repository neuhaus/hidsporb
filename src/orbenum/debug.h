#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG_BASE_H
#include "debug_base.h"
#endif


#if DBG

#define	ORB_DBG_SER	 	(1 << 1)
#define ORB_DBG_NOTIFY   	(1 << 2)
#define ORB_DBG_DETECT   	(1 << 3)
#define ORB_DBG_FDO      	(1 << 4)
#define ORB_DBG_INITUNLO 	(1 << 5)
#define ORB_DBG_LOCK     	(1 << 6)
#define ORB_DBG_ORBIO    	(1 << 7)
#define ORB_DBG_PNP         	(1 << 8)
#define ORB_DBG_POWER    	(1 << 9)
#define ORB_DBG_WMI       	(1 << 10)
#define ORB_DBG_DISPATCH 	(1 << 11)
#define ORB_DBG_PDO     	(1 << 12)
#define ORB_DBG_ALL 		(unsigned long) (~0L)

PCHAR
DbgSerIoctlToStr(IN ULONG ioctlCode);

#endif
#endif
