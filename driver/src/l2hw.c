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
        (RegisterOffset == L2_REG_MAC_STA_ADDR_HI) ||
        (RegisterOffset == L2_REG_MDIO_CTRL)
#if defined(L2_ENABLE_RING_PROGRAMMING) && L2_ENABLE_RING_PROGRAMMING
        || (RegisterOffset == L2_REG_DESC_BASE_HI)
        || (RegisterOffset == L2_REG_TXD_BASE_LO)
        || (RegisterOffset == L2_REG_TXS_BASE_LO)
        || (RegisterOffset == L2_REG_RXD_BASE_LO)
        || (RegisterOffset == L2_REG_MAC_CTRL)
        || (RegisterOffset == L2_REG_MAC_IPG_IFG)
        || (RegisterOffset == L2_REG_RX_HASH_0)
        || (RegisterOffset == L2_REG_RX_HASH_1)
        || (RegisterOffset == L2_REG_MAC_HALF_DUPLEX)
        || (RegisterOffset == L2_REG_MTU)
        || (RegisterOffset == L2_REG_TX_CUT_THRESHOLD)
        || (RegisterOffset == L2_REG_IMR)
        || (RegisterOffset == L2_REG_ISR)
#endif
        ) {
        return TRUE;
    }

    return FALSE;
}

static VOID
L2AddPhysicalOffset(
    IN ULONG BaseLow,
    IN ULONG BaseHigh,
    IN ULONG Offset,
    OUT PULONG ResultLow,
    OUT PULONG ResultHigh
    )
{
    ULONG low = BaseLow + Offset;

    *ResultLow = low;
    *ResultHigh = BaseHigh + ((low < BaseLow) ? 1 : 0);
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
    BOOLEAN writeAllowed = FALSE;

#if defined(L2_ENABLE_SOFT_RESET) && L2_ENABLE_SOFT_RESET
    if (RegisterOffset == L2_REG_MASTER_CTRL) {
        writeAllowed = TRUE;
    }
#endif

#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    if (RegisterOffset == L2_REG_MDIO_CTRL) {
        writeAllowed = TRUE;
    }
#endif

#if defined(L2_ENABLE_RING_PROGRAMMING) && L2_ENABLE_RING_PROGRAMMING
    if ((RegisterOffset == L2_REG_DESC_BASE_HI) ||
        (RegisterOffset == L2_REG_TXD_BASE_LO) ||
        (RegisterOffset == L2_REG_TXS_BASE_LO) ||
        (RegisterOffset == L2_REG_RXD_BASE_LO) ||
        (RegisterOffset == L2_REG_MAC_CTRL) ||
        (RegisterOffset == L2_REG_MAC_IPG_IFG) ||
        (RegisterOffset == L2_REG_MAC_STA_ADDR) ||
        (RegisterOffset == L2_REG_MAC_STA_ADDR_HI) ||
        (RegisterOffset == L2_REG_RX_HASH_0) ||
        (RegisterOffset == L2_REG_RX_HASH_1) ||
        (RegisterOffset == L2_REG_MAC_HALF_DUPLEX) ||
        (RegisterOffset == L2_REG_MTU) ||
        (RegisterOffset == L2_REG_TX_CUT_THRESHOLD) ||
        (RegisterOffset == L2_REG_IMR) ||
        (RegisterOffset == L2_REG_ISR)) {
        writeAllowed = TRUE;
    }
#endif

    if ((Adapter == NULL) || (Adapter->Registers == NULL) || !writeAllowed) {
        return;
    }

    NdisWriteRegisterUlong((PULONG)(Adapter->Registers + RegisterOffset), Value);
}

VOID
L2WriteReg16(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN USHORT Value
    )
{
    BOOLEAN writeAllowed = FALSE;

#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    if (RegisterOffset == L2_REG_PHY_ENABLE) {
        writeAllowed = TRUE;
    }
#endif

#if defined(L2_ENABLE_RING_PROGRAMMING) && L2_ENABLE_RING_PROGRAMMING
    if ((RegisterOffset == L2_REG_TXD_MEM_SIZE) ||
        (RegisterOffset == L2_REG_TXS_MEM_SIZE) ||
        (RegisterOffset == L2_REG_RXD_BUF_NUM) ||
        (RegisterOffset == L2_REG_MB_TXD_WR_IDX) ||
        (RegisterOffset == L2_REG_MB_RXD_RD_IDX) ||
        (RegisterOffset == L2_REG_IRQ_MOD_TIMER) ||
        (RegisterOffset == L2_REG_DMA_STOP_TIMER) ||
        (RegisterOffset == L2_REG_PAUSE_ON_THRESH) ||
        (RegisterOffset == L2_REG_PAUSE_OFF_THRESH)) {
        writeAllowed = TRUE;
    }
#endif

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !writeAllowed) {
        return;
    }

    NdisWriteRegisterUshort((PUSHORT)(Adapter->Registers + RegisterOffset), Value);
}

USHORT
L2ReadReg16(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    )
{
    USHORT value = 0;
    BOOLEAN readAllowed = FALSE;

#if defined(L2_ENABLE_RING_PROGRAMMING) && L2_ENABLE_RING_PROGRAMMING
    if ((RegisterOffset == L2_REG_TXD_MEM_SIZE) ||
        (RegisterOffset == L2_REG_TXS_MEM_SIZE) ||
        (RegisterOffset == L2_REG_RXD_BUF_NUM) ||
        (RegisterOffset == L2_REG_MB_TXD_WR_IDX) ||
        (RegisterOffset == L2_REG_MB_RXD_RD_IDX) ||
        (RegisterOffset == L2_REG_IRQ_MOD_TIMER) ||
        (RegisterOffset == L2_REG_DMA_STOP_TIMER) ||
        (RegisterOffset == L2_REG_PAUSE_ON_THRESH) ||
        (RegisterOffset == L2_REG_PAUSE_OFF_THRESH)) {
        readAllowed = TRUE;
    }
#endif

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !readAllowed) {
        return 0;
    }

    NdisReadRegisterUshort((PUSHORT)(Adapter->Registers + RegisterOffset), &value);
    return value;
}

UCHAR
L2ReadReg8(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    )
{
    UCHAR value = 0;

#if defined(L2_ENABLE_DMA_DIAGNOSTIC) && L2_ENABLE_DMA_DIAGNOSTIC
    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        ((RegisterOffset != L2_REG_DMAR) &&
         (RegisterOffset != L2_REG_DMAW))) {
        return 0;
    }

    NdisReadRegisterUchar((PUCHAR)(Adapter->Registers + RegisterOffset), &value);
#else
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(RegisterOffset);
#endif
    return value;
}

