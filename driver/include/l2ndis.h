#ifndef _L2NDIS_H_
#define _L2NDIS_H_

#include <ndis.h>

#define L2_ETH_ADDR_LENGTH 6

typedef struct _L2_ADAPTER {
    NDIS_HANDLE AdapterHandle;
    BOOLEAN MediaConnected;
    PUCHAR Registers;
    ULONG MmioPhysicalBaseLow;
    ULONG MmioPhysicalBaseHigh;
    BOOLEAN MmioMappingSucceeded;
    ULONG ResourceCount;
    ULONG SelectedResourceIndex;
    ULONG SelectedResourceType;
    ULONG SelectedResourceLength;
    ULONG MemoryLength;
    BOOLEAN HardwareReady;
    UCHAR PermanentAddress[L2_ETH_ADDR_LENGTH];
    UCHAR CurrentAddress[L2_ETH_ADDR_LENGTH];
    ULONG LinkSpeed;
    ULONG MaximumFrameSize;
} L2_ADAPTER, *PL2_ADAPTER;

extern NDIS_HANDLE g_NdisWrapperHandle;

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
L2MiniportHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
L2MiniportInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    );


BOOLEAN
L2MiniportCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
L2MiniportReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
L2MiniportSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    );

VOID
L2MiniportReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet
    );

NDIS_STATUS
L2MiniportQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
L2MiniportSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

#endif /* _L2NDIS_H_ */
