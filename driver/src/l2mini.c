#include "l2ndis.h"
#include "l2hw.h"
#include "debug.h"

#define L2_ETH_HEADER_SIZE 14
#define L2_DRIVER_VERSION 0x0500
#define L2_RESET_STAGE_IDLE 0
#define L2_RESET_STAGE_WAIT_IDLE 1
#define L2_RESET_STAGE_WAIT_LINK 2
#define L2_RESET_IDLE_POLL_LIMIT 8
#define L2_RESET_LINK_POLL_LIMIT 20
#define L2_ENABLE_RESET_SELFTEST 0
#define L2_RESET_SELFTEST_INITIAL_TICKS 80
#define L2_RESET_SELFTEST_INTERVAL_TICKS 40
#define L2_RESET_SELFTEST_LIMIT 20
#define L2_ENABLE_PHY_LINK_SELFTEST 0
#define L2_PHY_LINK_SELFTEST_STAGE_IDLE 0
#define L2_PHY_LINK_SELFTEST_STAGE_WAIT_DOWN 1
#define L2_PHY_LINK_SELFTEST_STAGE_WAIT_UP 2
#define L2_PHY_LINK_SELFTEST_STAGE_COMPLETE 3
#define L2_PHY_LINK_SELFTEST_TRIGGER_TICKS 80
#define L2_PHY_LINK_SELFTEST_TIMEOUT_TICKS 40
#if L2_TX_PENDING_SLOTS != L2_TXS_RING_COUNT
#error TX packet bookkeeping must match the hardware status ring
#endif
#define L2_SUPPORTED_PACKET_FILTER \
    (NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST | \
     NDIS_PACKET_TYPE_BROADCAST)

static const UCHAR g_L2PlaceholderMac[L2_ETH_ADDR_LENGTH] = {
    0x02, 0x00, 0x00, 0x00, 0x00, 0x01
};

static const UCHAR g_L2VendorDescription[] =
    "Attansic L2 Fast Ethernet Adapter";

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
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_MULTICAST_LIST,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_PERMANENT_ADDRESS
};

static ULONG
L2EthernetCrcLe(
    IN const UCHAR *Address
    )
{
    ULONG crc = 0xFFFFFFFF;
    UINT byteIndex;

    for (byteIndex = 0; byteIndex < L2_ETH_ADDR_LENGTH; byteIndex++) {
        UCHAR current = Address[byteIndex];
        UINT bitIndex;

        for (bitIndex = 0; bitIndex < 8; bitIndex++) {
            ULONG carry = (crc ^ current) & 1;

            crc >>= 1;
            if (carry != 0) {
                crc ^= 0xEDB88320;
            }
            current >>= 1;
        }
    }

    return crc;
}

static ULONG
L2ReverseBits32(
    IN ULONG Value
    )
{
    ULONG reversed = 0;
    UINT bitIndex;

    for (bitIndex = 0; bitIndex < 32; bitIndex++) {
        reversed |= ((Value >> bitIndex) & 1) << (31 - bitIndex);
    }

    return reversed;
}

static NDIS_STATUS
L2ApplyReceiveFilter(
    IN PL2_ADAPTER Adapter
    )
{
    ULONG hash[2];
    ULONG control;
    ULONG index;

    if (!Adapter->RxMacEnabled ||
        !Adapter->MacStaticConfigured ||
        (Adapter->Registers == NULL)) {
        return NDIS_STATUS_SUCCESS;
    }

    hash[0] = 0;
    hash[1] = 0;
    if ((Adapter->CurrentPacketFilter & NDIS_PACKET_TYPE_MULTICAST) != 0) {
        for (index = 0; index < Adapter->MulticastAddressCount; index++) {
            ULONG value = L2ReverseBits32(
                L2EthernetCrcLe(Adapter->MulticastAddresses[index]));
            ULONG hashRegister = (value >> 31) & 1;
            ULONG hashBit = (value >> 26) & 0x1F;

            hash[hashRegister] |= ((ULONG)1 << hashBit);
        }
    }

    control = L2ReadReg32(Adapter, L2_REG_MAC_CTRL);
    control &= ~(L2_MAC_CTRL_PROMISCUOUS |
                 L2_MAC_CTRL_ALL_MULTICAST |
                 L2_MAC_CTRL_BROADCAST);
    if ((Adapter->CurrentPacketFilter & NDIS_PACKET_TYPE_BROADCAST) != 0) {
        control |= L2_MAC_CTRL_BROADCAST;
    }

    L2WriteReg32(Adapter, L2_REG_RX_HASH_0, hash[0]);
    L2WriteReg32(Adapter, L2_REG_RX_HASH_1, hash[1]);
    L2WriteReg32(Adapter, L2_REG_MAC_CTRL, control);

    if ((L2ReadReg32(Adapter, L2_REG_RX_HASH_0) != hash[0]) ||
        (L2ReadReg32(Adapter, L2_REG_RX_HASH_1) != hash[1]) ||
        (L2ReadReg32(Adapter, L2_REG_MAC_CTRL) != control)) {
        DBGPRINT(("[L2] receive filter verification failed\n"));
        return NDIS_STATUS_FAILURE;
    }

    DBGPRINT(("[L2] receive filter applied: packet=%08X multicast=%lu hash=%08X:%08X ctrl=%08X\n",
              Adapter->CurrentPacketFilter,
              Adapter->MulticastAddressCount,
              hash[1],
              hash[0],
              control));
    return NDIS_STATUS_SUCCESS;
}

static BOOLEAN
L2CopyPacketToTxRing(
    IN PL2_ADAPTER Adapter,
    IN PNDIS_PACKET Packet,
    IN ULONG DataOffset,
    IN UINT PacketLength
    )
{
    PNDIS_BUFFER buffer;
    PVOID virtualAddress;
    UINT bufferLength;
    UINT copied = 0;
    ULONG ringOffset = DataOffset;

    NdisQueryPacket(Packet, NULL, NULL, &buffer, NULL);
    while ((buffer != NULL) && (copied < PacketLength)) {
        PUCHAR source;
        UINT remaining;

        NdisQueryBuffer(buffer, &virtualAddress, &bufferLength);
        source = (PUCHAR)virtualAddress;
        remaining = bufferLength;

        while ((remaining != 0) && (copied < PacketLength)) {
            ULONG ringBytes = L2_TXD_RING_BYTES - ringOffset;
            UINT packetBytes = PacketLength - copied;
            UINT amount = remaining;

            if (amount > packetBytes) {
                amount = packetBytes;
            }
            if ((ULONG)amount > ringBytes) {
                amount = (UINT)ringBytes;
            }

            NdisMoveMemory(Adapter->TxdRing + ringOffset, source, amount);
            source += amount;
            remaining -= amount;
            copied += amount;
            ringOffset += amount;
            if (ringOffset == L2_TXD_RING_BYTES) {
                ringOffset = 0;
            }
        }

        NdisGetNextBuffer(buffer, &buffer);
    }

    return (copied == PacketLength) ? TRUE : FALSE;
}