VOID
L2WriteReg8(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN UCHAR Value
    )
{
#if defined(L2_ENABLE_DMA_DIAGNOSTIC) && L2_ENABLE_DMA_DIAGNOSTIC
    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        ((RegisterOffset != L2_REG_DMAR) &&
         (RegisterOffset != L2_REG_DMAW))) {
        return;
    }

    NdisWriteRegisterUchar((PUCHAR)(Adapter->Registers + RegisterOffset), Value);
#else
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(RegisterOffset);
    UNREFERENCED_PARAMETER(Value);
#endif
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
    Adapter->InterruptResourceFound = FALSE;
    Adapter->InterruptVector = 0;
    Adapter->InterruptLevel = 0;
    Adapter->InterruptMode = NdisInterruptLevelSensitive;
    Adapter->InterruptShared = FALSE;

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

        if ((Adapter->SelectedResourceIndex == L2_INVALID_RESOURCE_INDEX) &&
            (partial->Type == CmResourceTypeMemory) &&
            (partial->u.Memory.Length > 0)) {
            Adapter->SelectedResourceIndex = i;
            Adapter->SelectedResourceType = partial->Type;
            Adapter->SelectedResourceLength = partial->u.Memory.Length;
            Adapter->MmioPhysicalBaseLow = partial->u.Memory.Start.LowPart;
            Adapter->MmioPhysicalBaseHigh = partial->u.Memory.Start.HighPart;
        } else if (!Adapter->InterruptResourceFound &&
                   (partial->Type == CmResourceTypeInterrupt)) {
            Adapter->InterruptResourceFound = TRUE;
            Adapter->InterruptVector = partial->u.Interrupt.Vector;
            Adapter->InterruptLevel = partial->u.Interrupt.Level;
            Adapter->InterruptShared =
                (partial->ShareDisposition == CmResourceShareShared) ?
                TRUE : FALSE;
            Adapter->InterruptMode =
                ((partial->Flags & CM_RESOURCE_INTERRUPT_LATCHED) != 0) ?
                NdisInterruptLatched : NdisInterruptLevelSensitive;
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
L2AllocateDmaMemory(
    IN PL2_ADAPTER Adapter
    )
{
    ULONG cursor;
    ULONG offset;
    ULONG physicalLow;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->RingAllocation = NULL;
    Adapter->RingAllocationLength = L2_RING_ALLOCATION_BYTES;
    Adapter->TxdRing = NULL;
    Adapter->TxsRing = NULL;
    Adapter->RxdRing = NULL;
    Adapter->DmaMemoryAllocated = FALSE;
    NdisZeroMemory(&Adapter->RingPhysicalAddress,
                   sizeof(Adapter->RingPhysicalAddress));

    NdisMAllocateSharedMemory(
        Adapter->AdapterHandle,
        Adapter->RingAllocationLength,
        FALSE,
        &Adapter->RingAllocation,
        &Adapter->RingPhysicalAddress
        );

    if (Adapter->RingAllocation == NULL) {
        DBGPRINT(("[L2] DMA memory allocation failed: len=%lu\n",
                  Adapter->RingAllocationLength));
        return NDIS_STATUS_RESOURCES;
    }

    NdisZeroMemory(Adapter->RingAllocation, Adapter->RingAllocationLength);

    physicalLow = Adapter->RingPhysicalAddress.LowPart;
    offset = (8 - (physicalLow & 7)) & 7;
    cursor = offset;
    Adapter->TxdRing = (PUCHAR)Adapter->RingAllocation + cursor;
    L2AddPhysicalOffset(Adapter->RingPhysicalAddress.LowPart,
                        Adapter->RingPhysicalAddress.HighPart,
                        cursor,
                        &Adapter->TxdPhysicalLow,
                        &Adapter->TxdPhysicalHigh);

    cursor += L2_TXD_RING_BYTES;
    physicalLow = Adapter->RingPhysicalAddress.LowPart + cursor;
    offset = (8 - (physicalLow & 7)) & 7;
    cursor += offset;
    Adapter->TxsRing = (PUCHAR)Adapter->RingAllocation + cursor;
    L2AddPhysicalOffset(Adapter->RingPhysicalAddress.LowPart,
                        Adapter->RingPhysicalAddress.HighPart,
                        cursor,
                        &Adapter->TxsPhysicalLow,
                        &Adapter->TxsPhysicalHigh);

    cursor += L2_TXS_RING_COUNT * L2_TXS_ENTRY_BYTES;
    physicalLow = Adapter->RingPhysicalAddress.LowPart + cursor;
    offset = (128 - (physicalLow & 127)) & 127;
    if (offset > 7) {
        offset -= 8;
    } else {
        offset += 120;
    }
    cursor += offset;
    Adapter->RxdRing = (PUCHAR)Adapter->RingAllocation + cursor;
    L2AddPhysicalOffset(Adapter->RingPhysicalAddress.LowPart,
                        Adapter->RingPhysicalAddress.HighPart,
                        cursor,
                        &Adapter->RxdPhysicalLow,
                        &Adapter->RxdPhysicalHigh);

    if ((cursor + (L2_RXD_RING_COUNT * L2_RXD_ENTRY_BYTES)) >
        Adapter->RingAllocationLength) {
        NdisMFreeSharedMemory(Adapter->AdapterHandle,
                              Adapter->RingAllocationLength,
                              FALSE,
                              Adapter->RingAllocation,
                              Adapter->RingPhysicalAddress);
        DBGPRINT(("[L2] DMA memory rejected: ring layout exceeds allocation\n"));
        Adapter->RingAllocation = NULL;
        return NDIS_STATUS_RESOURCES;
    }

    /*
     * ATL2 exposes one shared high-address register for all three rings.
     * Reject the extremely unlikely case where this allocation crosses a
     * 4-GiB boundary rather than programming an internally inconsistent set.
     */
    if ((Adapter->TxdPhysicalHigh !=
         (ULONG)Adapter->RingPhysicalAddress.HighPart) ||
        (Adapter->TxsPhysicalHigh !=
         (ULONG)Adapter->RingPhysicalAddress.HighPart) ||
        (Adapter->RxdPhysicalHigh !=
         (ULONG)Adapter->RingPhysicalAddress.HighPart)) {
        DBGPRINT(("[L2] DMA memory rejected: allocation crosses 4-GiB boundary\n"));
        NdisMFreeSharedMemory(Adapter->AdapterHandle,
                              Adapter->RingAllocationLength,
                              FALSE,
                              Adapter->RingAllocation,
                              Adapter->RingPhysicalAddress);
        Adapter->RingAllocation = NULL;
        Adapter->TxdRing = NULL;
        Adapter->TxsRing = NULL;
        Adapter->RxdRing = NULL;
        return NDIS_STATUS_RESOURCES;
    }

    Adapter->DmaMemoryAllocated = TRUE;
    DBGPRINT(("[L2] DMA memory: len=%lu base=%08X:%08X txd=%08X:%08X txs=%08X:%08X rxd=%08X:%08X\n",
              Adapter->RingAllocationLength,
              Adapter->RingPhysicalAddress.HighPart,
              Adapter->RingPhysicalAddress.LowPart,
              Adapter->TxdPhysicalHigh,
              Adapter->TxdPhysicalLow,
              Adapter->TxsPhysicalHigh,
              Adapter->TxsPhysicalLow,
              Adapter->RxdPhysicalHigh,
              Adapter->RxdPhysicalLow));
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
L2ProgramDmaRings(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_RING_PROGRAMMING) && L2_ENABLE_RING_PROGRAMMING
    ULONG high;
    ULONG txd;
    ULONG txs;
    ULONG rxd;
    USHORT txdSize;
    USHORT txsSize;
    USHORT rxdCount;
    USHORT txdIndex;
    USHORT rxdIndex;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !Adapter->ResetSucceeded ||
        !Adapter->DmaMemoryAllocated) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->DmaRingsProgrammed = FALSE;

    L2WriteReg32(Adapter,
                  L2_REG_DESC_BASE_HI,
                  (ULONG)Adapter->RingPhysicalAddress.HighPart);
    L2WriteReg32(Adapter, L2_REG_TXD_BASE_LO, Adapter->TxdPhysicalLow);
    L2WriteReg32(Adapter, L2_REG_TXS_BASE_LO, Adapter->TxsPhysicalLow);
    L2WriteReg32(Adapter, L2_REG_RXD_BASE_LO, Adapter->RxdPhysicalLow);
    L2WriteReg16(Adapter,
                  L2_REG_TXD_MEM_SIZE,
                  (USHORT)(L2_TXD_RING_BYTES / 4));
    L2WriteReg16(Adapter, L2_REG_TXS_MEM_SIZE, L2_TXS_RING_COUNT);
    L2WriteReg16(Adapter, L2_REG_RXD_BUF_NUM, L2_RXD_RING_COUNT);
    L2WriteReg16(Adapter, L2_REG_MB_TXD_WR_IDX, 0);
    L2WriteReg16(Adapter, L2_REG_MB_RXD_RD_IDX, 0);

    high = L2ReadReg32(Adapter, L2_REG_DESC_BASE_HI);
    txd = L2ReadReg32(Adapter, L2_REG_TXD_BASE_LO);
    txs = L2ReadReg32(Adapter, L2_REG_TXS_BASE_LO);
    rxd = L2ReadReg32(Adapter, L2_REG_RXD_BASE_LO);
    txdSize = L2ReadReg16(Adapter, L2_REG_TXD_MEM_SIZE);
    txsSize = L2ReadReg16(Adapter, L2_REG_TXS_MEM_SIZE);
    rxdCount = L2ReadReg16(Adapter, L2_REG_RXD_BUF_NUM);
    txdIndex = L2ReadReg16(Adapter, L2_REG_MB_TXD_WR_IDX);
    rxdIndex = L2ReadReg16(Adapter, L2_REG_MB_RXD_RD_IDX);

    DBGPRINT(("[L2] rings readback: hi=%08X txd=%08X/%04X txs=%08X/%04X rxd=%08X/%04X mb=%04X:%04X\n",
              high,
              txd,
              txdSize,
              txs,
              txsSize,
              rxd,
              rxdCount,
              txdIndex,
              rxdIndex));

    if ((high != (ULONG)Adapter->RingPhysicalAddress.HighPart) ||
        (txd != Adapter->TxdPhysicalLow) ||
        (txs != Adapter->TxsPhysicalLow) ||
        (rxd != Adapter->RxdPhysicalLow) ||
        (txdSize != (USHORT)(L2_TXD_RING_BYTES / 4)) ||
        (txsSize != L2_TXS_RING_COUNT) ||
        (rxdCount != L2_RXD_RING_COUNT) ||
        (txdIndex != 0) ||
        (rxdIndex != 0)) {
        DBGPRINT(("[L2] ring programming verification failed\n"));
        return NDIS_STATUS_FAILURE;
    }

    Adapter->DmaRingsProgrammed = TRUE;
    DBGPRINT(("[L2] ring programming verified; DMA remains disabled\n"));
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2ConfigureMacStatic(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_MAC_STATIC) && L2_ENABLE_MAC_STATIC
    ULONG stationLow;
    ULONG stationHigh;
    ULONG readStationLow;
    ULONG readStationHigh;
    ULONG readControl;
    ULONG readIpg;
    ULONG readHalf;
    ULONG readMtu;
    ULONG readCut;
    USHORT readIrqTimer;
    USHORT readStopTimer;
    USHORT readPauseOn;
    USHORT readPauseOff;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !Adapter->MacReadSucceeded ||
        !Adapter->DmaRingsProgrammed) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->MacStaticConfigured = FALSE;
    stationLow = ((ULONG)Adapter->PermanentMac[2] << 24) |
                 ((ULONG)Adapter->PermanentMac[3] << 16) |
                 ((ULONG)Adapter->PermanentMac[4] << 8) |
                 (ULONG)Adapter->PermanentMac[5];
    stationHigh = ((ULONG)Adapter->PermanentMac[0] << 8) |
                  (ULONG)Adapter->PermanentMac[1];

    /* Keep both MAC directions explicitly disabled throughout this stage. */
    L2WriteReg32(Adapter, L2_REG_MAC_CTRL, 0);
    L2WriteReg32(Adapter, L2_REG_MAC_STA_ADDR, stationLow);
    L2WriteReg32(Adapter, L2_REG_MAC_STA_ADDR_HI, stationHigh);
    L2WriteReg32(Adapter, L2_REG_RX_HASH_0, 0);
    L2WriteReg32(Adapter, L2_REG_RX_HASH_1, 0);
    L2WriteReg32(Adapter, L2_REG_MAC_IPG_IFG, L2_MAC_IPG_IFG_DEFAULT);
    L2WriteReg32(Adapter, L2_REG_MAC_HALF_DUPLEX, L2_MAC_HALF_DEFAULT);
    L2WriteReg32(Adapter, L2_REG_MTU, L2_MAC_FRAME_BYTES);
    L2WriteReg32(Adapter, L2_REG_TX_CUT_THRESHOLD, L2_TX_CUT_DEFAULT);
    L2WriteReg16(Adapter, L2_REG_IRQ_MOD_TIMER, L2_IRQ_MOD_DEFAULT);
    L2WriteReg16(Adapter, L2_REG_DMA_STOP_TIMER, L2_DMA_STOP_DEFAULT);
    L2WriteReg16(Adapter, L2_REG_PAUSE_ON_THRESH, L2_PAUSE_ON_DEFAULT);
    L2WriteReg16(Adapter, L2_REG_PAUSE_OFF_THRESH, L2_PAUSE_OFF_DEFAULT);

    readControl = L2ReadReg32(Adapter, L2_REG_MAC_CTRL);
    readStationLow = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR);
    readStationHigh = L2ReadReg32(Adapter, L2_REG_MAC_STA_ADDR_HI);
    readIpg = L2ReadReg32(Adapter, L2_REG_MAC_IPG_IFG);
    readHalf = L2ReadReg32(Adapter, L2_REG_MAC_HALF_DUPLEX);
    readMtu = L2ReadReg32(Adapter, L2_REG_MTU);
    readCut = L2ReadReg32(Adapter, L2_REG_TX_CUT_THRESHOLD);
    readIrqTimer = L2ReadReg16(Adapter, L2_REG_IRQ_MOD_TIMER);
    readStopTimer = L2ReadReg16(Adapter, L2_REG_DMA_STOP_TIMER);
    readPauseOn = L2ReadReg16(Adapter, L2_REG_PAUSE_ON_THRESH);
    readPauseOff = L2ReadReg16(Adapter, L2_REG_PAUSE_OFF_THRESH);

    DBGPRINT(("[L2] MAC static readback: ctrl=%08X addr=%08X:%08X ipg=%08X half=%08X mtu=%04X cut=%04X pause=%04X:%04X timers=%04X:%04X\n",
              readControl,
              readStationHigh,
              readStationLow,
              readIpg,
              readHalf,
              readMtu,
              readCut,
              readPauseOn,
              readPauseOff,
              readIrqTimer,
              readStopTimer));

    if ((readControl != 0) ||
        (readStationLow != stationLow) ||
        ((readStationHigh & 0xFFFF) != stationHigh) ||
        (L2ReadReg32(Adapter, L2_REG_RX_HASH_0) != 0) ||
        (L2ReadReg32(Adapter, L2_REG_RX_HASH_1) != 0) ||
        (readIpg != L2_MAC_IPG_IFG_DEFAULT) ||
        (readHalf != L2_MAC_HALF_DEFAULT) ||
        (readMtu != L2_MAC_FRAME_BYTES) ||
        (readCut != L2_TX_CUT_DEFAULT) ||
        (readIrqTimer != L2_IRQ_MOD_DEFAULT) ||
        (readStopTimer != L2_DMA_STOP_DEFAULT) ||
        (readPauseOn != L2_PAUSE_ON_DEFAULT) ||
        (readPauseOff != L2_PAUSE_OFF_DEFAULT)) {
        DBGPRINT(("[L2] MAC static configuration verification failed\n"));
        return NDIS_STATUS_FAILURE;
    }

    Adapter->MacStaticConfigured = TRUE;
    DBGPRINT(("[L2] MAC static configuration verified; TX/RX remain disabled\n"));
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2EnableDmaEnginesDiagnostic(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_DMA_DIAGNOSTIC) && L2_ENABLE_DMA_DIAGNOSTIC
    UCHAR dmar;
    UCHAR dmaw;
    ULONG idleStatus = 0xFFFFFFFF;
    ULONG poll;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !Adapter->DmaRingsProgrammed ||
        !Adapter->MacStaticConfigured) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->DmaEnginesEnabled = FALSE;
    Adapter->DmaIdleStatus = idleStatus;
    Adapter->DmaIdlePollCount = 0;

    L2WriteReg8(Adapter, L2_REG_DMAR, L2_DMAR_ENABLE);
    L2WriteReg8(Adapter, L2_REG_DMAW, L2_DMAW_ENABLE);
    dmar = L2ReadReg8(Adapter, L2_REG_DMAR);
    dmaw = L2ReadReg8(Adapter, L2_REG_DMAW);

    if (((dmar & L2_DMAR_ENABLE) == 0) ||
        ((dmaw & L2_DMAW_ENABLE) == 0)) {
        DBGPRINT(("[L2] DMA enable readback failed: dmar=%02X dmaw=%02X\n",
                  dmar,
                  dmaw));
        L2WriteReg8(Adapter, L2_REG_DMAR, 0);
        L2WriteReg8(Adapter, L2_REG_DMAW, 0);
        return NDIS_STATUS_FAILURE;
    }

    for (poll = 0; poll < 10; ++poll) {
        NdisStallExecution(1000);
        idleStatus = L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);
        Adapter->DmaIdleStatus = idleStatus;
        Adapter->DmaIdlePollCount = poll + 1;
        if ((idleStatus & L2_IDLE_STATUS_DMA_MASK) == 0) {
            Adapter->DmaEnginesEnabled = TRUE;
            DBGPRINT(("[L2] DMA enabled idle: dmar=%02X dmaw=%02X status=%08X polls=%lu; MAC remains disabled\n",
                      dmar,
                      dmaw,
                      idleStatus,
                      Adapter->DmaIdlePollCount));
            return NDIS_STATUS_SUCCESS;
        }
    }

    DBGPRINT(("[L2] DMA did not become idle: status=%08X polls=%lu; disabling\n",
              Adapter->DmaIdleStatus,
              Adapter->DmaIdlePollCount));
    L2WriteReg8(Adapter, L2_REG_DMAR, 0);
    L2WriteReg8(Adapter, L2_REG_DMAW, 0);
    return NDIS_STATUS_FAILURE;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2EnableTxMacDiagnostic(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_TX_MAC_DIAGNOSTIC) && L2_ENABLE_TX_MAC_DIAGNOSTIC
    ULONG control;
    ULONG readback;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        !Adapter->DmaEnginesEnabled ||
        !Adapter->PhyLinkUp ||
        !Adapter->PhyLinkResolved) {
        return NDIS_STATUS_INVALID_DATA;
    }

    control = L2_MAC_CTRL_TX_BASE;
    if (Adapter->PhyFullDuplex) {
        control |= L2_MAC_CTRL_DUPLEX;
    }

    L2WriteReg32(Adapter, L2_REG_MAC_CTRL, control);
    readback = L2ReadReg32(Adapter, L2_REG_MAC_CTRL);
    if (readback != control) {
        DBGPRINT(("[L2] TX MAC enable verification failed: wrote=%08X read=%08X\n",
                  control,
                  readback));
        L2WriteReg32(Adapter, L2_REG_MAC_CTRL, 0);
        Adapter->TxMacEnabled = FALSE;
        return NDIS_STATUS_FAILURE;
    }

    Adapter->TxMacEnabled = TRUE;
    DBGPRINT(("[L2] TX MAC enabled: ctrl=%08X speed=%lu duplex=%s; RX remains disabled\n",
              readback,
              Adapter->PhySpeedMbps,
              Adapter->PhyFullDuplex ? "full" : "half"));
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2RunTxSelfTest(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_TX_SELFTEST) && L2_ENABLE_TX_SELFTEST
    PUCHAR frame;
    volatile ULONG *txStatus;
    ULONG status;
    ULONG poll;
    ULONG i;
    NDIS_PHYSICAL_ADDRESS txPhysical;

    if ((Adapter == NULL) ||
        !Adapter->TxMacEnabled ||
        (Adapter->TxdRing == NULL) ||
        (Adapter->TxsRing == NULL)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->TxSelfTestAttempted = TRUE;
    Adapter->TxSelfTestSucceeded = FALSE;
    Adapter->TxSelfTestStatus = 0;
    Adapter->TxSelfTestPollCount = 0;

    NdisZeroMemory(Adapter->TxdRing, 64);
    NdisZeroMemory(Adapter->TxsRing, L2_TXS_ENTRY_BYTES);
    *(PULONG)Adapter->TxdRing = L2_TX_SELFTEST_FRAME_LEN;
    frame = Adapter->TxdRing + sizeof(ULONG);

    for (i = 0; i < 6; ++i) {
        frame[i] = 0xFF;
        frame[6 + i] = Adapter->PermanentMac[i];
    }
    frame[12] = 0x88;
    frame[13] = 0xB5;
    for (i = 14; i < L2_TX_SELFTEST_FRAME_LEN; ++i) {
        frame[i] = (UCHAR)i;
    }

    txPhysical.LowPart = Adapter->TxdPhysicalLow;
    txPhysical.HighPart = Adapter->TxdPhysicalHigh;
    NdisMUpdateSharedMemory(Adapter->AdapterHandle,
                             64,
                             Adapter->TxdRing,
                             txPhysical);

    /* Four-byte header plus 60-byte frame equals 16 mailbox dwords. */
    L2WriteReg16(Adapter, L2_REG_MB_TXD_WR_IDX, 16);
    txStatus = (volatile ULONG *)Adapter->TxsRing;

    for (poll = 0; poll < 100; ++poll) {
        NdisStallExecution(100);
        status = *txStatus;
        Adapter->TxSelfTestStatus = status;
        Adapter->TxSelfTestPollCount = poll + 1;
        if ((status & L2_TX_STATUS_UPDATE) != 0) {
            break;
        }
    }

    status = Adapter->TxSelfTestStatus;
    DBGPRINT(("[L2] TX self-test: mailbox=%04X status=%08X update=%lu ok=%lu size=%lu polls=%lu\n",
              L2ReadReg16(Adapter, L2_REG_MB_TXD_WR_IDX),
              status,
              (ULONG)((status & L2_TX_STATUS_UPDATE) != 0),
              (ULONG)((status & L2_TX_STATUS_OK) != 0),
              status & L2_TX_STATUS_SIZE_MASK,
              Adapter->TxSelfTestPollCount));

    if (((status & L2_TX_STATUS_UPDATE) == 0) ||
        ((status & L2_TX_STATUS_OK) == 0) ||
        ((status & L2_TX_STATUS_SIZE_MASK) != L2_TX_SELFTEST_FRAME_LEN)) {
        return NDIS_STATUS_FAILURE;
    }

    Adapter->TxSelfTestSucceeded = TRUE;
    *txStatus = 0;
    Adapter->TxWriteOffset = 64;
    Adapter->TxReadOffset = 64;
    Adapter->TxStatusIndex = 1;
    Adapter->TxStatusClearIndex = 1;
    Adapter->TxPendingCount = 0;
    Adapter->TxMaximumPendingCount = 0;
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2EnableRxMacDiagnostic(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_RX_DIAGNOSTIC) && L2_ENABLE_RX_DIAGNOSTIC
    ULONG control;
    ULONG readback;

    if ((Adapter == NULL) ||
        !Adapter->TxMacEnabled ||
        !Adapter->TxSelfTestSucceeded) {
        return NDIS_STATUS_INVALID_DATA;
    }

    control = L2ReadReg32(Adapter, L2_REG_MAC_CTRL);
    control |= L2_MAC_CTRL_RX_ENABLE | L2_MAC_CTRL_BROADCAST;
    L2WriteReg32(Adapter, L2_REG_MAC_CTRL, control);
    readback = L2ReadReg32(Adapter, L2_REG_MAC_CTRL);
    if (readback != control) {
        DBGPRINT(("[L2] RX MAC enable verification failed: wrote=%08X read=%08X\n",
                  control,
                  readback));
        return NDIS_STATUS_FAILURE;
    }

    Adapter->RxMacEnabled = TRUE;
    DBGPRINT(("[L2] RX MAC enabled: ctrl=%08X; IRQ remains masked until datapath ready\n",
              readback));
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

VOID
L2PollRxDiagnostic(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_RX_DIAGNOSTIC) && L2_ENABLE_RX_DIAGNOSTIC
    PUCHAR descriptor;
    PUCHAR frame;
    volatile ULONG *statusPointer;
    ULONG status;
    ULONG wireLength;
    ULONG frameLength;
    ULONG consumed = 0;
    ULONG indicated = 0;
    ULONG packetsBefore;

    if ((Adapter == NULL) ||
        !Adapter->RxMacEnabled ||
        (Adapter->RxdRing == NULL)) {
        return;
    }

    packetsBefore = Adapter->RxPacketsSeen;
    while (consumed < L2_RXD_RING_COUNT) {
        descriptor = Adapter->RxdRing +
                     (Adapter->RxReadIndex * L2_RXD_ENTRY_BYTES);
        statusPointer = (volatile ULONG *)descriptor;
        status = *statusPointer;
        if ((status & L2_RX_STATUS_UPDATE) == 0) {
            break;
        }

        wireLength = status & L2_RX_STATUS_SIZE_MASK;
        frameLength = (wireLength >= 4) ? (wireLength - 4) : 0;
        frame = descriptor + L2_RX_STATUS_BYTES;

        if (((status & L2_RX_STATUS_OK) == 0) ||
            (frameLength < 14) ||
            (frameLength > (Adapter->MaximumFrameSize + 14))) {
            ++Adapter->RxPacketErrors;
        }

        if (!Adapter->RxFirstPacketLogged) {
            DBGPRINT(("[L2] RX first: index=%lu status=%08X ok=%lu wire=%lu data=%lu dst=%02X:%02X:%02X:%02X:%02X:%02X src=%02X:%02X:%02X:%02X:%02X:%02X type=%02X%02X\n",
                      Adapter->RxReadIndex,
                      status,
                      (ULONG)((status & L2_RX_STATUS_OK) != 0),
                      wireLength,
                      frameLength,
                      frame[0], frame[1], frame[2], frame[3], frame[4], frame[5],
                      frame[6], frame[7], frame[8], frame[9], frame[10], frame[11],
                      frame[12], frame[13]));
            Adapter->RxFirstPacketLogged = TRUE;
        }

#if defined(L2_ENABLE_NDIS_DATAPATH) && L2_ENABLE_NDIS_DATAPATH
        if (Adapter->HardwareReady &&
            Adapter->MediaConnected &&
            ((status & L2_RX_STATUS_OK) != 0) &&
            (frameLength >= 14) &&
            (frameLength <= (Adapter->MaximumFrameSize + 14))) {
            NdisMEthIndicateReceive(
                Adapter->AdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)frame,
                14,
                (PCHAR)(frame + 14),
                frameLength - 14,
                frameLength - 14
                );
            ++Adapter->RxPacketsIndicated;
            ++indicated;
        }
#endif

        *statusPointer = 0;
        ++Adapter->RxPacketsSeen;
        ++consumed;
        ++Adapter->RxReadIndex;
        if (Adapter->RxReadIndex == L2_RXD_RING_COUNT) {
            Adapter->RxReadIndex = 0;
        }
    }

    if (consumed != 0) {
        L2WriteReg16(Adapter,
                      L2_REG_MB_RXD_RD_IDX,
                      (USHORT)Adapter->RxReadIndex);
        if ((packetsBefore >> 10) != (Adapter->RxPacketsSeen >> 10)) {
            DBGPRINT(("[L2] RX progress: total=%lu next=%lu\n",
                      Adapter->RxPacketsSeen,
                      Adapter->RxReadIndex));
        }
    }


