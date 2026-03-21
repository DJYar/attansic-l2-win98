#include "l2ndis.h"
#include "l2hw.h"

#define L2_RESOURCE_DESC_CAPACITY 16
#define L2_RESET_POLL_COUNT       10
#define L2_RESET_POLL_DELAY_US    1000

#define L2_LONGSWAP(_a)     ((((ULONG)(_a) & 0x00ff00ffUL) << 8) | (((ULONG)(_a) & 0xff00ff00UL) >> 8))
#define L2_SWAPDWORD(_a) ((L2_LONGSWAP(_a) << 16) | (L2_LONGSWAP(_a) >> 16))
#define L2_SWAPWORD(_a)  (USHORT)((((USHORT)(_a)) << 8) | (((USHORT)(_a)) >> 8))

static BOOLEAN
L2IsValidPermanentMac(
    IN PUCHAR Address
    )
{
    static const UCHAR zeroMac[L2_ETH_ADDR_LENGTH] = {0, 0, 0, 0, 0, 0};
    static const UCHAR ffMac[L2_ETH_ADDR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (Address == NULL) {
        return FALSE;
    }

    if (NdisEqualMemory(Address, zeroMac, L2_ETH_ADDR_LENGTH)) {
        return FALSE;
    }

    if (NdisEqualMemory(Address, ffMac, L2_ETH_ADDR_LENGTH)) {
        return FALSE;
    }

    if (Address[0] & 0x01) {
        return FALSE;
    }

    return TRUE;
}

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
L2MapHardwareResources(
    IN PL2_ADAPTER Adapter,
    IN NDIS_HANDLE WrapperConfigurationContext
    )
{
    UCHAR resourceBuffer[
        sizeof(NDIS_RESOURCE_LIST) +
        (L2_RESOURCE_DESC_CAPACITY * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR))
    ];
    PNDIS_RESOURCE_LIST resourceList = (PNDIS_RESOURCE_LIST)resourceBuffer;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PHYSICAL_ADDRESS selectedAddress;
    UINT bufferSize = sizeof(resourceBuffer);
    NDIS_STATUS status;
    UINT i;
    ULONG selectedLength = 0;

    if ((Adapter == NULL) || (WrapperConfigurationContext == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    NdisZeroMemory(&selectedAddress, sizeof(selectedAddress));

    NdisMQueryAdapterResources(
        &status,
        WrapperConfigurationContext,
        resourceList,
        &bufferSize
        );

    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    for (i = 0, partial = resourceList->PartialDescriptors;
         i < resourceList->Count;
         ++i, ++partial) {

        if ((partial->Type == CmResourceTypeMemory) &&
            (partial->u.Memory.Length > 0)) {

            selectedAddress = partial->u.Memory.Start;
            selectedLength = partial->u.Memory.Length;
            break;
        }
    }

    if (selectedLength == 0) {
        return NDIS_STATUS_RESOURCE_CONFLICT;
    }

    Adapter->MmioPhysicalBaseLow = selectedAddress.LowPart;
    Adapter->MemoryLength = selectedLength;

    status = NdisMMapIoSpace(
        (PVOID *)&Adapter->Registers,
        Adapter->AdapterHandle,
        selectedAddress,
        selectedLength
        );

    if ((status != NDIS_STATUS_SUCCESS) || (Adapter->Registers == NULL)) {
        Adapter->Registers = NULL;
        Adapter->MemoryLength = 0;
        Adapter->MmioPhysicalBaseLow = 0;
        return status;
    }

    return NDIS_STATUS_SUCCESS;
}

VOID
L2HwShutdown(
    IN PL2_ADAPTER Adapter
    )
{
    if (Adapter == NULL) {
        return;
    }

    if (Adapter->Registers != NULL) {
        NdisMUnmapIoSpace(
            Adapter->AdapterHandle,
            Adapter->Registers,
            Adapter->MemoryLength
            );

        Adapter->Registers = NULL;
    }

    Adapter->MemoryLength = 0;
    Adapter->MmioPhysicalBaseLow = 0;
    Adapter->HardwareReady = FALSE;
}

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    )
{
    ULONG idleStatus = 0xFFFFFFFF;
    ULONG masterControl = 0xFFFFFFFF;
    UINT i;

    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    L2WriteReg32(Adapter, L2_REG_MASTER_CTRL, L2_MASTER_CTRL_SOFT_RST);

    for (i = 0; i < L2_RESET_POLL_COUNT; ++i) {
        NdisMSleep(L2_RESET_POLL_DELAY_US);

        masterControl = L2ReadReg32(Adapter, L2_REG_MASTER_CTRL);
        idleStatus = L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);

        if (((masterControl & L2_MASTER_CTRL_SOFT_RST) == 0) &&
            (idleStatus == 0)) {
            return NDIS_STATUS_SUCCESS;
        }
    }

    return NDIS_STATUS_HARD_ERRORS;
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
    ULONG swappedLow;
    USHORT swappedHigh;

    if ((Adapter == NULL) || (Address == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressLength < L2_ETH_ADDR_LENGTH) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    if (Adapter->Registers == NULL) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    /*
     * Linux atl2 fallback path reads REG_MAC_STA_ADDR / +4 for station MAC.
     * We use the same register source conservatively at this stage.
     */
    macLow = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR);
    macHigh = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR + 4);

    swappedLow = L2_SWAPDWORD(macLow);
    swappedHigh = L2_SWAPWORD((USHORT)(macHigh & 0xFFFF));

    Address[0] = (UCHAR)(swappedHigh & 0xFF);
    Address[1] = (UCHAR)((swappedHigh >> 8) & 0xFF);
    Address[2] = (UCHAR)(swappedLow & 0xFF);
    Address[3] = (UCHAR)((swappedLow >> 8) & 0xFF);
    Address[4] = (UCHAR)((swappedLow >> 16) & 0xFF);
    Address[5] = (UCHAR)((swappedLow >> 24) & 0xFF);

    if (!L2IsValidPermanentMac(Address)) {
        return NDIS_STATUS_INVALID_ADDRESS;
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2HwInitialize(
    IN PL2_ADAPTER Adapter
    )
{
    NDIS_STATUS status;
    ULONG sanityMaster;
    ULONG sanityIdle;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (Adapter->Registers == NULL) {
        Adapter->HardwareReady = FALSE;
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    sanityMaster = L2ReadReg32(Adapter, L2_REG_MASTER_CTRL);
    sanityIdle = L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);

    if ((sanityMaster == 0xFFFFFFFF) && (sanityIdle == 0xFFFFFFFF)) {
        Adapter->HardwareReady = FALSE;
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
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