static ULONG
L2DrainPendingTx(
    IN PL2_ADAPTER Adapter,
    IN BOOLEAN FromWatchdog
    )
{
#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    ULONG completed = 0;

    if ((Adapter == NULL) ||
        (Adapter->TxPendingCount == 0) ||
        (Adapter->TxsRing == NULL)) {
        return 0;
    }

    while (Adapter->TxPendingCount != 0) {
        ULONG slot = Adapter->TxStatusIndex;
        volatile ULONG *txStatus = (volatile ULONG *)(Adapter->TxsRing +
                                   (slot * L2_TXS_ENTRY_BYTES));
        ULONG status = *txStatus;
        PNDIS_PACKET packet;
        UINT packetLength;
        NDIS_STATUS completionStatus;
        ULONG consumedBytes;

        if ((status & L2_TX_STATUS_UPDATE) == 0) {
            break;
        }

        packet = Adapter->PendingTxPackets[slot];
        packetLength = Adapter->PendingTxPacketLengths[slot];
        if (packet == NULL) {
            Adapter->TxPathFailed = TRUE;
            DBGPRINT(("[L2] TX status without queued packet: slot=%lu status=%08X\n",
                      slot,
                      status));
            break;
        }

        Adapter->PendingTxPackets[slot] = NULL;
        Adapter->PendingTxPacketLengths[slot] = 0;
        Adapter->PendingTxStartTicks[slot] = 0;
        completionStatus = NDIS_STATUS_SUCCESS;
        if (((status & L2_TX_STATUS_OK) == 0) ||
            ((status & L2_TX_STATUS_SIZE_MASK) != packetLength)) {
            completionStatus = NDIS_STATUS_FAILURE;
            ++Adapter->TxPacketErrors;
        } else {
            ++Adapter->TxPacketsSent;
        }

        *txStatus = 0;
        ++Adapter->TxStatusIndex;
        if (Adapter->TxStatusIndex == L2_TXS_RING_COUNT) {
            Adapter->TxStatusIndex = 0;
        }
        --Adapter->TxPendingCount;

        consumedBytes = sizeof(ULONG) + (((ULONG)packetLength + 3) & ~3UL);
        Adapter->TxReadOffset += consumedBytes;
        if (Adapter->TxReadOffset >= L2_TXD_RING_BYTES) {
            Adapter->TxReadOffset -= L2_TXD_RING_BYTES;
        }

        if (FromWatchdog) {
            ++Adapter->TxWatchdogCompletions;
        } else {
            ++Adapter->TxInterruptCompletions;
        }

        if ((Adapter->TxPacketsSent + Adapter->TxPacketErrors) <= 8) {
            DBGPRINT(("[L2] NDIS TX complete: len=%u status=%08X source=%s pending=%lu\n",
                      packetLength,
                      status,
                      FromWatchdog ? "watchdog" : "irq",
                      Adapter->TxPendingCount));
        }

        ++completed;
        NdisMSendComplete(Adapter->AdapterHandle, packet, completionStatus);
    }

    if ((completed != 0) && Adapter->TxResourcesExhausted &&
        !Adapter->TxPathFailed) {
        Adapter->TxResourcesExhausted = FALSE;
        NdisMSendResourcesAvailable(Adapter->AdapterHandle);
    }
    return completed;
#else
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(FromWatchdog);
    return 0;
#endif
}

static VOID
L2FailPendingTx(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    while ((Adapter != NULL) && (Adapter->TxPendingCount != 0)) {
        ULONG slot = Adapter->TxStatusIndex;
        PNDIS_PACKET packet = Adapter->PendingTxPackets[slot];

        Adapter->PendingTxPackets[slot] = NULL;
        Adapter->PendingTxPacketLengths[slot] = 0;
        Adapter->PendingTxStartTicks[slot] = 0;
        if (Adapter->TxsRing != NULL) {
            *(volatile ULONG *)(Adapter->TxsRing +
              (slot * L2_TXS_ENTRY_BYTES)) = 0;
        }
        ++Adapter->TxStatusIndex;
        if (Adapter->TxStatusIndex == L2_TXS_RING_COUNT) {
            Adapter->TxStatusIndex = 0;
        }
        --Adapter->TxPendingCount;
        ++Adapter->TxPacketErrors;
        if (packet != NULL) {
            NdisMSendComplete(Adapter->AdapterHandle,
                              packet,
                              NDIS_STATUS_FAILURE);
        }
    }
    Adapter->TxResourcesExhausted = FALSE;
#else
    UNREFERENCED_PARAMETER(Adapter);
#endif
}

static VOID
L2RegisterMaskedInterrupt(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_IRQ_REGISTRATION) && L2_ENABLE_IRQ_REGISTRATION
    NDIS_STATUS status;
    ULONG imr;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !Adapter->InterruptResourceFound) {
        DBGPRINT(("[L2] IRQ registration skipped: no interrupt resource\n"));
        return;
    }

    L2WriteReg32(Adapter, L2_REG_IMR, 0);
    imr = L2ReadReg32(Adapter, L2_REG_IMR);
    if (imr != 0) {
        DBGPRINT(("[L2] IRQ registration skipped: IMR did not clear (%08X)\n",
                  imr));
        return;
    }

    status = NdisMRegisterInterrupt(
        &Adapter->Interrupt,
        Adapter->AdapterHandle,
        Adapter->InterruptVector,
        Adapter->InterruptLevel,
        TRUE,
        Adapter->InterruptShared,
        Adapter->InterruptMode);
    if (status != NDIS_STATUS_SUCCESS) {
        DBGPRINT(("[L2] IRQ registration failed: status=%08X vector=%lu level=%lu\n",
                  status,
                  Adapter->InterruptVector,
                  Adapter->InterruptLevel));
        return;
    }

    Adapter->InterruptRegistered = TRUE;
    DBGPRINT(("[L2] IRQ registered masked: vector=%lu level=%lu shared=%lu mode=%s imr=%08X\n",
              Adapter->InterruptVector,
              Adapter->InterruptLevel,
              (ULONG)Adapter->InterruptShared,
              (Adapter->InterruptMode == NdisInterruptLatched) ?
                  "latched" : "level",
              imr));
#else
    UNREFERENCED_PARAMETER(Adapter);
#endif
}

static VOID
L2ArmDatapathInterrupt(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_RX_INTERRUPT) && L2_ENABLE_RX_INTERRUPT
    ULONG imr;

    if ((Adapter == NULL) ||
        !Adapter->InterruptRegistered ||
        !Adapter->HardwareReady ||
        Adapter->InterruptArmed) {
        return;
    }

    /* Clear stale status before the RX/TX completion sources are unmasked. */
    L2WriteReg32(Adapter, L2_REG_ISR, L2_ISR_CLEARABLE_MASK);
    L2WriteReg32(Adapter, L2_REG_ISR, 0);
    Adapter->InterruptArmed = TRUE;
    L2WriteReg32(Adapter, L2_REG_IMR, L2_IMR_DATAPATH);
    imr = L2ReadReg32(Adapter, L2_REG_IMR);
    if (imr != L2_IMR_DATAPATH) {
        Adapter->InterruptArmed = FALSE;
        L2WriteReg32(Adapter, L2_REG_IMR, 0);
        DBGPRINT(("[L2] RX/TX IRQ arm failed: imr=%08X\n", imr));
        return;
    }

    DBGPRINT(("[L2] RX/TX IRQ armed: imr=%08X timer remains fallback\n", imr));
#else
    UNREFERENCED_PARAMETER(Adapter);
#endif
}

