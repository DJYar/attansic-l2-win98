#ifndef _I910TEST_MMIO_H_
#define _I910TEST_MMIO_H_

#include "i910test.h"

BOOL I910_MapBars(PI910_DEVICE_CONTEXT ctx);
DWORD I910_ReadMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD *value);
DWORD I910_WriteMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD value);
void I910_DisableMmio(PI910_DEVICE_CONTEXT ctx, const char *reason);

#endif