#if defined(L2_ENABLE_NDIS_DATAPATH) && L2_ENABLE_NDIS_DATAPATH
    if (indicated != 0) {
        NdisMEthIndicateReceiveComplete(Adapter->AdapterHandle);
    }
#endif
#else
    UNREFERENCED_PARAMETER(Adapter);
#endif
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
L2HwQuiesce(
    IN PL2_ADAPTER Adapter
    )
{
    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return;
    }

    L2WriteReg32(Adapter, L2_REG_IMR, 0);
    Adapter->InterruptArmed = FALSE;

#if defined(L2_ENABLE_TX_MAC_DIAGNOSTIC) && L2_ENABLE_TX_MAC_DIAGNOSTIC
    L2WriteReg32(Adapter, L2_REG_MAC_CTRL, 0);
    Adapter->TxMacEnabled = FALSE;
    Adapter->RxMacEnabled = FALSE;
#endif

#if defined(L2_ENABLE_DMA_DIAGNOSTIC) && L2_ENABLE_DMA_DIAGNOSTIC
    {
        ULONG poll;

        L2WriteReg8(Adapter, L2_REG_DMAR, 0);
        L2WriteReg8(Adapter, L2_REG_DMAW, 0);
        for (poll = 0; poll < 10; ++poll) {
            NdisStallExecution(1000);
            Adapter->DmaIdleStatus =
                L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);
            if ((Adapter->DmaIdleStatus & L2_IDLE_STATUS_DMA_MASK) == 0) {
                break;
            }
        }
        Adapter->DmaEnginesEnabled = FALSE;
        Adapter->DmaIdlePollCount = (poll < 10) ? (poll + 1) : 10;
    }
