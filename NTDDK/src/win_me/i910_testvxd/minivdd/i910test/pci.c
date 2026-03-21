#include <string.h>
#include "pci.h"
#include "log.h"

#pragma VxD_LOCKED_CODE_SEG

static const WORD g_supported_devices[] = {
    0x2592, /* 915G */
    0x259A, /* 915GM */
    0x2772, /* 945G */
    0x27A2  /* 945GM/910GML class */
};

static DWORD
I910_PciConfigAddress(BYTE bus, BYTE slot, BYTE function, BYTE offset)
{
    return 0x80000000UL |
           ((DWORD)bus << 16) |
           ((DWORD)slot << 11) |
           ((DWORD)function << 8) |
           ((DWORD)offset & 0xFC);
}

static DWORD
I910_ReadPci32(BYTE bus, BYTE slot, BYTE function, BYTE offset)
{
    DWORD address = I910_PciConfigAddress(bus, slot, function, offset);
    _outpd(0xCF8, address);
    return _inpd(0xCFC);
}

static WORD
I910_ReadPci16(BYTE bus, BYTE slot, BYTE function, BYTE offset)
{
    DWORD value = I910_ReadPci32(bus, slot, function, offset);
    return (WORD)((value >> ((offset & 2) * 8)) & 0xFFFF);
}

static BYTE
I910_ReadPci8(BYTE bus, BYTE slot, BYTE function, BYTE offset)
{
    DWORD value = I910_ReadPci32(bus, slot, function, offset);
    return (BYTE)((value >> ((offset & 3) * 8)) & 0xFF);
}

static BOOL
I910_IsSupportedIntelDevice(WORD vendor, WORD device)
{
    DWORD i;

    if (vendor != 0x8086)
    {
        return FALSE;
    }

    for (i = 0; i < sizeof(g_supported_devices) / sizeof(g_supported_devices[0]); ++i)
    {
        if (g_supported_devices[i] == device)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
I910_FindDevice(PI910_PCI_INFO info)
{
    BYTE bus;
    BYTE slot;
    BYTE function;

    memset(info, 0, sizeof(*info));

    for (bus = 0; bus < 0x100; ++bus)
    {
        for (slot = 0; slot < 32; ++slot)
        {
            for (function = 0; function < 8; ++function)
            {
                WORD vendor = I910_ReadPci16(bus, slot, function, 0x00);
                WORD device;

                if (vendor == 0xFFFF)
                {
                    if (function == 0)
                    {
                        break;
                    }
                    continue;
                }

                device = I910_ReadPci16(bus, slot, function, 0x02);

                if (!I910_IsSupportedIntelDevice(vendor, device))
                {
                    continue;
                }

                info->vendor_id = vendor;
                info->device_id = device;
                info->revision_id = I910_ReadPci8(bus, slot, function, 0x08);
                info->command_reg = I910_ReadPci16(bus, slot, function, 0x04);
                info->bar0_mmio_base = I910_ReadPci32(bus, slot, function, 0x10) & ~0x0FUL;
                info->bar2_aperture_base = I910_ReadPci32(bus, slot, function, 0x18) & ~0x0FUL;
                info->bar0_size = 0;
                info->bar2_size = 0;
                info->bus = bus;
                info->slot = slot;
                info->function = function;
                info->found = 1;

                I910_Log("pci: matched bus=%lu slot=%lu func=%lu vendor=0x%04lx device=0x%04lx",
                         info->bus,
                         info->slot,
                         info->function,
                         info->vendor_id,
                         info->device_id);
                return TRUE;
            }
        }
    }

    I910_Log("pci: no supported intel device found");
    return FALSE;
}
