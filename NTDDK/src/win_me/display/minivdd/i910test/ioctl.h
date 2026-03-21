#ifndef _I910TEST_IOCTL_H_
#define _I910TEST_IOCTL_H_

#include "i910test.h"

DWORD _stdcall I910_DeviceIoControl(DWORD service, DWORD ddb, DWORD hDevice, LPDIOC dio);

#endif
