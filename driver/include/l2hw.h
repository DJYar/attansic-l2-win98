#ifndef _L2HW_H_
#define _L2HW_H_

#include <ndis.h>

typedef struct _L2_ADAPTER L2_ADAPTER;
typedef struct _L2_ADAPTER *PL2_ADAPTER;

/* Minimal register offsets/constants imported for phase-2 scaffolding. */
#define L2_REG_MASTER_CTRL      0x1400
#define L2_MASTER_CTRL_SOFT_RST 0x00000001
#define L2_REG_MAC_STA_ADDR     0x1488
#define L2_REG_MAC_STA_ADDR_HI  0x148C
#define L2_REG_IDLE_STATUS      0x1410
#define L2_REG_PHY_ENABLE       0x140C
#define L2_REG_IRQ_MOD_TIMER    0x1408
#define L2_REG_DMA_STOP_TIMER   0x140E
#define L2_REG_MDIO_CTRL        0x1414
#define L2_MDIO_DATA_MASK       0x0000FFFF
#define L2_MDIO_REG_ADDR_MASK   0x0000001F
#define L2_MDIO_REG_ADDR_SHIFT  16
#define L2_MDIO_RW              0x00200000
#define L2_MDIO_SUP_PREAMBLE    0x00400000
#define L2_MDIO_START           0x00800000
#define L2_MDIO_BUSY            0x08000000
#define L2_MDIO_WAIT_TIMES      10
#define L2_MII_BMCR             0x00
#define L2_MII_BMSR             0x01
#define L2_MII_PHYSID1          0x02
#define L2_MII_PHYSID2          0x03
#define L2_MII_ADVERTISE        0x04
#define L2_MII_DBG_ADDR         0x1D
#define L2_MII_DBG_DATA         0x1E
#define L2_MII_PSSR             0x11
#define L2_MII_BMSR_LINK        0x0004
#define L2_MII_BMSR_AN_COMPLETE 0x0020
#define L2_MII_PSSR_RESOLVED    0x0800
#define L2_MII_PSSR_DUPLEX      0x2000
#define L2_MII_PSSR_SPEED_MASK  0xC000
#define L2_MII_PSSR_SPEED_10    0x0000
#define L2_MII_PSSR_SPEED_100   0x4000
#define L2_MII_ADV_10_100_PAUSE 0x0DE0
#define L2_MII_BMCR_RESET       0x8000
#define L2_MII_BMCR_AN_ENABLE   0x1000
#define L2_MII_BMCR_AN_RESTART  0x0200
#define L2_REG_STS_RX_PAUSE     0x1700
#define L2_REG_STS_RXD_OV       0x1704
#define L2_REG_MAC_CTRL          0x1480
#define L2_REG_MAC_IPG_IFG       0x1484
#define L2_REG_RX_HASH_0         0x1490
#define L2_REG_RX_HASH_1         0x1494
#define L2_REG_MAC_HALF_DUPLEX   0x1498
#define L2_REG_MTU               0x149C
#define L2_REG_DESC_BASE_HI      0x1540
#define L2_REG_TXD_BASE_LO       0x1544
#define L2_REG_TXD_MEM_SIZE      0x1548
#define L2_REG_TXS_BASE_LO       0x154C
#define L2_REG_TXS_MEM_SIZE      0x1550
#define L2_REG_RXD_BASE_LO       0x1554
#define L2_REG_RXD_BUF_NUM       0x1558
#define L2_REG_DMAR              0x1580
#define L2_DMAR_ENABLE           0x01
#define L2_REG_DMAW              0x15A0
#define L2_DMAW_ENABLE           0x01
#define L2_REG_TX_CUT_THRESHOLD  0x1590
#define L2_REG_PAUSE_ON_THRESH   0x15A8
#define L2_REG_PAUSE_OFF_THRESH  0x15AA
#define L2_REG_MB_TXD_WR_IDX     0x15F0
#define L2_REG_MB_RXD_RD_IDX     0x15F4
#define L2_REG_ISR               0x1600
#define L2_REG_IMR               0x1604
#define L2_ISR_TX_STATUS_UPDATE  0x00010000
#define L2_ISR_RX_STATUS_UPDATE  0x00020000
#define L2_ISR_DISABLE_INTERRUPT 0x80000000
#define L2_ISR_CLEARABLE_MASK    0x3FFFFFFF
#define L2_ISR_DATAPATH_EVENTS   \
    (L2_ISR_TX_STATUS_UPDATE | L2_ISR_RX_STATUS_UPDATE)
#define L2_IMR_DATAPATH          L2_ISR_DATAPATH_EVENTS
#define L2_IDLE_STATUS_DMAW      0x00000004
#define L2_IDLE_STATUS_DMAR      0x00000008
#define L2_IDLE_STATUS_DMA_MASK  \
    (L2_IDLE_STATUS_DMAW | L2_IDLE_STATUS_DMAR)
