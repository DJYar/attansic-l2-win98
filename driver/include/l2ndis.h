#ifndef _L2NDIS_H_
#define _L2NDIS_H_

#include <ndis.h>

#define L2_ETH_ADDR_LENGTH 6
#define L2_MAX_MULTICAST_ADDRESSES 32
#define L2_TX_PENDING_SLOTS 64

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
    ULONG CurrentPacketFilter;
    BOOLEAN PacketFilterSet;
    UCHAR MulticastAddresses[L2_MAX_MULTICAST_ADDRESSES][L2_ETH_ADDR_LENGTH];
    ULONG MulticastAddressCount;
    ULONG CurrentLookahead;
    ULONG MaximumLookahead;
    UCHAR PermanentMac[L2_ETH_ADDR_LENGTH];
    BOOLEAN MacReadSucceeded;
    ULONG SanityReadOffsets[3];
    ULONG SanityReadValues[3];
    ULONG SanityReadSuccessCount;
    BOOLEAN SanityReadSucceeded;
    BOOLEAN ResetAttempted;
    BOOLEAN ResetSucceeded;
    ULONG ResetIdleStatus;
    ULONG ResetPollCount;
    BOOLEAN PhyEnableAttempted;
    BOOLEAN PhyProbeSucceeded;
    USHORT PhyBmcr;
    USHORT PhyBmsr;
    USHORT PhyId1;
    USHORT PhyId2;
    BOOLEAN PhyConfigureSucceeded;
    USHORT PhyDebugData;
    USHORT PhyAdvertise;
    ULONG PhyStatusLogCountdown;
    USHORT PhyPssr;
    BOOLEAN PhyLinkUp;
    BOOLEAN PhyLinkResolved;
    BOOLEAN PhyFullDuplex;
    ULONG PhySpeedMbps;
    PVOID RingAllocation;
    NDIS_PHYSICAL_ADDRESS RingPhysicalAddress;
    ULONG RingAllocationLength;
    PUCHAR TxdRing;
    PUCHAR TxsRing;
    PUCHAR RxdRing;
    ULONG TxdPhysicalLow;
    ULONG TxdPhysicalHigh;
    ULONG TxsPhysicalLow;
    ULONG TxsPhysicalHigh;
    ULONG RxdPhysicalLow;
    ULONG RxdPhysicalHigh;
    BOOLEAN DmaMemoryAllocated;
    BOOLEAN DmaRingsProgrammed;
    BOOLEAN MacStaticConfigured;
    BOOLEAN DmaEnginesEnabled;
    ULONG DmaIdleStatus;
    ULONG DmaIdlePollCount;
    BOOLEAN TxMacEnabled;
    BOOLEAN TxSelfTestAttempted;
    BOOLEAN TxSelfTestSucceeded;
    ULONG TxSelfTestStatus;
    ULONG TxSelfTestPollCount;
    BOOLEAN RxMacEnabled;
    ULONG RxReadIndex;
    ULONG RxPacketsSeen;
    BOOLEAN RxFirstPacketLogged;
    ULONG TxWriteOffset;
    ULONG TxReadOffset;
    ULONG TxStatusIndex;
    ULONG TxStatusClearIndex;
    ULONG TxPendingCount;
    ULONG TxMaximumPendingCount;
    ULONG TxPacketsSent;
    ULONG TxPacketErrors;
    BOOLEAN TxPathFailed;
    PNDIS_PACKET PendingTxPackets[L2_TX_PENDING_SLOTS];
    UINT PendingTxPacketLengths[L2_TX_PENDING_SLOTS];
    ULONG PendingTxStartTicks[L2_TX_PENDING_SLOTS];
    BOOLEAN TxResourcesExhausted;
    ULONG TxInterruptCompletions;
    ULONG TxWatchdogCompletions;
    ULONG TxTimeouts;
    ULONG RxPacketsIndicated;
    ULONG RxPacketErrors;
    ULONG RxNoBuffer;
    NDIS_MINIPORT_TIMER PollTimer;
    BOOLEAN PollTimerInitialized;
    BOOLEAN PollTimerRunning;
    ULONG PollTimerTicks;
    ULONG PollRecoveryCount;
    ULONG PollRecoveredPackets;
    ULONG TelemetryLogCountdown;
    BOOLEAN RuntimeResetInProgress;
    ULONG RuntimeResetStage;
    ULONG RuntimeResetPollCount;
    ULONG RuntimeResetCount;
    ULONG RuntimeResetFailures;
    ULONG RuntimeResetSelfTestRequests;
    ULONG PhyLinkSelfTestStage;
    ULONG PhyLinkSelfTestStartTick;
    ULONG PhyLinkSelfTestSuccesses;
    ULONG PhyLinkSelfTestFailures;
    BOOLEAN ShutdownHandlerRegistered;
    BOOLEAN InterruptResourceFound;
    ULONG InterruptVector;
    ULONG InterruptLevel;
    NDIS_INTERRUPT_MODE InterruptMode;
    BOOLEAN InterruptShared;
    NDIS_MINIPORT_INTERRUPT Interrupt;
    BOOLEAN InterruptRegistered;
    BOOLEAN InterruptArmed;
    ULONG InterruptCount;
    ULONG InterruptDpcCount;
    ULONG LastInterruptStatus;
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

VOID
L2MiniportShutdown(
    IN PVOID ShutdownContext
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

VOID
L2MiniportPollTimer(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    );

VOID
L2MiniportDisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
L2MiniportEnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
L2MiniportHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
L2MiniportIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
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