static VOID
L2FinishRuntimeReset(
    IN PL2_ADAPTER Adapter,
    IN NDIS_STATUS Status
    )
{
    Adapter->RuntimeResetInProgress = FALSE;
    Adapter->RuntimeResetStage = L2_RESET_STAGE_IDLE;
    Adapter->RuntimeResetPollCount = 0;
    if (Status == NDIS_STATUS_SUCCESS) {
        ++Adapter->RuntimeResetCount;
    } else {
        ++Adapter->RuntimeResetFailures;
    }

    DBGPRINT(("[L2] runtime reset complete: status=%08X ready=%lu media=%lu resets=%lu failures=%lu\n",
              Status,
              (ULONG)Adapter->HardwareReady,
              (ULONG)Adapter->MediaConnected,
              Adapter->RuntimeResetCount,
              Adapter->RuntimeResetFailures));
    NdisMResetComplete(Adapter->AdapterHandle, Status, FALSE);
}

static VOID
L2ContinueRuntimeReset(
    IN PL2_ADAPTER Adapter
    )
{
    NDIS_STATUS status;

    if ((Adapter == NULL) || !Adapter->RuntimeResetInProgress) {
        return;
    }

    if (Adapter->RuntimeResetStage == L2_RESET_STAGE_WAIT_IDLE) {
        Adapter->ResetIdleStatus =
            L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);
        ++Adapter->RuntimeResetPollCount;
        Adapter->ResetPollCount = Adapter->RuntimeResetPollCount;
        if (Adapter->ResetIdleStatus != 0) {
            if (Adapter->RuntimeResetPollCount >= L2_RESET_IDLE_POLL_LIMIT) {
                DBGPRINT(("[L2] runtime reset idle timeout: status=%08X polls=%lu\n",
                          Adapter->ResetIdleStatus,
                          Adapter->RuntimeResetPollCount));
                L2FinishRuntimeReset(Adapter, NDIS_STATUS_HARD_ERRORS);
            }
            return;
        }

        Adapter->ResetSucceeded = TRUE;
        NdisZeroMemory(Adapter->RingAllocation,
                       Adapter->RingAllocationLength);
        NdisMUpdateSharedMemory(Adapter->AdapterHandle,
                                Adapter->RingAllocationLength,
                                Adapter->RingAllocation,
                                Adapter->RingPhysicalAddress);
        Adapter->DmaRingsProgrammed = FALSE;
        Adapter->MacStaticConfigured = FALSE;
        Adapter->DmaEnginesEnabled = FALSE;
        Adapter->TxMacEnabled = FALSE;
        Adapter->TxSelfTestAttempted = FALSE;
        Adapter->TxSelfTestSucceeded = FALSE;
        Adapter->RxMacEnabled = FALSE;
        Adapter->RxReadIndex = 0;
        Adapter->TxWriteOffset = 0;
        Adapter->TxReadOffset = 0;
        Adapter->TxStatusIndex = 0;
        Adapter->TxStatusClearIndex = 0;
        Adapter->TxPendingCount = 0;
        Adapter->TxResourcesExhausted = FALSE;
        Adapter->TxPathFailed = FALSE;
        Adapter->LastInterruptStatus = 0;
        NdisZeroMemory(Adapter->PendingTxPackets,
                       sizeof(Adapter->PendingTxPackets));
        NdisZeroMemory(Adapter->PendingTxPacketLengths,
                       sizeof(Adapter->PendingTxPacketLengths));
        NdisZeroMemory(Adapter->PendingTxStartTicks,
                       sizeof(Adapter->PendingTxStartTicks));

        status = L2EnablePhy(Adapter);
        if (status == NDIS_STATUS_SUCCESS) {
            status = L2ConfigurePhy(Adapter);
        }
        if (status == NDIS_STATUS_SUCCESS) {
            status = L2ProbePhy(Adapter);
        }
        if (status == NDIS_STATUS_SUCCESS) {
            status = L2ProgramDmaRings(Adapter);
        }
        if (status == NDIS_STATUS_SUCCESS) {
            status = L2ConfigureMacStatic(Adapter);
        }
        if (status == NDIS_STATUS_SUCCESS) {
            status = L2EnableDmaEnginesDiagnostic(Adapter);
        }
        if (status != NDIS_STATUS_SUCCESS) {
            DBGPRINT(("[L2] runtime reset reinitialization failed: status=%08X\n",
                      status));
            L2FinishRuntimeReset(Adapter, NDIS_STATUS_HARD_ERRORS);
            return;
        }

        Adapter->RuntimeResetStage = L2_RESET_STAGE_WAIT_LINK;
        Adapter->RuntimeResetPollCount = 0;
        DBGPRINT(("[L2] runtime reset hardware restored; waiting for link\n"));
        return;
    }

    if (Adapter->RuntimeResetStage == L2_RESET_STAGE_WAIT_LINK) {
        ++Adapter->RuntimeResetPollCount;
        status = L2UpdatePhyLinkState(Adapter);
        if ((status == NDIS_STATUS_SUCCESS) &&
            Adapter->PhyLinkUp && Adapter->PhyLinkResolved) {
            status = L2EnableTxMacDiagnostic(Adapter);
            if (status == NDIS_STATUS_SUCCESS) {
                status = L2RunTxSelfTest(Adapter);
            }
            if (status == NDIS_STATUS_SUCCESS) {
                status = L2EnableRxMacDiagnostic(Adapter);
            }
            if ((status == NDIS_STATUS_SUCCESS) && Adapter->PacketFilterSet) {
                status = L2ApplyReceiveFilter(Adapter);
            }
            if (status != NDIS_STATUS_SUCCESS) {
                DBGPRINT(("[L2] runtime reset datapath restore failed: status=%08X\n",
                          status));
                L2FinishRuntimeReset(Adapter, NDIS_STATUS_HARD_ERRORS);
                return;
            }

            Adapter->HardwareReady = TRUE;
            Adapter->MediaConnected = TRUE;
            Adapter->TelemetryLogCountdown = 0;
            L2ArmDatapathInterrupt(Adapter);
            L2FinishRuntimeReset(Adapter, NDIS_STATUS_SUCCESS);
            return;
        }

        if (Adapter->RuntimeResetPollCount >= L2_RESET_LINK_POLL_LIMIT) {
            Adapter->HardwareReady = TRUE;
            Adapter->MediaConnected = FALSE;
            Adapter->TelemetryLogCountdown = 0;
            DBGPRINT(("[L2] runtime reset completed with media disconnected\n"));
            L2FinishRuntimeReset(Adapter, NDIS_STATUS_SUCCESS);
        }
    }
}

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
    adapter->PacketFilterSet = FALSE;
    adapter->MulticastAddressCount = 0;
    adapter->MaximumLookahead = adapter->MaximumFrameSize;
    adapter->CurrentLookahead = adapter->MaximumLookahead;
    adapter->PermanentMac[0] = 0;
    adapter->PermanentMac[1] = 0;
    adapter->PermanentMac[2] = 0;
    adapter->PermanentMac[3] = 0;
    adapter->PermanentMac[4] = 0;
    adapter->PermanentMac[5] = 0;
    adapter->MacReadSucceeded = FALSE;
    adapter->SanityReadOffsets[0] = 0;
    adapter->SanityReadOffsets[1] = 0;
    adapter->SanityReadOffsets[2] = 0;
    adapter->SanityReadValues[0] = 0;
    adapter->SanityReadValues[1] = 0;
    adapter->SanityReadValues[2] = 0;
    adapter->SanityReadSuccessCount = 0;
    adapter->SanityReadSucceeded = FALSE;
    adapter->ResetAttempted = FALSE;
    adapter->ResetSucceeded = FALSE;
    adapter->ResetIdleStatus = 0;
    adapter->ResetPollCount = 0;
    adapter->PhyEnableAttempted = FALSE;
    adapter->PhyProbeSucceeded = FALSE;
    adapter->PhyBmcr = 0;
    adapter->PhyBmsr = 0;
    adapter->PhyId1 = 0;
    adapter->PhyId2 = 0;
    adapter->PhyConfigureSucceeded = FALSE;
    adapter->PhyDebugData = 0;
    adapter->PhyAdvertise = 0;
    adapter->PhyStatusLogCountdown = 0;
    adapter->PhyPssr = 0;
    adapter->PhyLinkUp = FALSE;
    adapter->PhyLinkResolved = FALSE;
    adapter->PhyFullDuplex = FALSE;
    adapter->PhySpeedMbps = 0;
    adapter->RingAllocation = NULL;
    NdisZeroMemory(&adapter->RingPhysicalAddress,
                   sizeof(adapter->RingPhysicalAddress));
    adapter->RingAllocationLength = 0;
    adapter->TxdRing = NULL;
    adapter->TxsRing = NULL;
    adapter->RxdRing = NULL;
    adapter->TxdPhysicalLow = 0;
    adapter->TxdPhysicalHigh = 0;
    adapter->TxsPhysicalLow = 0;
    adapter->TxsPhysicalHigh = 0;
    adapter->RxdPhysicalLow = 0;
    adapter->RxdPhysicalHigh = 0;
    adapter->DmaMemoryAllocated = FALSE;
    adapter->DmaRingsProgrammed = FALSE;
    adapter->MacStaticConfigured = FALSE;
    adapter->DmaEnginesEnabled = FALSE;
    adapter->DmaIdleStatus = 0;
    adapter->DmaIdlePollCount = 0;
    adapter->TxMacEnabled = FALSE;
    adapter->TxSelfTestAttempted = FALSE;
    adapter->TxSelfTestSucceeded = FALSE;
    adapter->TxSelfTestStatus = 0;
    adapter->TxSelfTestPollCount = 0;
    adapter->RxMacEnabled = FALSE;
    adapter->RxReadIndex = 0;
    adapter->RxPacketsSeen = 0;
    adapter->RxFirstPacketLogged = FALSE;
    adapter->TxWriteOffset = 0;
    adapter->TxReadOffset = 0;
    adapter->TxStatusIndex = 0;
    adapter->TxStatusClearIndex = 0;
    adapter->TxPendingCount = 0;
    adapter->TxMaximumPendingCount = 0;
    adapter->TxPacketsSent = 0;
    adapter->TxPacketErrors = 0;
    adapter->TxPathFailed = FALSE;
    adapter->TxResourcesExhausted = FALSE;
    adapter->TxInterruptCompletions = 0;
    adapter->TxWatchdogCompletions = 0;
    adapter->TxTimeouts = 0;
    adapter->RxPacketsIndicated = 0;
    adapter->RxPacketErrors = 0;
    adapter->RxNoBuffer = 0;
    adapter->PollTimerInitialized = FALSE;
    adapter->PollTimerRunning = FALSE;
    adapter->PollTimerTicks = 0;
    adapter->PollRecoveryCount = 0;
    adapter->PollRecoveredPackets = 0;
    adapter->TelemetryLogCountdown = 0;
    adapter->RuntimeResetInProgress = FALSE;
    adapter->RuntimeResetStage = L2_RESET_STAGE_IDLE;
    adapter->RuntimeResetPollCount = 0;
    adapter->RuntimeResetCount = 0;
    adapter->RuntimeResetFailures = 0;
    adapter->RuntimeResetSelfTestRequests = 0;
    adapter->PhyLinkSelfTestStage = L2_PHY_LINK_SELFTEST_STAGE_IDLE;
    adapter->PhyLinkSelfTestStartTick = 0;
    adapter->PhyLinkSelfTestSuccesses = 0;
    adapter->PhyLinkSelfTestFailures = 0;
    adapter->ShutdownHandlerRegistered = FALSE;
    adapter->InterruptResourceFound = FALSE;
    adapter->InterruptVector = 0;
    adapter->InterruptLevel = 0;
    adapter->InterruptMode = NdisInterruptLevelSensitive;
    adapter->InterruptShared = FALSE;
    adapter->InterruptRegistered = FALSE;
    adapter->InterruptArmed = FALSE;
    adapter->InterruptCount = 0;
    adapter->InterruptDpcCount = 0;
    adapter->LastInterruptStatus = 0;

    NdisMSetAttributesEx(
        MiniportAdapterHandle,
        adapter,
        0,
        0,
        NdisInterfacePci
        );

    NdisMInitializeTimer(&adapter->PollTimer,
                          adapter->AdapterHandle,
                          L2MiniportPollTimer,
                          adapter);
    adapter->PollTimerInitialized = TRUE;

