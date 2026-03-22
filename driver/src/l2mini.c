#include "l2ndis.h"
#include "l2hw.h"

#define L2_ETH_HEADER_SIZE 14
#define L2_DRIVER_VERSION 0x0500

static const UCHAR g_L2PlaceholderMac[L2_ETH_ADDR_LENGTH] = {
    0x02, 0x00, 0x00, 0x00, 0x00, 0x01
};

static const UCHAR g_L2VendorId[4] = {
    0x02, 0x00, 0x00, 0x00
};

static const UCHAR g_L2VendorDescription[] = "L2 NDIS5 phase-1 skeleton";

static const NDIS_OID g_L2SupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_LINK_SPEED,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_PERMANENT_ADDRESS
};

NDIS_STATUS
L2MiniportInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    )
{
    UINT i;
    PL2_ADAPTER adapter;

    *OpenErrorStatus = NDIS_STATUS_SUCCESS;

    for (i = 0; i < MediumArraySize; ++i) {
        if (MediumArray[i] == NdisMedium802_3) {
            *SelectedMediumIndex = i;
            break;
        }
    }

    if (i == MediumArraySize) {
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

    NdisAllocateMemoryWithTag((PVOID *)&adapter, sizeof(*adapter), '2dNL');
    if (adapter == NULL) {
        return NDIS_STATUS_RESOURCES;
    }

    NdisZeroMemory(adapter, sizeof(*adapter));
    adapter->AdapterHandle = MiniportAdapterHandle;
    adapter->MediaConnected = FALSE;

    NdisMoveMemory(adapter->PermanentAddress,
                   g_L2PlaceholderMac,
                   sizeof(adapter->PermanentAddress));

    NdisMoveMemory(adapter->CurrentAddress,
                   adapter->PermanentAddress,
                   sizeof(adapter->CurrentAddress));

    adapter->LinkSpeed = 100000;
    adapter->MaximumFrameSize = 1500;
    adapter->Registers = NULL;
    adapter->MmioPhysicalBaseLow = 0;
    adapter->MmioPhysicalBaseHigh = 0;
    adapter->MmioMappingSucceeded = FALSE;
    adapter->ResourceCount = 0;
    adapter->SelectedResourceIndex = 0xFFFFFFFF;
    adapter->SelectedResourceType = 0;
    adapter->SelectedResourceLength = 0;
    adapter->MemoryLength = 0;
    adapter->HardwareReady = FALSE;
    adapter->CurrentPacketFilter = 0;
    adapter->MaximumLookahead = adapter->MaximumFrameSize;
    adapter->CurrentLookahead = adapter->MaximumLookahead;

    NdisMSetAttributesEx(
        MiniportAdapterHandle,
        adapter,
        0,
        0,
        NdisInterfacePci
        );

#if L2_ENABLE_RESOURCE_DISCOVERY
    if (L2DiscoverAdapterResources(adapter, WrapperConfigurationContext) != NDIS_STATUS_SUCCESS) {
        NdisFreeMemory(adapter, sizeof(*adapter), 0);
        return NDIS_STATUS_FAILURE;
    }
#else
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
#endif

    return NDIS_STATUS_SUCCESS;
}

BOOLEAN
L2MiniportCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    /* Diagnostic bring-up stub: no hardware monitoring in this phase. */
    return FALSE;
}

NDIS_STATUS
L2MiniportReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    /* Diagnostic bring-up stub: no hardware reset is performed. */
    *AddressingReset = FALSE;
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2MiniportSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(Packet);
    UNREFERENCED_PARAMETER(Flags);

    /* Diagnostic bring-up stub: TX path not implemented yet. */
    return NDIS_STATUS_NOT_ACCEPTED;
}


VOID
L2MiniportHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if (adapter != NULL) {
        NdisFreeMemory(adapter, sizeof(*adapter), 0);
    }
}

