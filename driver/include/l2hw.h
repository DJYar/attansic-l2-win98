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
#define L2_REG_STS_RX_PAUSE     0x1700
#define L2_REG_STS_RXD_OV       0x1704

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

VOID
L2PerformMmioSanityRead(
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