#else
    Adapter->DmaIdleStatus = 0;
    Adapter->DmaIdlePollCount = 0;
#endif

    Adapter->HardwareReady = FALSE;
    Adapter->MediaConnected = FALSE;
    DBGPRINT(("[L2] hardware quiesced: idle=%08X polls=%lu\n",
              Adapter->DmaIdleStatus,
              Adapter->DmaIdlePollCount));
}

VOID
L2HwShutdown(
    IN PL2_ADAPTER Adapter
    )
{
    if (Adapter == NULL) {
        return;
    }

    L2HwQuiesce(Adapter);

    if (Adapter->RingAllocation != NULL) {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->RingAllocationLength,
            FALSE,
            Adapter->RingAllocation,
            Adapter->RingPhysicalAddress
            );
        Adapter->RingAllocation = NULL;
    }

    NdisZeroMemory(&Adapter->RingPhysicalAddress,
                   sizeof(Adapter->RingPhysicalAddress));

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
    Adapter->ResetAttempted = FALSE;
    Adapter->ResetSucceeded = FALSE;
    Adapter->ResetIdleStatus = 0;
    Adapter->ResetPollCount = 0;
    Adapter->PhyEnableAttempted = FALSE;
    Adapter->PhyProbeSucceeded = FALSE;
    Adapter->PhyBmcr = 0;
    Adapter->PhyBmsr = 0;
    Adapter->PhyId1 = 0;
    Adapter->PhyId2 = 0;
    Adapter->PhyConfigureSucceeded = FALSE;
    Adapter->PhyDebugData = 0;
    Adapter->PhyAdvertise = 0;
    Adapter->PhyStatusLogCountdown = 0;
    Adapter->PhyPssr = 0;
    Adapter->PhyLinkUp = FALSE;
    Adapter->PhyLinkResolved = FALSE;
    Adapter->PhyFullDuplex = FALSE;
    Adapter->PhySpeedMbps = 0;
    Adapter->RingAllocationLength = 0;
    Adapter->TxdRing = NULL;
    Adapter->TxsRing = NULL;
    Adapter->RxdRing = NULL;
    Adapter->TxdPhysicalLow = 0;
    Adapter->TxdPhysicalHigh = 0;
    Adapter->TxsPhysicalLow = 0;
    Adapter->TxsPhysicalHigh = 0;
    Adapter->RxdPhysicalLow = 0;
    Adapter->RxdPhysicalHigh = 0;
    Adapter->DmaMemoryAllocated = FALSE;
    Adapter->DmaRingsProgrammed = FALSE;
    Adapter->MacStaticConfigured = FALSE;
    Adapter->DmaEnginesEnabled = FALSE;
    Adapter->DmaIdleStatus = 0;
    Adapter->DmaIdlePollCount = 0;
    Adapter->TxMacEnabled = FALSE;
    Adapter->TxSelfTestAttempted = FALSE;
    Adapter->TxSelfTestSucceeded = FALSE;
    Adapter->TxSelfTestStatus = 0;
    Adapter->TxSelfTestPollCount = 0;
    Adapter->RxMacEnabled = FALSE;
    Adapter->RxReadIndex = 0;
    Adapter->RxPacketsSeen = 0;
    Adapter->RxFirstPacketLogged = FALSE;
    Adapter->TxWriteOffset = 0;
    Adapter->TxReadOffset = 0;
    Adapter->TxStatusIndex = 0;
    Adapter->TxStatusClearIndex = 0;
    Adapter->TxPendingCount = 0;
    Adapter->TxMaximumPendingCount = 0;
    Adapter->TxPacketsSent = 0;
    Adapter->TxPacketErrors = 0;
    Adapter->TxPathFailed = FALSE;
    Adapter->TxResourcesExhausted = FALSE;
    Adapter->TxInterruptCompletions = 0;
    Adapter->TxWatchdogCompletions = 0;
    Adapter->TxTimeouts = 0;
    Adapter->RxPacketsIndicated = 0;
    Adapter->RxPacketErrors = 0;
    Adapter->RxNoBuffer = 0;
    Adapter->HardwareReady = FALSE;
}

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_SOFT_RESET) && L2_ENABLE_SOFT_RESET
    ULONG idleStatus = 0xFFFFFFFF;
    ULONG poll;

    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    Adapter->ResetAttempted = TRUE;
    Adapter->ResetSucceeded = FALSE;
    Adapter->ResetIdleStatus = idleStatus;
    Adapter->ResetPollCount = 0;

    DBGPRINT(("[L2] RESET begin: MASTER_CTRL=0x%08X\n",
              L2_MASTER_CTRL_SOFT_RST));

    L2WriteReg32(Adapter, L2_REG_MASTER_CTRL, L2_MASTER_CTRL_SOFT_RST);
    NdisStallExecution(1000);

    for (poll = 0; poll < 10; ++poll) {
        idleStatus = L2ReadReg32(Adapter, L2_REG_IDLE_STATUS);
        Adapter->ResetIdleStatus = idleStatus;
        Adapter->ResetPollCount = poll + 1;

        if (idleStatus == 0) {
            Adapter->ResetSucceeded = TRUE;
            DBGPRINT(("[L2] RESET complete: polls=%lu idle=0x%08X\n",
                      Adapter->ResetPollCount,
                      Adapter->ResetIdleStatus));
            return NDIS_STATUS_SUCCESS;
        }

        NdisStallExecution(1000);
    }

    DBGPRINT(("[L2] RESET timeout: polls=%lu idle=0x%08X\n",
              Adapter->ResetPollCount,
              Adapter->ResetIdleStatus));
    return NDIS_STATUS_FAILURE;
