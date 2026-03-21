#include <basedef.h>
#include <vmm.h>
#include <vxdwraps.h>
#include <string.h>
#include "i910test.h"
#include "pci.h"
#include "mmio.h"
#include "log.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

I910_DEVICE_CONTEXT g_i910;

BOOL
I910_InitializeDriver(DWORD devNode)
{
    memset(&g_i910, 0, sizeof(g_i910));

    I910_LogInitEarly();
    I910_Log("init: start, devnode=0x%08lx", devNode);

    if (!I910_FindDevice(&g_i910.pci))
    {
        I910_Log("init: intel gpu not found; device remains available for diagnostics");
        return TRUE;
    }

    I910_Log("init: pci found ven=0x%04lx dev=0x%04lx rev=0x%02lx bar0=0x%08lx bar2=0x%08lx cmd=0x%04lx",
             g_i910.pci.vendor_id,
             g_i910.pci.device_id,
             g_i910.pci.revision_id,
             g_i910.pci.bar0_mmio_base,
             g_i910.pci.bar2_aperture_base,
             g_i910.pci.command_reg);

    if (!I910_MapBars(&g_i910))
    {
        I910_Log("init: mmio map failed; read/write ioctls will return error");
    }

    return TRUE;
}
