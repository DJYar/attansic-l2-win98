#ifndef _I910TEST_H_
#define _I910TEST_H_

#include <basedef.h>
#include <vmm.h>
#include <vwin32.h>
#include <winerror.h>

#define I910TEST_DEVICE_NAME "I910TEST"

#define FILE_DEVICE_I910TEST 0x00008010UL
#define I910TEST_IOCTL_BASE  0x800UL

#define I910TEST_CTL_CODE(id) \
    ((FILE_DEVICE_I910TEST << 16) | ((id) << 2) | 0)

#define IOCTL_I910_GET_PCI_INFO      I910TEST_CTL_CODE(I910TEST_IOCTL_BASE + 1)
#define IOCTL_I910_READ_MMIO32       I910TEST_CTL_CODE(I910TEST_IOCTL_BASE + 2)
#define IOCTL_I910_WRITE_MMIO32      I910TEST_CTL_CODE(I910TEST_IOCTL_BASE + 3)
#define IOCTL_I910_MASK_INTERRUPTS   I910TEST_CTL_CODE(I910TEST_IOCTL_BASE + 4)

#define I910_MAX_MMIO_SIZE_FALLBACK  (16UL * 1024UL * 1024UL)
#define I910_MMIO_WATCHDOG_LIMIT     16

typedef struct _I910_PCI_INFO {
    DWORD vendor_id;
    DWORD device_id;
    DWORD revision_id;
    DWORD command_reg;
    DWORD bar0_mmio_base;
    DWORD bar0_size;
    DWORD bar2_aperture_base;
    DWORD bar2_size;
    DWORD bus;
    DWORD slot;
    DWORD function;
    DWORD found;
} I910_PCI_INFO, *PI910_PCI_INFO;

typedef struct _I910_MMIO_READ_INPUT {
    DWORD offset;
} I910_MMIO_READ_INPUT, *PI910_MMIO_READ_INPUT;

typedef struct _I910_MMIO_READ_OUTPUT {
    DWORD value;
} I910_MMIO_READ_OUTPUT, *PI910_MMIO_READ_OUTPUT;

typedef struct _I910_MMIO_WRITE_INPUT {
    DWORD offset;
    DWORD value;
} I910_MMIO_WRITE_INPUT, *PI910_MMIO_WRITE_INPUT;

typedef struct _I910_DEVICE_CONTEXT {
    I910_PCI_INFO pci;
    PVOID mmio_linear;
    DWORD mmio_size;
    DWORD mmio_disabled;
    DWORD mmio_error_count;
} I910_DEVICE_CONTEXT, *PI910_DEVICE_CONTEXT;

extern I910_DEVICE_CONTEXT g_i910;

BOOL I910_InitializeDriver(DWORD devNode);
DWORD _stdcall I910_DeviceIoControl(DWORD service, DWORD ddb, DWORD hDevice, LPDIOC dio);

BOOL I910_FindDevice(PI910_PCI_INFO info);
BOOL I910_MapBars(PI910_DEVICE_CONTEXT ctx);
DWORD I910_ReadMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD *value);
DWORD I910_WriteMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD value);
void I910_DisableMmio(PI910_DEVICE_CONTEXT ctx, const char *reason);

BOOL I910_LogInitEarly(void);
void I910_Log(const char *fmt, ...);

#endif