#else
    UNREFERENCED_PARAMETER(Adapter);

    /* Ultra-safe bring-up mode: reset is disabled to avoid MMIO write hangs. */
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2EnablePhy(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    if ((Adapter == NULL) || (Adapter->Registers == NULL)) {
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    Adapter->PhyEnableAttempted = TRUE;
    L2WriteReg16(Adapter, L2_REG_PHY_ENABLE, 1);
    NdisStallExecution(1000);
    DBGPRINT(("[L2] PHY enabled\n"));
    return NDIS_STATUS_SUCCESS;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2ReadPhyReg(
    IN PL2_ADAPTER Adapter,
    IN USHORT RegisterAddress,
    OUT PUSHORT RegisterValue
    )
{
#if defined(L2_ENABLE_PHY_PROBE) && L2_ENABLE_PHY_PROBE
    ULONG control;
    ULONG poll;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        (RegisterValue == NULL) ||
        (RegisterAddress > L2_MDIO_REG_ADDR_MASK)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    control = (((ULONG)RegisterAddress & L2_MDIO_REG_ADDR_MASK) <<
               L2_MDIO_REG_ADDR_SHIFT) |
              L2_MDIO_START |
              L2_MDIO_SUP_PREAMBLE |
              L2_MDIO_RW;

    L2WriteReg32(Adapter, L2_REG_MDIO_CTRL, control);

    for (poll = 0; poll < L2_MDIO_WAIT_TIMES; ++poll) {
        NdisStallExecution(2);
        control = L2ReadReg32(Adapter, L2_REG_MDIO_CTRL);
        if ((control & (L2_MDIO_START | L2_MDIO_BUSY)) == 0) {
            *RegisterValue = (USHORT)(control & L2_MDIO_DATA_MASK);
            return NDIS_STATUS_SUCCESS;
        }
    }

    return NDIS_STATUS_FAILURE;
#else
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(RegisterAddress);
    UNREFERENCED_PARAMETER(RegisterValue);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2WritePhyReg(
    IN PL2_ADAPTER Adapter,
    IN USHORT RegisterAddress,
    IN USHORT RegisterValue
    )
{
#if defined(L2_ENABLE_PHY_CONFIG) && L2_ENABLE_PHY_CONFIG
    ULONG control;
    ULONG poll;

    if ((Adapter == NULL) ||
        (Adapter->Registers == NULL) ||
        (RegisterAddress > L2_MDIO_REG_ADDR_MASK)) {
        return NDIS_STATUS_INVALID_DATA;
    }

    control = ((ULONG)RegisterValue & L2_MDIO_DATA_MASK) |
              (((ULONG)RegisterAddress & L2_MDIO_REG_ADDR_MASK) <<
               L2_MDIO_REG_ADDR_SHIFT) |
              L2_MDIO_START |
              L2_MDIO_SUP_PREAMBLE;

    L2WriteReg32(Adapter, L2_REG_MDIO_CTRL, control);

    for (poll = 0; poll < L2_MDIO_WAIT_TIMES; ++poll) {
        NdisStallExecution(2);
        control = L2ReadReg32(Adapter, L2_REG_MDIO_CTRL);
        if ((control & (L2_MDIO_START | L2_MDIO_BUSY)) == 0) {
            return NDIS_STATUS_SUCCESS;
        }
    }

    return NDIS_STATUS_FAILURE;
#else
    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(RegisterAddress);
    UNREFERENCED_PARAMETER(RegisterValue);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2ForcePhyPowerSaving(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_PHY_CONFIG) && L2_ENABLE_PHY_CONFIG
    NDIS_STATUS status;
    USHORT debugData;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    status = L2WritePhyReg(Adapter, L2_MII_DBG_ADDR, 0);
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2ReadPhyReg(Adapter, L2_MII_DBG_DATA, &debugData);
    }
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2WritePhyReg(Adapter,
                               L2_MII_DBG_DATA,
                               (USHORT)(debugData | 0x1000));
    }
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2WritePhyReg(Adapter, L2_MII_DBG_ADDR, 2);
    }
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2WritePhyReg(Adapter, L2_MII_DBG_DATA, 0x3000);
    }
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2WritePhyReg(Adapter, L2_MII_DBG_ADDR, 3);
    }
    if (status == NDIS_STATUS_SUCCESS) {
        status = L2WritePhyReg(Adapter, L2_MII_DBG_DATA, 0);
    }

    DBGPRINT(("[L2] PHY force power-saving: status=%08X debug0=%04X\n",
              status,
              (status == NDIS_STATUS_SUCCESS) ?
                  (USHORT)(debugData | 0x1000) : 0));
    return status;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2ConfigurePhy(
    IN PL2_ADAPTER Adapter
    )
{
#if defined(L2_ENABLE_PHY_CONFIG) && L2_ENABLE_PHY_CONFIG
    NDIS_STATUS status;
    USHORT debugData;
    USHORT bmcr;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->PhyConfigureSucceeded = FALSE;
    Adapter->PhyDebugData = 0;
    Adapter->PhyAdvertise = L2_MII_ADV_10_100_PAUSE;

    status = L2WritePhyReg(Adapter, L2_MII_DBG_ADDR, 0);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ConfigureFailed;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_DBG_DATA, &debugData);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ConfigureFailed;
    }

    if ((debugData & 0x1000) != 0) {
        debugData = (USHORT)(debugData & ~0x1000);
        status = L2WritePhyReg(Adapter, L2_MII_DBG_DATA, debugData);
        if (status != NDIS_STATUS_SUCCESS) {
            goto ConfigureFailed;
        }
    }
    Adapter->PhyDebugData = debugData;
    NdisStallExecution(1000);

    status = L2WritePhyReg(Adapter,
                           L2_MII_ADVERTISE,
                           Adapter->PhyAdvertise);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ConfigureFailed;
    }

    bmcr = L2_MII_BMCR_RESET |
           L2_MII_BMCR_AN_ENABLE |
           L2_MII_BMCR_AN_RESTART;
    status = L2WritePhyReg(Adapter, L2_MII_BMCR, bmcr);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ConfigureFailed;
    }

    Adapter->PhyConfigureSucceeded = TRUE;
    DBGPRINT(("[L2] PHY configured: debug=0x%04X advertise=0x%04X bmcr=0x%04X\n",
              Adapter->PhyDebugData,
              Adapter->PhyAdvertise,
              bmcr));
    return NDIS_STATUS_SUCCESS;