#if L2_ENABLE_RESOURCE_DISCOVERY
    if (L2DiscoverAdapterResources(adapter, WrapperConfigurationContext) != NDIS_STATUS_SUCCESS) {
        NdisFreeMemory(adapter, sizeof(*adapter), 0);
        return NDIS_STATUS_FAILURE;
    }

#if L2_ENABLE_MMIO_MAPPING
    if (L2MapDiscoveredMmio(adapter) != NDIS_STATUS_SUCCESS) {
        NdisFreeMemory(adapter, sizeof(*adapter), 0);
        return NDIS_STATUS_FAILURE;
    }

    /* Fixed-set read-only sanity accesses after successful MMIO mapping. */
    L2PerformMmioSanityRead(adapter);

    /*
     * Read and validate the station address without modifying hardware.  Once
     * validated, expose it through the standard Ethernet address OIDs.
     */
    if (L2ReadPermanentMac(adapter,
                           adapter->PermanentMac,
                           sizeof(adapter->PermanentMac)) == NDIS_STATUS_SUCCESS) {
        NdisMoveMemory(adapter->PermanentAddress,
                       adapter->PermanentMac,
                       sizeof(adapter->PermanentAddress));
        NdisMoveMemory(adapter->CurrentAddress,
                       adapter->PermanentMac,
                       sizeof(adapter->CurrentAddress));
    }

#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    (VOID)L2EnablePhy(adapter);
#endif

#if defined(L2_ENABLE_PHY_CONFIG) && L2_ENABLE_PHY_CONFIG
    (VOID)L2ConfigurePhy(adapter);
#endif

#if defined(L2_ENABLE_SOFT_RESET) && L2_ENABLE_SOFT_RESET
    /* Diagnostic stage: one reset write followed by a strictly bounded poll. */
    (VOID)L2HwReset(adapter);
#endif

#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    (VOID)L2ProbePhy(adapter);
#endif

    /* Allocate and validate ring layout; no DMA registers are written yet. */
    if (L2AllocateDmaMemory(adapter) == NDIS_STATUS_SUCCESS) {
        if (L2ProgramDmaRings(adapter) == NDIS_STATUS_SUCCESS) {
            if (L2ConfigureMacStatic(adapter) == NDIS_STATUS_SUCCESS) {
                (VOID)L2EnableDmaEnginesDiagnostic(adapter);
            }
        }
    }

    L2RegisterMaskedInterrupt(adapter);
#endif
#else
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
#endif

    NdisMRegisterAdapterShutdownHandler(adapter->AdapterHandle,
                                         adapter,
                                         L2MiniportShutdown);
    adapter->ShutdownHandlerRegistered = TRUE;
    DBGPRINT(("[L2] adapter shutdown handler registered\n"));

    return NDIS_STATUS_SUCCESS;
}

