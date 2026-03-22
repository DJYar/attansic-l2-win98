#include "l2ndis.h"
#include "l2hw.h"

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
    if (RegisterOffset == L2_REG_IDLE_STATUS) {
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
    ULONG value = 0;

    if (Adapter == NULL) {
        return;
    }

    Adapter->SanityRegisterOffset = L2_REG_IDLE_STATUS;
    Adapter->SanityRegisterValue = 0;
    Adapter->SanityReadSucceeded = FALSE;

    if ((Adapter->Registers == NULL) ||
        !L2IsSafeReadRegister(Adapter->SanityRegisterOffset)) {
        return;
    }

    NdisReadRegisterUlong(
        (PULONG)(Adapter->Registers + Adapter->SanityRegisterOffset),
        &value
        );

    Adapter->SanityRegisterValue = value;
    Adapter->SanityReadSucceeded = TRUE;
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
    Adapter->SanityRegisterOffset = 0;
    Adapter->SanityRegisterValue = 0;
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
    UNREFERENCED_PARAMETER(Adapter);

    if (Address == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressLength < L2_ETH_ADDR_LENGTH) {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    /*
     * Ultra-safe bring-up mode: avoid permanent MAC reads from hardware
     * registers until register-level safety is proven.
     */
    return NDIS_STATUS_NOT_SUPPORTED;
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