ConfigureFailed:
    DBGPRINT(("[L2] PHY configure failed: status=0x%08X\n", status));
    return status;
#else
    UNREFERENCED_PARAMETER(Adapter);
    return NDIS_STATUS_NOT_SUPPORTED;
#endif
}

NDIS_STATUS
L2ProbePhy(
    IN PL2_ADAPTER Adapter
    )
{
    NDIS_STATUS status;
    USHORT firstBmsr;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    Adapter->PhyProbeSucceeded = FALSE;
    Adapter->PhyBmcr = 0;
    Adapter->PhyBmsr = 0;
    Adapter->PhyId1 = 0;
    Adapter->PhyId2 = 0;

    status = L2ReadPhyReg(Adapter, L2_MII_BMCR, &Adapter->PhyBmcr);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ProbeFailed;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_BMSR, &firstBmsr);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ProbeFailed;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_BMSR, &Adapter->PhyBmsr);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ProbeFailed;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_PHYSID1, &Adapter->PhyId1);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ProbeFailed;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_PHYSID2, &Adapter->PhyId2);
    if (status != NDIS_STATUS_SUCCESS) {
        goto ProbeFailed;
    }

    Adapter->PhyProbeSucceeded = TRUE;
    DBGPRINT(("[L2] PHY probe: bmcr=0x%04X bmsr=0x%04X id=%04X:%04X\n",
              Adapter->PhyBmcr,
              Adapter->PhyBmsr,
              Adapter->PhyId1,
              Adapter->PhyId2));
    return NDIS_STATUS_SUCCESS;