BOOLEAN
L2MiniportCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;
#if L2_ENABLE_PHY_LINK_SELFTEST
    NDIS_STATUS phyTestStatus;
#endif

    if ((adapter != NULL) && adapter->RuntimeResetInProgress) {
        return FALSE;
    }

    if ((adapter != NULL) && adapter->PhyConfigureSucceeded) {
        if (L2UpdatePhyLinkState(adapter) == NDIS_STATUS_SUCCESS) {
            if (adapter->PhyLinkResolved &&
                adapter->DmaEnginesEnabled &&
                !adapter->TxMacEnabled) {
                (VOID)L2EnableTxMacDiagnostic(adapter);
            }
            if (adapter->TxMacEnabled &&
                !adapter->TxSelfTestAttempted) {
                (VOID)L2RunTxSelfTest(adapter);
            }
            if (adapter->TxSelfTestSucceeded &&
                !adapter->RxMacEnabled) {
                (VOID)L2EnableRxMacDiagnostic(adapter);
            }
            if (adapter->RxMacEnabled &&
                !adapter->HardwareReady) {
                if (adapter->PacketFilterSet &&
                    (L2ApplyReceiveFilter(adapter) != NDIS_STATUS_SUCCESS)) {
                    return FALSE;
                }
                adapter->HardwareReady = TRUE;
                adapter->MediaConnected = TRUE;
                NdisMIndicateStatus(adapter->AdapterHandle,
                                     NDIS_STATUS_MEDIA_CONNECT,
                                     NULL,
                                     0);
                NdisMIndicateStatusComplete(adapter->AdapterHandle);
                adapter->PollTimerRunning = TRUE;
                NdisMSetPeriodicTimer(&adapter->PollTimer, 250);
                DBGPRINT(("[L2] NDIS datapath ready: media connected\n"));
                DBGPRINT(("[L2] RX watchdog timer started: period=250 ms\n"));
            }
            if (adapter->HardwareReady &&
                adapter->RxMacEnabled &&
                adapter->InterruptRegistered &&
                !adapter->InterruptArmed) {
                L2ArmDatapathInterrupt(adapter);
            }
            if (adapter->HardwareReady &&
                adapter->MediaConnected &&
                (!adapter->PhyLinkUp || !adapter->PhyLinkResolved)) {
                adapter->MediaConnected = FALSE;
                NdisMIndicateStatus(adapter->AdapterHandle,
                                     NDIS_STATUS_MEDIA_DISCONNECT,
                                     NULL,
                                     0);
                NdisMIndicateStatusComplete(adapter->AdapterHandle);
                DBGPRINT(("[L2] NDIS media disconnected\n"));
            } else if (adapter->HardwareReady &&
                       !adapter->MediaConnected &&
                       adapter->PhyLinkUp &&
                       adapter->PhyLinkResolved) {
                adapter->MediaConnected = TRUE;
                NdisMIndicateStatus(adapter->AdapterHandle,
                                     NDIS_STATUS_MEDIA_CONNECT,
                                     NULL,
                                     0);
                NdisMIndicateStatusComplete(adapter->AdapterHandle);
                DBGPRINT(("[L2] NDIS media connected: speed=%lu duplex=%s\n",
                          adapter->PhySpeedMbps,
                          adapter->PhyFullDuplex ? "full" : "half"));
            }
            if (adapter->RxMacEnabled &&
                !adapter->PollTimerRunning) {
                L2PollRxDiagnostic(adapter);
            }
            if (adapter->PhyStatusLogCountdown == 0) {
                DBGPRINT(("[L2] PHY status: bmsr=0x%04X pssr=0x%04X link=%lu autoneg=%lu resolved=%lu speed=%lu duplex=%s\n",
                          adapter->PhyBmsr,
                          adapter->PhyPssr,
                          (ULONG)adapter->PhyLinkUp,
                          (ULONG)((adapter->PhyBmsr & L2_MII_BMSR_AN_COMPLETE) != 0),
                          (ULONG)adapter->PhyLinkResolved,
                          adapter->PhySpeedMbps,
                          adapter->PhyFullDuplex ? "full" : "half"));
                adapter->PhyStatusLogCountdown = 5;
            } else {
                --adapter->PhyStatusLogCountdown;
            }
        }
    }

#if L2_ENABLE_RESET_SELFTEST
    if ((adapter != NULL) && adapter->HardwareReady &&
        (adapter->RuntimeResetSelfTestRequests < L2_RESET_SELFTEST_LIMIT) &&
        (adapter->PollTimerTicks >=
         (L2_RESET_SELFTEST_INITIAL_TICKS +
          (adapter->RuntimeResetSelfTestRequests *
           L2_RESET_SELFTEST_INTERVAL_TICKS)))) {
        ++adapter->RuntimeResetSelfTestRequests;
        DBGPRINT(("[L2] requesting runtime reset self-test %lu/%lu\n",
                  adapter->RuntimeResetSelfTestRequests,
                  (ULONG)L2_RESET_SELFTEST_LIMIT));
        return TRUE;
    }
#endif

#if L2_ENABLE_PHY_LINK_SELFTEST
    if ((adapter != NULL) && adapter->HardwareReady &&
        (adapter->PhyLinkSelfTestStage ==
         L2_PHY_LINK_SELFTEST_STAGE_IDLE) &&
        (adapter->PollTimerTicks >= L2_PHY_LINK_SELFTEST_TRIGGER_TICKS)) {
        phyTestStatus = L2ForcePhyPowerSaving(adapter);
        if (phyTestStatus == NDIS_STATUS_SUCCESS) {
            adapter->PhyLinkSelfTestStage =
                L2_PHY_LINK_SELFTEST_STAGE_WAIT_DOWN;
            adapter->PhyLinkSelfTestStartTick = adapter->PollTimerTicks;
            DBGPRINT(("[L2] PHY link self-test: force power-saving requested\n"));
        } else {
            ++adapter->PhyLinkSelfTestFailures;
            adapter->PhyLinkSelfTestStage =
                L2_PHY_LINK_SELFTEST_STAGE_COMPLETE;
            DBGPRINT(("[L2] PHY link self-test: force power-saving failed status=%08X\n",
                      phyTestStatus));
        }
    } else if ((adapter != NULL) &&
               (adapter->PhyLinkSelfTestStage ==
                L2_PHY_LINK_SELFTEST_STAGE_WAIT_DOWN)) {
        if (!adapter->PhyLinkUp) {
            DBGPRINT(("[L2] PHY link self-test: disconnect observed; restarting autoneg\n"));
            phyTestStatus = L2EnablePhy(adapter);
            if (phyTestStatus == NDIS_STATUS_SUCCESS) {
                phyTestStatus = L2ConfigurePhy(adapter);
            }
            if (phyTestStatus == NDIS_STATUS_SUCCESS) {
                adapter->PhyLinkSelfTestStage =
                    L2_PHY_LINK_SELFTEST_STAGE_WAIT_UP;
                adapter->PhyLinkSelfTestStartTick = adapter->PollTimerTicks;
            } else {
                ++adapter->PhyLinkSelfTestFailures;
                adapter->PhyLinkSelfTestStage =
                    L2_PHY_LINK_SELFTEST_STAGE_COMPLETE;
                DBGPRINT(("[L2] PHY link self-test: autoneg restart failed status=%08X\n",
                          phyTestStatus));
            }
        } else if ((adapter->PollTimerTicks -
                    adapter->PhyLinkSelfTestStartTick) >=
                   L2_PHY_LINK_SELFTEST_TIMEOUT_TICKS) {
            ++adapter->PhyLinkSelfTestFailures;
            adapter->PhyLinkSelfTestStage =
                L2_PHY_LINK_SELFTEST_STAGE_COMPLETE;
            (VOID)L2EnablePhy(adapter);
            (VOID)L2ConfigurePhy(adapter);
            DBGPRINT(("[L2] PHY link self-test: disconnect timeout\n"));
        }
    } else if ((adapter != NULL) &&
               (adapter->PhyLinkSelfTestStage ==
                L2_PHY_LINK_SELFTEST_STAGE_WAIT_UP)) {
        if (adapter->PhyLinkUp && adapter->PhyLinkResolved) {
            ++adapter->PhyLinkSelfTestSuccesses;
            adapter->PhyLinkSelfTestStage =
                L2_PHY_LINK_SELFTEST_STAGE_COMPLETE;
            DBGPRINT(("[L2] PHY link self-test complete: successes=%lu failures=%lu media=%lu\n",
                      adapter->PhyLinkSelfTestSuccesses,
                      adapter->PhyLinkSelfTestFailures,
                      (ULONG)adapter->MediaConnected));
        } else if ((adapter->PollTimerTicks -
                    adapter->PhyLinkSelfTestStartTick) >=
                   L2_PHY_LINK_SELFTEST_TIMEOUT_TICKS) {
            ++adapter->PhyLinkSelfTestFailures;
            adapter->PhyLinkSelfTestStage =
                L2_PHY_LINK_SELFTEST_STAGE_COMPLETE;
            DBGPRINT(("[L2] PHY link self-test: reconnect timeout\n"));
        }
    }
