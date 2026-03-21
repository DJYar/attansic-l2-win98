#include "ioctl.h"
#include "log.h"
#include "mmio.h"

#pragma VxD_LOCKED_CODE_SEG

static void
I910_SetBytesReturned(LPDIOC dio, DWORD size)
{
    if (dio && dio->lpcbBytesReturned)
    {
        *((DWORD *)dio->lpcbBytesReturned) = size;
    }
}

DWORD _stdcall
I910_DeviceIoControl(DWORD service, DWORD ddb, DWORD hDevice, LPDIOC dio)
{
    DWORD code;

    UNREFERENCED_PARAMETER(service);
    UNREFERENCED_PARAMETER(ddb);
    UNREFERENCED_PARAMETER(hDevice);

    if (!dio)
    {
        return ERROR_INVALID_PARAMETER;
    }

    code = dio->dwIoControlCode;

    if (code == DIOC_OPEN || code == DIOC_CLOSEHANDLE)
    {
        I910_Log("ioctl: open/close code=0x%08lx", code);
        return NO_ERROR;
    }

    switch (code)
    {
    case IOCTL_I910_GET_PCI_INFO:
        if (dio->cbOutBuffer < sizeof(I910_PCI_INFO) || !dio->lpvOutBuffer)
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
        *((PI910_PCI_INFO)dio->lpvOutBuffer) = g_i910.pci;
        I910_SetBytesReturned(dio, sizeof(I910_PCI_INFO));
        I910_Log("ioctl: GET_PCI_INFO found=%lu", g_i910.pci.found);
        return NO_ERROR;

    case IOCTL_I910_READ_MMIO32:
    {
        PI910_MMIO_READ_INPUT in;
        PI910_MMIO_READ_OUTPUT out;
        DWORD value;
        DWORD status;

        if (dio->cbInBuffer < sizeof(I910_MMIO_READ_INPUT) ||
            dio->cbOutBuffer < sizeof(I910_MMIO_READ_OUTPUT) ||
            !dio->lpvInBuffer || !dio->lpvOutBuffer)
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        in = (PI910_MMIO_READ_INPUT)dio->lpvInBuffer;
        out = (PI910_MMIO_READ_OUTPUT)dio->lpvOutBuffer;

        status = I910_ReadMmio32(&g_i910, in->offset, &value);
        if (status != NO_ERROR)
        {
            I910_Log("ioctl: READ_MMIO32 failed off=0x%08lx status=%lu", in->offset, status);
            return status;
        }

        out->value = value;
        I910_SetBytesReturned(dio, sizeof(I910_MMIO_READ_OUTPUT));
        I910_Log("ioctl: READ_MMIO32 off=0x%08lx val=0x%08lx", in->offset, value);
        return NO_ERROR;
    }

    case IOCTL_I910_WRITE_MMIO32:
    {
        PI910_MMIO_WRITE_INPUT in;
        DWORD status;

        if (dio->cbInBuffer < sizeof(I910_MMIO_WRITE_INPUT) || !dio->lpvInBuffer)
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        in = (PI910_MMIO_WRITE_INPUT)dio->lpvInBuffer;
        status = I910_WriteMmio32(&g_i910, in->offset, in->value);
        I910_Log("ioctl: WRITE_MMIO32 off=0x%08lx val=0x%08lx status=%lu", in->offset, in->value, status);
        return status;
    }

    case IOCTL_I910_MASK_INTERRUPTS:
        I910_Log("ioctl: MASK_INTERRUPTS stub");
        return ERROR_CALL_NOT_IMPLEMENTED;

    default:
        I910_Log("ioctl: unsupported code=0x%08lx", code);
        return ERROR_NOT_SUPPORTED;
    }
}
