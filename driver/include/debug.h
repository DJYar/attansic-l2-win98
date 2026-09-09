#ifndef _L2_DEBUG_H_
#define _L2_DEBUG_H_

#include <ndis.h>

#if DBG
#define DBGPRINT(_x) DbgPrint _x
#else
#define DBGPRINT(_x)
#endif

#endif /* _L2_DEBUG_H_ */