#endif

    /* Link is diagnostic only until MAC/DMA and datapath setup are complete. */
    return FALSE;
}

VOID
L2MiniportPollTimer(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)FunctionContext;
    ULONG packetsBefore;

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    if ((adapter == NULL) || !adapter->PollTimerRunning) {
        return;
    }

    ++adapter->PollTimerTicks;
    if (adapter->RuntimeResetInProgress) {
        L2ContinueRuntimeReset(adapter);
        return;
    }
#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    if (adapter->TxPendingCount != 0) {
        (VOID)L2DrainPendingTx(adapter, TRUE);
        if ((adapter->TxPendingCount != 0) &&
            ((adapter->PollTimerTicks -
              adapter->PendingTxStartTicks[adapter->TxStatusIndex]) >= 4)) {
            adapter->TxPathFailed = TRUE;
            ++adapter->TxTimeouts;
            DBGPRINT(("[L2] NDIS TX watchdog timeout; failing %lu queued packets and stopping path\n",
                      adapter->TxPendingCount));
            L2FailPendingTx(adapter);
        }
    }
#endif
    if (adapter->RxMacEnabled && adapter->HardwareReady) {
        packetsBefore = adapter->RxPacketsSeen;
        L2PollRxDiagnostic(adapter);
        if (adapter->InterruptArmed &&
            (adapter->RxPacketsSeen != packetsBefore)) {
            ++adapter->PollRecoveryCount;
            adapter->PollRecoveredPackets +=
                adapter->RxPacketsSeen - packetsBefore;
            if (adapter->PollRecoveryCount <= 4) {
                DBGPRINT(("[L2] RX watchdog recovery: count=%lu packets=%lu total=%lu\n",
                          adapter->PollRecoveryCount,
                          adapter->RxPacketsSeen - packetsBefore,
                          adapter->RxPacketsSeen));
            }
        }

        if (adapter->TelemetryLogCountdown == 0) {
            DBGPRINT(("[L2] telemetry: link=%lu tx=%lu/%lu pending=%lu max=%lu irq=%lu/%lu txirq=%lu txwd=%lu txto=%lu rx=%lu/%lu rxwd=%lu/%lu reset=%lu/%lu phytest=%lu/%lu\n",
                      (ULONG)adapter->MediaConnected,
                      adapter->TxPacketsSent,
                      adapter->TxPacketErrors,
                      adapter->TxPendingCount,
                      adapter->TxMaximumPendingCount,
                      adapter->InterruptCount,
                      adapter->InterruptDpcCount,
                      adapter->TxInterruptCompletions,
                      adapter->TxWatchdogCompletions,
                      adapter->TxTimeouts,
                      adapter->RxPacketsIndicated,
                      adapter->RxPacketErrors,
                      adapter->PollRecoveryCount,
                      adapter->PollRecoveredPackets,
                      adapter->RuntimeResetCount,
                      adapter->RuntimeResetFailures,
                      adapter->PhyLinkSelfTestSuccesses,
                      adapter->PhyLinkSelfTestFailures));
            adapter->TelemetryLogCountdown = 80;
        } else {
            --adapter->TelemetryLogCountdown;
        }
    }
}

VOID
L2MiniportDisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if ((adapter != NULL) && adapter->InterruptRegistered) {
        L2WriteReg32(adapter, L2_REG_IMR, 0);
    }
}

VOID
L2MiniportEnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if ((adapter != NULL) &&
        adapter->InterruptRegistered &&
        adapter->InterruptArmed) {
        L2WriteReg32(adapter, L2_REG_IMR, L2_IMR_DATAPATH);
    }
}

VOID
L2MiniportHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if ((adapter == NULL) || !adapter->InterruptArmed) {
        return;
    }

    ++adapter->InterruptDpcCount;
    if ((adapter->LastInterruptStatus & L2_ISR_TX_STATUS_UPDATE) != 0) {
        (VOID)L2DrainPendingTx(adapter, FALSE);
    }
    if ((adapter->LastInterruptStatus & L2_ISR_RX_STATUS_UPDATE) != 0) {
        L2PollRxDiagnostic(adapter);
    }
    L2WriteReg32(adapter, L2_REG_ISR, 0);

    if (adapter->InterruptDpcCount <= 4) {
        DBGPRINT(("[L2] datapath IRQ DPC: count=%lu isr=%08X rx_total=%lu\n",
                  adapter->InterruptDpcCount,
                  adapter->LastInterruptStatus,
                  adapter->RxPacketsSeen));
    }
}

VOID
L2MiniportIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;
    ULONG status;

    *InterruptRecognized = FALSE;
    *QueueMiniportHandleInterrupt = FALSE;

    if ((adapter == NULL) || !adapter->InterruptArmed) {
        return;
    }

    status = L2ReadReg32(adapter, L2_REG_ISR);
    if ((status == 0) ||
        (status == 0xFFFFFFFF) ||
        ((status & L2_ISR_DATAPATH_EVENTS) == 0)) {
        return;
    }

    adapter->LastInterruptStatus = status;
    ++adapter->InterruptCount;
    L2WriteReg32(adapter,
                  L2_REG_ISR,
                  status | L2_ISR_DISABLE_INTERRUPT);
    *InterruptRecognized = TRUE;
    *QueueMiniportHandleInterrupt = TRUE;
}

