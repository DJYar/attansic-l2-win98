#include "l2ndis.h"
#include "l2hw.h"

#define L2_RESOURCE_DESC_CAPACITY 16

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

    Adapter->IoBase = selectedAddress.LowPart;
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
        Adapter->IoBase = 0;
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
    Adapter->IoBase = 0;
    Adapter->HardwareReady = FALSE;
}

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    )
{
    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
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
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
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
    ULONG sanityReg;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    if (Adapter->Registers == NULL) {
        Adapter->HardwareReady = FALSE;
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    sanityReg = L2ReadReg32(Adapter, L2_REG_MASTER_CTRL);
    UNREFERENCED_PARAMETER(sanityReg);

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
