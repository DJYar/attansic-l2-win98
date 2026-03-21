#include "mmio.h"
#include "log.h"

#pragma VxD_LOCKED_CODE_SEG

BOOL
I910_MapBars(PI910_DEVICE_CONTEXT ctx)
{
    DWORD size = ctx->pci.bar0_size;

    if (!ctx->pci.found)
    {
        I910_Log("mmio: map skipped, pci device is not present");
        return FALSE;
    }

    if (!ctx->pci.bar0_mmio_base)
    {
        I910_Log("mmio: BAR0 is zero");
        return FALSE;
    }

    if (size == 0)
    {
        size = I910_MAX_MMIO_SIZE_FALLBACK;
        I910_Log("mmio: BAR0 size unknown, using fallback size=0x%08lx", size);
    }

    ctx->mmio_linear = _MapPhysToLinear(ctx->pci.bar0_mmio_base, size, 0);
    if (!ctx->mmio_linear)
    {
        I910_Log("mmio: map failed phys=0x%08lx size=0x%08lx", ctx->pci.bar0_mmio_base, size);
        return FALSE;
    }

    ctx->mmio_size = size;
    ctx->mmio_disabled = 0;
    ctx->mmio_error_count = 0;

    I910_Log("mmio: mapped phys=0x%08lx -> lin=0x%08lx size=0x%08lx",
             ctx->pci.bar0_mmio_base,
             (DWORD)ctx->mmio_linear,
             ctx->mmio_size);

    return TRUE;
}

void
I910_DisableMmio(PI910_DEVICE_CONTEXT ctx, const char *reason)
{
    ctx->mmio_disabled = 1;
    I910_Log("mmio: disabled (%s)", reason ? reason : "unknown reason");
}

static DWORD
I910_ValidateOffset(PI910_DEVICE_CONTEXT ctx, DWORD offset)
{
    if (ctx->mmio_disabled)
    {
        return ERROR_GEN_FAILURE;
    }

    if (!ctx->mmio_linear || ctx->mmio_size < 4)
    {
        return ERROR_DEV_NOT_EXIST;
    }

    if (offset & 0x3)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (offset > (ctx->mmio_size - sizeof(DWORD)))
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

DWORD
I910_ReadMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD *value)
{
    DWORD status = I910_ValidateOffset(ctx, offset);

    if (status != NO_ERROR)
    {
        ctx->mmio_error_count++;
        if (ctx->mmio_error_count >= I910_MMIO_WATCHDOG_LIMIT)
        {
            I910_DisableMmio(ctx, "watchdog threshold reached in read");
        }
        return status;
    }

    *value = *((volatile DWORD *)((BYTE *)ctx->mmio_linear + offset));
    ctx->mmio_error_count = 0;
    return NO_ERROR;
}

DWORD
I910_WriteMmio32(PI910_DEVICE_CONTEXT ctx, DWORD offset, DWORD value)
{
    DWORD status = I910_ValidateOffset(ctx, offset);

    if (status != NO_ERROR)
    {
        ctx->mmio_error_count++;
        if (ctx->mmio_error_count >= I910_MMIO_WATCHDOG_LIMIT)
        {
            I910_DisableMmio(ctx, "watchdog threshold reached in write");
        }
        return status;
    }

    *((volatile DWORD *)((BYTE *)ctx->mmio_linear + offset)) = value;
    ctx->mmio_error_count = 0;
    return NO_ERROR;
}