NDIS_STATUS
L2MiniportReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    *AddressingReset = FALSE;
    if ((adapter == NULL) ||
        (adapter->Registers == NULL) ||
        !adapter->DmaMemoryAllocated ||
        !adapter->PollTimerRunning) {
        return NDIS_STATUS_NOT_RESETTABLE;
    }
    if (adapter->RuntimeResetInProgress) {
        return NDIS_STATUS_RESET_IN_PROGRESS;
    }

    DBGPRINT(("[L2] runtime reset requested: pending=%lu media=%lu\n",
              adapter->TxPendingCount,
              (ULONG)adapter->MediaConnected));
    adapter->RuntimeResetInProgress = TRUE;
    adapter->RuntimeResetStage = L2_RESET_STAGE_WAIT_IDLE;
    adapter->RuntimeResetPollCount = 0;
    adapter->HardwareReady = FALSE;
    adapter->MediaConnected = FALSE;
    adapter->InterruptArmed = FALSE;
    L2WriteReg32(adapter, L2_REG_IMR, 0);
    if (adapter->TxPendingCount != 0) {
        L2FailPendingTx(adapter);
    }

    L2WriteReg32(adapter, L2_REG_MAC_CTRL, 0);
    L2WriteReg8(adapter, L2_REG_DMAR, 0);
    L2WriteReg8(adapter, L2_REG_DMAW, 0);
    adapter->TxMacEnabled = FALSE;
    adapter->RxMacEnabled = FALSE;
    adapter->DmaEnginesEnabled = FALSE;
    adapter->ResetAttempted = TRUE;
    adapter->ResetSucceeded = FALSE;
    adapter->ResetIdleStatus = 0xFFFFFFFF;
    adapter->ResetPollCount = 0;
    L2WriteReg32(adapter,
                  L2_REG_MASTER_CTRL,
                  L2_MASTER_CTRL_SOFT_RST);
    return NDIS_STATUS_PENDING;
}

NDIS_STATUS
L2MiniportSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;
    PNDIS_BUFFER firstBuffer;
    UINT packetLength;
    ULONG startOffset;
    ULONG dataOffset;
    ULONG paddedLength;
    ULONG requiredBytes;
    ULONG freeBytes;
    ULONG newOffset;
    ULONG statusSlot;
    volatile ULONG *txStatus;
#if !defined(L2_ENABLE_TX_INTERRUPT) || !L2_ENABLE_TX_INTERRUPT
    ULONG status = 0;
    ULONG poll;
#endif
    NDIS_PHYSICAL_ADDRESS txPhysical;

    UNREFERENCED_PARAMETER(Flags);

#if defined(L2_ENABLE_NDIS_DATAPATH) && L2_ENABLE_NDIS_DATAPATH
    if ((adapter == NULL) ||
        (Packet == NULL) ||
        !adapter->HardwareReady ||
        !adapter->MediaConnected ||
        !adapter->TxMacEnabled ||
        adapter->TxPathFailed) {
        return NDIS_STATUS_NOT_ACCEPTED;
    }

    NdisQueryPacket(Packet, NULL, NULL, &firstBuffer, &packetLength);
    if ((firstBuffer == NULL) ||
        (packetLength < L2_ETH_HEADER_SIZE) ||
        (packetLength > (adapter->MaximumFrameSize + L2_ETH_HEADER_SIZE))) {
        return NDIS_STATUS_INVALID_LENGTH;
    }

#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    if (adapter->TxPendingCount >= (L2_TXS_RING_COUNT - 1)) {
        adapter->TxResourcesExhausted = TRUE;
        return NDIS_STATUS_RESOURCES;
    }
#endif

    paddedLength = ((ULONG)packetLength + 3) & ~3UL;
    requiredBytes = sizeof(ULONG) + paddedLength;
    if (adapter->TxWriteOffset >= adapter->TxReadOffset) {
        freeBytes = L2_TXD_RING_BYTES - adapter->TxWriteOffset +
                    adapter->TxReadOffset - 1;
    } else {
        freeBytes = adapter->TxReadOffset - adapter->TxWriteOffset - 1;
    }
    if (requiredBytes > freeBytes) {
        adapter->TxResourcesExhausted = TRUE;
        return NDIS_STATUS_RESOURCES;
    }

    startOffset = adapter->TxWriteOffset;
    *(PULONG)(adapter->TxdRing + startOffset) = packetLength;
    dataOffset = startOffset + sizeof(ULONG);
    if (dataOffset == L2_TXD_RING_BYTES) {
        dataOffset = 0;
    }

    if (!L2CopyPacketToTxRing(adapter, Packet, dataOffset, packetLength)) {
        return NDIS_STATUS_FAILURE;
    }

    newOffset = startOffset + sizeof(ULONG) + paddedLength;
    if (newOffset >= L2_TXD_RING_BYTES) {
        newOffset -= L2_TXD_RING_BYTES;
    }

#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    statusSlot = adapter->TxStatusClearIndex;
#else
    statusSlot = adapter->TxStatusIndex;
#endif
    txStatus = (volatile ULONG *)(adapter->TxsRing +
                 (statusSlot * L2_TXS_ENTRY_BYTES));
    *txStatus = 0;
    txPhysical.LowPart = adapter->TxdPhysicalLow;
    txPhysical.HighPart = adapter->TxdPhysicalHigh;
    NdisMUpdateSharedMemory(adapter->AdapterHandle,
                             L2_TXD_RING_BYTES,
                             adapter->TxdRing,
                             txPhysical);
#if defined(L2_ENABLE_TX_INTERRUPT) && L2_ENABLE_TX_INTERRUPT
    adapter->PendingTxPackets[statusSlot] = Packet;
    adapter->PendingTxPacketLengths[statusSlot] = packetLength;
    adapter->PendingTxStartTicks[statusSlot] = adapter->PollTimerTicks;
    ++adapter->TxStatusClearIndex;
    if (adapter->TxStatusClearIndex == L2_TXS_RING_COUNT) {
        adapter->TxStatusClearIndex = 0;
    }
    ++adapter->TxPendingCount;
    if (adapter->TxPendingCount > adapter->TxMaximumPendingCount) {
        adapter->TxMaximumPendingCount = adapter->TxPendingCount;
    }
    adapter->TxWriteOffset = newOffset;
    L2WriteReg16(adapter,
                  L2_REG_MB_TXD_WR_IDX,
                  (USHORT)(newOffset >> 2));
    return NDIS_STATUS_PENDING;
#else
    L2WriteReg16(adapter,
                  L2_REG_MB_TXD_WR_IDX,
                  (USHORT)(newOffset >> 2));

    for (poll = 0; poll < 100; ++poll) {
        NdisStallExecution(100);
        status = *txStatus;
        if ((status & L2_TX_STATUS_UPDATE) != 0) {
            break;
        }
    }

    if ((adapter->TxPacketsSent + adapter->TxPacketErrors) < 8) {
        DBGPRINT(("[L2] NDIS TX: len=%u mailbox=%04X status=%08X polls=%lu\n",
                  packetLength,
                  (USHORT)(newOffset >> 2),
                  status,
                  poll + 1));
    }

    if ((status & L2_TX_STATUS_UPDATE) == 0) {
        adapter->TxPathFailed = TRUE;
        ++adapter->TxPacketErrors;
        DBGPRINT(("[L2] NDIS TX timeout; path stopped\n"));
        return NDIS_STATUS_FAILURE;
    }

    adapter->TxWriteOffset = newOffset;
    adapter->TxReadOffset = newOffset;
    ++adapter->TxStatusIndex;
    if (adapter->TxStatusIndex == L2_TXS_RING_COUNT) {
        adapter->TxStatusIndex = 0;
    }
    *txStatus = 0;

    if (((status & L2_TX_STATUS_OK) == 0) ||
        ((status & L2_TX_STATUS_SIZE_MASK) != packetLength)) {
        ++adapter->TxPacketErrors;
        return NDIS_STATUS_FAILURE;
    }

    ++adapter->TxPacketsSent;
    return NDIS_STATUS_SUCCESS;
