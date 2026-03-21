#include "l2ndis.h"
#include "l2hw.h"

ULONG
L2ReadReg32(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    )
{
    ULONG value = 0;

    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return 0;
    }

    NdisReadRegisterUlong((PULONG)(Adapter->Registers + RegisterOffset), &value);
    return value;
}

VOID
L2WriteReg32(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN ULONG Value
    )
{
    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return;
    }

    NdisWriteRegisterUlong((PULONG)(Adapter->Registers + RegisterOffset), Value);
}

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    )
{
    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return NDIS_STATUS_SUCCESS;
    }

    L2WriteReg32(Adapter, L2_REG_MASTER_CTRL, L2_MASTER_CTRL_SOFT_RST);
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2ReadPermanentMac(
    IN PL2_ADAPTER Adapter,
    OUT PUCHAR Address,
    IN ULONG AddressLength
    )
{
    ULONG macLow;
    ULONG macHigh;

    if ((Adapter == NULL) || (Address == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressLength < L2_ETH_ADDR_LENGTH) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (Adapter->Registers == NULL) {
        NdisMoveMemory(Address, Adapter->PermanentAddress, L2_ETH_ADDR_LENGTH);
        return NDIS_STATUS_SUCCESS;
    }

    macLow = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR);
    macHigh = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR + 4);

    Address[0] = (UCHAR)(macLow & 0xFF);
    Address[1] = (UCHAR)((macLow >> 8) & 0xFF);
    Address[2] = (UCHAR)((macLow >> 16) & 0xFF);
    Address[3] = (UCHAR)((macLow >> 24) & 0xFF);
    Address[4] = (UCHAR)(macHigh & 0xFF);
    Address[5] = (UCHAR)((macHigh >> 8) & 0xFF);

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2HwInitialize(
    IN PL2_ADAPTER Adapter
    )
{
    NDIS_STATUS status;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    /*
     * TODO(phase-3): discover PCI resources and map MMIO BAR before touching
     * device registers. Until then, keep this safe and non-fatal.
     */
    if (Adapter->Registers == NULL) {
        Adapter->HardwareReady = FALSE;
        return NDIS_STATUS_SUCCESS;
    }

    status = L2HwReset(Adapter);
    if (status != NDIS_STATUS_SUCCESS) {
        Adapter->HardwareReady = FALSE;
        return status;
    }

    status = L2ReadPermanentMac(Adapter, Adapter->PermanentAddress, L2_ETH_ADDR_LENGTH);
    if (status != NDIS_STATUS_SUCCESS) {
        Adapter->HardwareReady = FALSE;
        return status;
    }

    NdisMoveMemory(Adapter->CurrentAddress,
                   Adapter->PermanentAddress,
                   L2_ETH_ADDR_LENGTH);

    Adapter->HardwareReady = TRUE;
    return NDIS_STATUS_SUCCESS;
}
