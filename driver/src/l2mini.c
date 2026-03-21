#include "l2ndis.h"

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

    UNREFERENCED_PARAMETER(WrapperConfigurationContext);

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

    NdisMSetAttributesEx(
        MiniportAdapterHandle,
        adapter,
        0,
        0,
        NdisInterfacePci
        );

    return NDIS_STATUS_SUCCESS;
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
    NDIS_HARDWARE_STATUS hardwareStatus = NdisHardwareStatusReady;
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
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(Oid);
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);

    *BytesRead = 0;
    *BytesNeeded = 0;

    return NDIS_STATUS_NOT_SUPPORTED;
}
