#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\minivdd\i910test\i910test.h"

static void usage(void)
{
    printf("Usage:\n");
    printf("  vxd_ping.exe <offset_hex>\n");
    printf("  vxd_ping.exe <offset_hex> <value_hex>\n");
}

int main(int argc, char **argv)
{
    HANDLE h;
    DWORD returned;
    I910_PCI_INFO info;
    I910_MMIO_READ_INPUT inRead;
    I910_MMIO_READ_OUTPUT outRead;

    h = CreateFileA("\\\\.\\I910TEST", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed: %lu\n", GetLastError());
        return 1;
    }

    if (!DeviceIoControl(h,
                         IOCTL_I910_GET_PCI_INFO,
                         NULL,
                         0,
                         &info,
                         sizeof(info),
                         &returned,
                         NULL))
    {
        printf("GET_PCI_INFO failed: %lu\n", GetLastError());
        CloseHandle(h);
        return 1;
    }

    printf("found=%lu ven=%04lX dev=%04lX rev=%02lX cmd=%04lX\n",
           info.found,
           info.vendor_id,
           info.device_id,
           info.revision_id,
           info.command_reg);
    printf("bar0=0x%08lX size=0x%08lX bar2=0x%08lX size=0x%08lX bdf=%lu:%lu.%lu\n",
           info.bar0_mmio_base,
           info.bar0_size,
           info.bar2_aperture_base,
           info.bar2_size,
           info.bus,
           info.slot,
           info.function);

    if (argc >= 2)
    {
        inRead.offset = strtoul(argv[1], NULL, 16);
        if (!DeviceIoControl(h,
                             IOCTL_I910_READ_MMIO32,
                             &inRead,
                             sizeof(inRead),
                             &outRead,
                             sizeof(outRead),
                             &returned,
                             NULL))
        {
            printf("READ_MMIO32 failed: %lu\n", GetLastError());
            CloseHandle(h);
            return 1;
        }

        printf("MMIO[0x%08lX] = 0x%08lX\n", inRead.offset, outRead.value);
    }

    if (argc >= 3)
    {
        I910_MMIO_WRITE_INPUT inWrite;
        inWrite.offset = strtoul(argv[1], NULL, 16);
        inWrite.value = strtoul(argv[2], NULL, 16);

        if (!DeviceIoControl(h,
                             IOCTL_I910_WRITE_MMIO32,
                             &inWrite,
                             sizeof(inWrite),
                             NULL,
                             0,
                             &returned,
                             NULL))
        {
            printf("WRITE_MMIO32 failed: %lu\n", GetLastError());
            CloseHandle(h);
            return 1;
        }

        printf("MMIO write complete offset=0x%08lX value=0x%08lX\n", inWrite.offset, inWrite.value);
    }

    if (argc < 2)
    {
        usage();
    }

    CloseHandle(h);
    return 0;
}