#define L2_MAC_IPG_IFG_DEFAULT   0x60405060
#define L2_MAC_HALF_DEFAULT      0x07A1F037
#define L2_MAC_CTRL_TX_ENABLE    0x00000001
#define L2_MAC_CTRL_RX_ENABLE    0x00000002
#define L2_MAC_CTRL_DUPLEX       0x00000020
#define L2_MAC_CTRL_ADD_CRC      0x00000040
#define L2_MAC_CTRL_PAD          0x00000080
#define L2_MAC_CTRL_PREAMBLE_7   0x00001C00
#define L2_MAC_CTRL_PHY_CLOCK    0x08000000
#define L2_MAC_CTRL_PROMISCUOUS  0x00008000
#define L2_MAC_CTRL_ALL_MULTICAST 0x02000000
#define L2_MAC_CTRL_BROADCAST    0x04000000
#define L2_MAC_CTRL_RETRY_BUF_2  0x20000000
#define L2_MAC_CTRL_TX_BASE      \
    (L2_MAC_CTRL_TX_ENABLE | L2_MAC_CTRL_ADD_CRC | L2_MAC_CTRL_PAD | \
     L2_MAC_CTRL_PREAMBLE_7 | L2_MAC_CTRL_PHY_CLOCK | \
     L2_MAC_CTRL_RETRY_BUF_2)
#define L2_MAC_FRAME_BYTES       1522
#define L2_TX_CUT_DEFAULT        0x00000177
#define L2_IRQ_MOD_DEFAULT       100
#define L2_DMA_STOP_DEFAULT      50000
#define L2_PAUSE_ON_DEFAULT      56
#define L2_PAUSE_OFF_DEFAULT     2
#define L2_TX_SELFTEST_FRAME_LEN 60
#define L2_TX_STATUS_SIZE_MASK   0x000007FF
#define L2_TX_STATUS_OK          0x00010000
#define L2_TX_STATUS_UPDATE      0x80000000
#define L2_RX_STATUS_SIZE_MASK   0x000007FF
#define L2_RX_STATUS_OK          0x00010000
#define L2_RX_STATUS_UPDATE      0x80000000
#define L2_RX_STATUS_BYTES       8

#define L2_TXD_RING_BYTES       (8 * 1024)
#define L2_TXS_RING_COUNT       64
#define L2_TXS_ENTRY_BYTES      4
#define L2_RXD_RING_COUNT       64
#define L2_RXD_ENTRY_BYTES      1536
#define L2_RING_ALLOCATION_BYTES \
    (L2_TXD_RING_BYTES + 7 + \
     (L2_TXS_RING_COUNT * L2_TXS_ENTRY_BYTES) + 7 + \
     (L2_RXD_RING_COUNT * L2_RXD_ENTRY_BYTES) + 127)

ULONG
L2ReadReg32(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    );

VOID
L2WriteReg32(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN ULONG Value
    );

VOID
L2WriteReg16(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN USHORT Value
    );

USHORT
L2ReadReg16(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    );

UCHAR
L2ReadReg8(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset
    );

VOID
L2WriteReg8(
    IN PL2_ADAPTER Adapter,
    IN ULONG RegisterOffset,
    IN UCHAR Value
    );

NDIS_STATUS
L2DiscoverAdapterResources(
    IN PL2_ADAPTER Adapter,
    IN NDIS_HANDLE WrapperConfigurationContext
    );

NDIS_STATUS
L2MapDiscoveredMmio(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2MapHardwareResources(
    IN PL2_ADAPTER Adapter,
    IN NDIS_HANDLE WrapperConfigurationContext
    );

NDIS_STATUS
L2AllocateDmaMemory(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ProgramDmaRings(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ConfigureMacStatic(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2EnableDmaEnginesDiagnostic(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2EnableTxMacDiagnostic(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2RunTxSelfTest(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2EnableRxMacDiagnostic(
    IN PL2_ADAPTER Adapter
    );

VOID
L2PollRxDiagnostic(
    IN PL2_ADAPTER Adapter
    );

VOID
L2PerformMmioSanityRead(
    IN PL2_ADAPTER Adapter
    );

VOID
L2HwQuiesce(
    IN PL2_ADAPTER Adapter
    );

VOID
L2HwShutdown(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2HwReset(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2EnablePhy(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ReadPhyReg(
    IN PL2_ADAPTER Adapter,
    IN USHORT RegisterAddress,
    OUT PUSHORT RegisterValue
    );

NDIS_STATUS
L2WritePhyReg(
    IN PL2_ADAPTER Adapter,
    IN USHORT RegisterAddress,
    IN USHORT RegisterValue
    );

NDIS_STATUS
L2ConfigurePhy(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ForcePhyPowerSaving(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ProbePhy(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2UpdatePhyLinkState(
    IN PL2_ADAPTER Adapter
    );

NDIS_STATUS
L2ReadPermanentMac(
    IN PL2_ADAPTER Adapter,
    OUT PUCHAR Address,
    IN ULONG AddressLength
    );

NDIS_STATUS
L2HwInitialize(
    IN PL2_ADAPTER Adapter
    );

#endif /* _L2HW_H_ */