ProbeFailed:
    DBGPRINT(("[L2] PHY probe failed: status=0x%08X\n", status));
    return status;
}

NDIS_STATUS
L2UpdatePhyLinkState(
    IN PL2_ADAPTER Adapter
    )
{
    NDIS_STATUS status;
    USHORT firstBmsr;
    USHORT currentBmsr;
    USHORT pssr;

    if (Adapter == NULL) {
        return NDIS_STATUS_INVALID_DATA;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_BMSR, &firstBmsr);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_BMSR, &currentBmsr);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    Adapter->PhyBmsr = currentBmsr;
    Adapter->PhyLinkUp =
        ((currentBmsr & L2_MII_BMSR_LINK) != 0) ? TRUE : FALSE;
    Adapter->PhyLinkResolved = FALSE;
    Adapter->PhyFullDuplex = FALSE;
    Adapter->PhySpeedMbps = 0;
    Adapter->PhyPssr = 0;

    if (!Adapter->PhyLinkUp) {
        return NDIS_STATUS_SUCCESS;
    }

    status = L2ReadPhyReg(Adapter, L2_MII_PSSR, &pssr);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    Adapter->PhyPssr = pssr;
    if ((pssr & L2_MII_PSSR_RESOLVED) == 0) {
        return NDIS_STATUS_SUCCESS;
    }

    Adapter->PhyLinkResolved = TRUE;
    Adapter->PhyFullDuplex =
        ((pssr & L2_MII_PSSR_DUPLEX) != 0) ? TRUE : FALSE;

    switch (pssr & L2_MII_PSSR_SPEED_MASK) {
    case L2_MII_PSSR_SPEED_100:
        Adapter->PhySpeedMbps = 100;
        Adapter->LinkSpeed = 1000000;
        break;

    case L2_MII_PSSR_SPEED_10:
        Adapter->PhySpeedMbps = 10;
        Adapter->LinkSpeed = 100000;
        break;

    default:
        Adapter->PhyLinkResolved = FALSE;
        return NDIS_STATUS_NOT_SUPPORTED;
    }

    return NDIS_STATUS_SUCCESS;
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
    /*
     * Hardware stores bytes 2..5 in the low register in network order,
     * and bytes 0..1 in the low half of the high register.  This mirrors
     * LONGSWAP/SHORTSWAP in the Linux atl2 get_permanent_address path.
     */
    mac[0] = (UCHAR)((macHigh >> 8) & 0xFF);
    mac[1] = (UCHAR)(macHigh & 0xFF);
    mac[2] = (UCHAR)((macLow >> 24) & 0xFF);
    mac[3] = (UCHAR)((macLow >> 16) & 0xFF);
    mac[4] = (UCHAR)((macLow >> 8) & 0xFF);
    mac[5] = (UCHAR)(macLow & 0xFF);

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

    if (allZero || allOnes || ((mac[0] & 0x01) != 0)) {
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
