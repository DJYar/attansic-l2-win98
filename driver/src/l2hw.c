#include "l2ndis.h"
#include "l2hw.h"
#include "debug.h"

#define L2_RESOURCE_DESC_CAPACITY 16
#define L2_INVALID_RESOURCE_INDEX 0xFFFFFFFF

static BOOLEAN
L2IsSafeReadRegister(
    IN ULONG RegisterOffset
    )
{
    UNREFERENCED_PARAMETER(RegisterOffset);

    /*
     * Ultra-safe bring-up mode: no MMIO validation reads are performed.
     * Return FALSE for all offsets until read safety is proven per-register.
     */
    if ((RegisterOffset == L2_REG_IDLE_STATUS) ||
        (RegisterOffset == L2_REG_STS_RX_PAUSE) ||
        (RegisterOffset == L2_REG_STS_RXD_OV) ||
        (RegisterOffset == L2_REG_MAC_STA_ADDR) ||
        (RegisterOffset == L2_REG_MAC_STA_ADDR_HI)) {
        return TRUE;
    }

    return FALSE;
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

    if (!L2IsSafeReadRegister(RegisterOffset)) {
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
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(RegisterOffset);
    UNREFERENCED_PARAMETER(Value);

    /* Ultra-safe bring-up mode: all MMIO writes are intentionally disabled. */
}

NDIS_STATUS
L2DiscoverAdapterResources(
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
    UINT bufferSize = sizeof(resourceBuffer);
    NDIS_STATUS status;
    UINT i;

    if ((Adapter == NULL) || (WrapperConfigurationContext == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->MmioPhysicalBaseLow = 0;
    Adapter->MmioPhysicalBaseHigh = 0;
    Adapter->MmioMappingSucceeded = FALSE;
    Adapter->ResourceCount = 0;
    Adapter->SelectedResourceIndex = L2_INVALID_RESOURCE_INDEX;
    Adapter->SelectedResourceType = 0;
    Adapter->SelectedResourceLength = 0;

    NdisMQueryAdapterResources(
        &status,
        WrapperConfigurationContext,
        resourceList,
        &bufferSize
        );

    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    Adapter->ResourceCount = resourceList->Count;

    for (i = 0, partial = resourceList->PartialDescriptors;
         i < resourceList->Count;
         ++i, ++partial) {

        if ((partial->Type == CmResourceTypeMemory) &&
            (partial->u.Memory.Length > 0)) {
            Adapter->SelectedResourceIndex = i;
            Adapter->SelectedResourceType = partial->Type;
            Adapter->SelectedResourceLength = partial->u.Memory.Length;
            Adapter->MmioPhysicalBaseLow = partial->u.Memory.Start.LowPart;
            Adapter->MmioPhysicalBaseHigh = partial->u.Memory.Start.HighPart;
            break;
        }
    }

    if (Adapter->SelectedResourceIndex == L2_INVALID_RESOURCE_INDEX) {
        return NDIS_STATUS_RESOURCE_CONFLICT;
    }

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
L2MapDiscoveredMmio(
    IN PL2_ADAPTER Adapter
    )
{
    PHYSICAL_ADDRESS physicalBase;
    NDIS_STATUS status;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->Registers = NULL;
    Adapter->MemoryLength = 0;
    Adapter->MmioMappingSucceeded = FALSE;

    if ((Adapter->SelectedResourceIndex == L2_INVALID_RESOURCE_INDEX) ||
        (Adapter->SelectedResourceType != CmResourceTypeMemory) ||
        (Adapter->SelectedResourceLength == 0)) {
        return NDIS_STATUS_RESOURCE_CONFLICT;
    }

    physicalBase.LowPart = Adapter->MmioPhysicalBaseLow;
    physicalBase.HighPart = Adapter->MmioPhysicalBaseHigh;

    status = NdisMMapIoSpace(
        (PVOID *)&Adapter->Registers,
        Adapter->AdapterHandle,
        physicalBase,
        Adapter->SelectedResourceLength
        );

    if ((status != NDIS_STATUS_SUCCESS) || (Adapter->Registers == NULL)) {
        Adapter->Registers = NULL;
        Adapter->MemoryLength = 0;
        Adapter->MmioMappingSucceeded = FALSE;
        return status;
    }

    Adapter->MemoryLength = Adapter->SelectedResourceLength;
    Adapter->MmioMappingSucceeded = TRUE;
    DBGPRINT(("[L2] MMIO mapped: phys=%08X:%08X len=%lu virt=%p\n",
              Adapter->MmioPhysicalBaseHigh,
              Adapter->MmioPhysicalBaseLow,
              Adapter->MemoryLength,
              Adapter->Registers));
    return NDIS_STATUS_SUCCESS;
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

    Adapter->MmioPhysicalBaseLow = 0;
    Adapter->MmioPhysicalBaseHigh = 0;
    Adapter->MemoryLength = 0;
    Adapter->MmioMappingSucceeded = FALSE;
    Adapter->ResourceCount = 0;
    Adapter->SelectedResourceIndex = L2_INVALID_RESOURCE_INDEX;
    Adapter->SelectedResourceType = 0;
    Adapter->SelectedResourceLength = 0;

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

    Adapter->ResourceCount = resourceList->Count;

    for (i = 0, partial = resourceList->PartialDescriptors;
         i < resourceList->Count;
         ++i, ++partial) {

        if ((partial->Type == CmResourceTypeMemory) &&
            (partial->u.Memory.Length > 0)) {

            selectedAddress = partial->u.Memory.Start;
            selectedLength = partial->u.Memory.Length;
            Adapter->SelectedResourceIndex = i;
            Adapter->SelectedResourceType = partial->Type;
            Adapter->SelectedResourceLength = partial->u.Memory.Length;
            break;
        }
    }

    if (selectedLength == 0) {
        return NDIS_STATUS_RESOURCE_CONFLICT;
    }

    Adapter->MmioPhysicalBaseLow = selectedAddress.LowPart;
    Adapter->MmioPhysicalBaseHigh = selectedAddress.HighPart;
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
        Adapter->MmioPhysicalBaseHigh = 0;
        return status;
    }

    Adapter->MmioMappingSucceeded = TRUE;
    return NDIS_STATUS_SUCCESS;
}

VOID
L2PerformMmioSanityRead(
    IN PL2_ADAPTER Adapter
    )
{
    ULONG offsets[3] = {
        L2_REG_IDLE_STATUS,
        L2_REG_STS_RX_PAUSE,
        L2_REG_STS_RXD_OV
    };
    ULONG values[3] = {0, 0, 0};
    ULONG readCount = 0;
    ULONG allOnesCount = 0;
    UINT i;

    if (Adapter == NULL) {
        return;
    }

    Adapter->SanityReadOffsets[0] = offsets[0];
    Adapter->SanityReadOffsets[1] = offsets[1];
    Adapter->SanityReadOffsets[2] = offsets[2];
    Adapter->SanityReadValues[0] = 0;
    Adapter->SanityReadValues[1] = 0;
    Adapter->SanityReadValues[2] = 0;
    Adapter->SanityReadSuccessCount = 0;
    Adapter->SanityReadSucceeded = FALSE;

    if (Adapter->Registers == NULL) {
        return;
    }

    for (i = 0; i < 3; ++i) {
        if (!L2IsSafeReadRegister(offsets[i])) {
            continue;
        }

        NdisReadRegisterUlong((PULONG)(Adapter->Registers + offsets[i]), &values[i]);
        DBGPRINT(("[L2] SANITY read: reg=0x%X val=0x%08X\n", offsets[i], values[i]));
        ++readCount;

        if (values[i] == 0xFFFFFFFF) {
            ++allOnesCount;
        }
    }

    Adapter->SanityReadValues[0] = values[0];
    Adapter->SanityReadValues[1] = values[1];
    Adapter->SanityReadValues[2] = values[2];

    if ((readCount > 0) && (allOnesCount == readCount)) {
        Adapter->SanityReadSuccessCount = 0;
        Adapter->SanityReadSucceeded = FALSE;
        return;
    }

    Adapter->SanityReadSuccessCount = readCount;
    Adapter->SanityReadSucceeded = (readCount > 0) ? TRUE : FALSE;
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
    Adapter->MmioPhysicalBaseHigh = 0;
    Adapter->MmioMappingSucceeded = FALSE;
    Adapter->PermanentMac[0] = 0;
    Adapter->PermanentMac[1] = 0;
    Adapter->PermanentMac[2] = 0;
    Adapter->PermanentMac[3] = 0;
    Adapter->PermanentMac[4] = 0;
    Adapter->PermanentMac[5] = 0;
    Adapter->MacReadSucceeded = FALSE;
    Adapter->SanityReadOffsets[0] = 0;
    Adapter->SanityReadOffsets[1] = 0;
    Adapter->SanityReadOffsets[2] = 0;
    Adapter->SanityReadValues[0] = 0;
    Adapter->SanityReadValues[1] = 0;
    Adapter->SanityReadValues[2] = 0;
    Adapter->SanityReadSuccessCount = 0;
    Adapter->SanityReadSucceeded = FALSE;
    Adapter->HardwareReady = FALSE;
}

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    )
{
    UNREFERENCED_PARAMETER(Adapter);

    /* Ultra-safe bring-up mode: reset is disabled to avoid MMIO write hangs. */
    return NDIS_STATUS_NOT_SUPPORTED;
}

NDIS_STATUS
L2ReadPermanentMac(
    IN PL2_ADAPTER Adapter,
    OUT PUCHAR Address,
    IN ULONG AddressLength
    )
{
    ULONG macLow = 0;
    ULONG macHigh = 0;
    UCHAR mac[L2_ETH_ADDR_LENGTH];
    BOOLEAN allZero = TRUE;
    BOOLEAN allOnes = TRUE;
    UINT i;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (Address == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressLength < L2_ETH_ADDR_LENGTH) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    Adapter->PermanentMac[0] = 0;
    Adapter->PermanentMac[1] = 0;
    Adapter->PermanentMac[2] = 0;
    Adapter->PermanentMac[3] = 0;
    Adapter->PermanentMac[4] = 0;
    Adapter->PermanentMac[5] = 0;
    Adapter->MacReadSucceeded = FALSE;

    if ((Adapter->Registers == NULL) ||
        !L2IsSafeReadRegister(L2_REG_MAC_STA_ADDR) ||
        !L2IsSafeReadRegister(L2_REG_MAC_STA_ADDR_HI)) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    NdisReadRegisterUlong((PULONG)(Adapter->Registers + L2_REG_MAC_STA_ADDR), &macLow);
    NdisReadRegisterUlong((PULONG)(Adapter->Registers + L2_REG_MAC_STA_ADDR_HI), &macHigh);

    /*
     * Linux atl2 uses REG_MAC_STA_ADDR (0x1488) and +4 for upper bytes.
     * Read-only extraction only; no writes/reset/enable/PHY access.
     */
    mac[0] = (UCHAR)(macLow & 0xFF);
    mac[1] = (UCHAR)((macLow >> 8) & 0xFF);
    mac[2] = (UCHAR)((macLow >> 16) & 0xFF);
    mac[3] = (UCHAR)((macLow >> 24) & 0xFF);
    mac[4] = (UCHAR)(macHigh & 0xFF);
    mac[5] = (UCHAR)((macHigh >> 8) & 0xFF);

    for (i = 0; i < L2_ETH_ADDR_LENGTH; ++i) {
        if (mac[i] != 0x00) {
            allZero = FALSE;
        }

        if (mac[i] != 0xFF) {
            allOnes = FALSE;
        }
    }

    Adapter->PermanentMac[0] = mac[0];
    Adapter->PermanentMac[1] = mac[1];
    Adapter->PermanentMac[2] = mac[2];
    Adapter->PermanentMac[3] = mac[3];
    Adapter->PermanentMac[4] = mac[4];
    Adapter->PermanentMac[5] = mac[5];

    DBGPRINT(("[L2] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

    if (allZero || allOnes) {
        DBGPRINT(("[L2] MAC INVALID\n"));
        NdisZeroMemory(Address, AddressLength);
        Adapter->MacReadSucceeded = FALSE;
        return NDIS_STATUS_FAILURE;
    }

    NdisMoveMemory(Address, mac, L2_ETH_ADDR_LENGTH);
    Adapter->MacReadSucceeded = TRUE;
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2HwInitialize(
    IN PL2_ADAPTER Adapter
    )
{
    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if ((Adapter->Registers == NULL) || !Adapter->MmioMappingSucceeded) {
        Adapter->HardwareReady = FALSE;
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    /*
     * Ultra-safe bring-up mode: MMIO mapping only.
     * No MMIO writes, reset, PHY access, interrupt setup, or polling loops.
     */
    Adapter->HardwareReady = FALSE;
    return NDIS_STATUS_SUCCESS;
}