#endif
#else
    UNREFERENCED_PARAMETER(adapter);
    UNREFERENCED_PARAMETER(Packet);
    return NDIS_STATUS_NOT_ACCEPTED;
#endif
}


VOID
L2MiniportShutdown(
    IN PVOID ShutdownContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)ShutdownContext;

    if (adapter == NULL) {
        return;
    }

    DBGPRINT(("[L2] adapter shutdown requested\n"));
    L2HwQuiesce(adapter);
}

VOID
L2MiniportHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PL2_ADAPTER adapter = (PL2_ADAPTER)MiniportAdapterContext;

    if (adapter != NULL) {
        if (adapter->ShutdownHandlerRegistered) {
            NdisMDeregisterAdapterShutdownHandler(adapter->AdapterHandle);
            adapter->ShutdownHandlerRegistered = FALSE;
            DBGPRINT(("[L2] adapter shutdown handler deregistered\n"));
        }
        if (adapter->PollTimerInitialized && adapter->PollTimerRunning) {
            BOOLEAN timerCancelled = FALSE;

            adapter->PollTimerRunning = FALSE;
            NdisMCancelTimer(&adapter->PollTimer, &timerCancelled);
            DBGPRINT(("[L2] RX watchdog stopped: cancelled=%lu ticks=%lu recoveries=%lu packets=%lu\n",
                      (ULONG)timerCancelled,
                      adapter->PollTimerTicks,
                      adapter->PollRecoveryCount,
                      adapter->PollRecoveredPackets));
        }
        if (adapter->InterruptRegistered) {
            adapter->InterruptArmed = FALSE;
            L2WriteReg32(adapter, L2_REG_IMR, 0);
            DBGPRINT(("[L2] RX IRQ stopped: isr_count=%lu dpc_count=%lu last=%08X\n",
                      adapter->InterruptCount,
                      adapter->InterruptDpcCount,
                      adapter->LastInterruptStatus));
            NdisMDeregisterInterrupt(&adapter->Interrupt);
            adapter->InterruptRegistered = FALSE;
            DBGPRINT(("[L2] IRQ deregistered\n"));
        }
        if (adapter->TxPendingCount != 0) {
            L2FailPendingTx(adapter);
        }
        DBGPRINT(("[L2] NDIS stats: tx_ok=%lu tx_err=%lu rx_ok=%lu rx_err=%lu rx_no_buffer=%lu\n",
                  adapter->TxPacketsSent,
                  adapter->TxPacketErrors,
                  adapter->RxPacketsIndicated,
                  adapter->RxPacketErrors,
                  adapter->RxNoBuffer));
        DBGPRINT(("[L2] TX async: irq=%lu watchdog=%lu timeouts=%lu max_pending=%lu\n",
                  adapter->TxInterruptCompletions,
                  adapter->TxWatchdogCompletions,
                  adapter->TxTimeouts,
                  adapter->TxMaximumPendingCount));
        L2HwShutdown(adapter);
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
    UCHAR vendorId[4];

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

    case OID_GEN_CURRENT_PACKET_FILTER:
        moveSource = &adapter->CurrentPacketFilter;
        moveBytes = sizeof(adapter->CurrentPacketFilter);
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

    case OID_GEN_XMIT_OK:
        moveSource = &adapter->TxPacketsSent;
        moveBytes = sizeof(adapter->TxPacketsSent);
        break;

    case OID_GEN_RCV_OK:
        moveSource = &adapter->RxPacketsIndicated;
        moveBytes = sizeof(adapter->RxPacketsIndicated);
        break;

    case OID_GEN_XMIT_ERROR:
        moveSource = &adapter->TxPacketErrors;
        moveBytes = sizeof(adapter->TxPacketErrors);
        break;

    case OID_GEN_RCV_ERROR:
        moveSource = &adapter->RxPacketErrors;
        moveBytes = sizeof(adapter->RxPacketErrors);
        break;

    case OID_GEN_RCV_NO_BUFFER:
        moveSource = &adapter->RxNoBuffer;
        moveBytes = sizeof(adapter->RxNoBuffer);
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
        NdisMoveMemory(vendorId, adapter->PermanentAddress, 3);
        vendorId[3] = 0;
        moveSource = (PVOID)vendorId;
        moveBytes = sizeof(vendorId);
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
        genericUlong = L2_MAX_MULTICAST_ADDRESSES;
        moveSource = &genericUlong;
        moveBytes = sizeof(genericUlong);
        break;

    case OID_802_3_MULTICAST_LIST:
        moveSource = adapter->MulticastAddresses;
        moveBytes = adapter->MulticastAddressCount * L2_ETH_ADDR_LENGTH;
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

    if ((adapter == NULL) ||
        ((InformationBuffer == NULL) && (InformationBufferLength != 0))) {
        return NDIS_STATUS_INVALID_DATA;
    }

    *BytesRead = 0;
    *BytesNeeded = 0;

    switch (Oid) {
    case OID_GEN_CURRENT_PACKET_FILTER:
    {
        ULONG requestedFilter;

        if (InformationBufferLength < sizeof(ULONG)) {
            *BytesNeeded = sizeof(ULONG);
            return NDIS_STATUS_INVALID_LENGTH;
        }

        requestedFilter = *(PULONG)InformationBuffer;
        if ((requestedFilter & ~L2_SUPPORTED_PACKET_FILTER) != 0) {
            DBGPRINT(("[L2] unsupported packet filter requested: %08X supported=%08X\n",
                      requestedFilter,
                      (ULONG)L2_SUPPORTED_PACKET_FILTER));
            return NDIS_STATUS_NOT_SUPPORTED;
        }

        adapter->CurrentPacketFilter = requestedFilter;
        adapter->PacketFilterSet = TRUE;
        *BytesRead = sizeof(ULONG);
        return L2ApplyReceiveFilter(adapter);
    }

    case OID_802_3_MULTICAST_LIST:
    {
        ULONG addressCount;
        ULONG index;

        if ((InformationBufferLength % L2_ETH_ADDR_LENGTH) != 0) {
            return NDIS_STATUS_INVALID_LENGTH;
        }

        addressCount = InformationBufferLength / L2_ETH_ADDR_LENGTH;
        if (addressCount > L2_MAX_MULTICAST_ADDRESSES) {
            *BytesNeeded = L2_MAX_MULTICAST_ADDRESSES * L2_ETH_ADDR_LENGTH;
            return NDIS_STATUS_MULTICAST_FULL;
        }

        for (index = 0; index < addressCount; index++) {
            PUCHAR address = (PUCHAR)InformationBuffer +
                             (index * L2_ETH_ADDR_LENGTH);

            if (((address[0] & 1) == 0) ||
                ((address[0] == 0xFF) && (address[1] == 0xFF) &&
                 (address[2] == 0xFF) && (address[3] == 0xFF) &&
                 (address[4] == 0xFF) && (address[5] == 0xFF))) {
                return NDIS_STATUS_INVALID_DATA;
            }
        }

        if (InformationBufferLength != 0) {
            NdisMoveMemory(adapter->MulticastAddresses,
                           InformationBuffer,
                           InformationBufferLength);
        }
        adapter->MulticastAddressCount = addressCount;
        *BytesRead = InformationBufferLength;
        return L2ApplyReceiveFilter(adapter);
    }

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