NDIS_STATUS
L2MiniportQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    PVOID moveSource = NULL;
    ULONG moveBytes = 0;
    NDIS_HARDWARE_STATUS hardwareStatus = NdisHardwareStatusNotReady;
    NDIS_MEDIUM medium = NdisMedium802_3;
    NDIS_MEDIA_STATE mediaState = NdisMediaStateDisconnected;
    ULONG genericUlong = 0;
    USHORT genericUshort = 0;

    if (adapter == NULL) {
        return NDIS_STATUS_FAILURE;
    }

    *BytesWritten = 0;
    *BytesNeeded = 0;

    switch (Oid) {
    case OID_GEN_SUPPORTED_LIST:
        moveSource = (PVOID)g_L2SupportedOids;
        moveBytes = sizeof(g_L2SupportedOids);
        break;

    case OID_GEN_HARDWARE_STATUS:
        if (adapter->HardwareReady) {
            hardwareStatus = NdisHardwareStatusReady;
        }
        moveSource = &hardwareStatus;
        moveBytes = sizeof(hardwareStatus);
        break;

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:
        moveSource = &medium;
        moveBytes = sizeof(medium);
        break;

    case OID_GEN_MEDIA_CONNECT_STATUS:
        if (adapter->MediaConnected) {
            mediaState = NdisMediaStateConnected;
        }
        moveSource = &mediaState;
        moveBytes = sizeof(mediaState);
        break;

    case OID_GEN_CURRENT_LOOKAHEAD:
        moveSource = &adapter->CurrentLookahead;
        moveBytes = sizeof(adapter->CurrentLookahead);
        break;

    case OID_GEN_MAXIMUM_LOOKAHEAD:
        moveSource = &adapter->MaximumLookahead;
        moveBytes = sizeof(adapter->MaximumLookahead);
        break;

    case OID_GEN_MAC_OPTIONS:
        genericUlong = NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                       NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                       NDIS_MAC_OPTION_NO_LOOPBACK;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_GEN_TRANSMIT_BUFFER_SPACE:
    case OID_GEN_RECEIVE_BUFFER_SPACE:
        genericUlong = adapter->MaximumFrameSize;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_GEN_MAXIMUM_SEND_PACKETS:
        genericUlong = 1;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_GEN_LINK_SPEED:
        moveSource = &adapter->LinkSpeed;
        moveBytes = sizeof(adapter->LinkSpeed);
        break;

    case OID_GEN_MAXIMUM_FRAME_SIZE:
        moveSource = &adapter->MaximumFrameSize;
        moveBytes = sizeof(adapter->MaximumFrameSize);
        break;

    case OID_GEN_MAXIMUM_TOTAL_SIZE:
        genericUlong = adapter->MaximumFrameSize + L2_ETH_HEADER_SIZE;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_GEN_VENDOR_ID:
        moveSource = (PVOID)g_L2VendorId;
        moveBytes = sizeof(g_L2VendorId);
        break;

    case OID_GEN_VENDOR_DESCRIPTION:
        moveSource = (PVOID)g_L2VendorDescription;
        moveBytes = sizeof(g_L2VendorDescription);
        break;

    case OID_GEN_DRIVER_VERSION:
        genericUshort = L2_DRIVER_VERSION;
        moveSource = &genericUshort;
        moveBytes = sizeof(genericUshort);
        break;

    case OID_802_3_MAXIMUM_LIST_SIZE:
        genericUlong = 1;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_802_3_CURRENT_ADDRESS:
        moveSource = adapter->CurrentAddress;
        moveBytes = sizeof(adapter->CurrentAddress);
        break;

    case OID_802_3_PERMANENT_ADDRESS:
        moveSource = adapter->PermanentAddress;
        moveBytes = sizeof(adapter->PermanentAddress);
        break;

    default:
        status = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    if (status == NDIS_STATUS_SUCCESS) {
        if (InformationBufferLength < moveBytes) {
            *BytesNeeded = moveBytes;
            status = NDIS_STATUS_BUFFER_TOO_SHORT;
        } else {
            NdisMoveMemory(InformationBuffer, moveSource, moveBytes);
            *BytesWritten = moveBytes;
        }
    }

    return status;
}

NDIS_STATUS
L2MiniportSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if ((adapter == NULL) || (InformationBuffer == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    *BytesRead = 0;
    *BytesNeeded = 0;

    switch (Oid) {
    case OID_GEN_CURRENT_PACKET_FILTER:
        if (InformationBufferLength < sizeof(ULONG)) {
            *BytesNeeded = sizeof(ULONG);
            return NDIS_STATUS_INVALID_LENGTH;
        }

        adapter->CurrentPacketFilter = *(PULONG)InformationBuffer;
        *BytesRead = sizeof(ULONG);
        return NDIS_STATUS_SUCCESS;

    case OID_GEN_CURRENT_LOOKAHEAD:
        if (InformationBufferLength < sizeof(ULONG)) {
            *BytesNeeded = sizeof(ULONG);
            return NDIS_STATUS_INVALID_LENGTH;
        }

        adapter->CurrentLookahead = *(PULONG)InformationBuffer;
        if (adapter->CurrentLookahead > adapter->MaximumLookahead) {
            adapter->CurrentLookahead = adapter->MaximumLookahead;
        }

        *BytesRead = sizeof(ULONG);
        return NDIS_STATUS_SUCCESS;

    default:
        return NDIS_STATUS_NOT_SUPPORTED;
    }
}
