# NDIS 5.0 Porting Notes (Windows 2000 DDK)

## Headers used from `/ntddk`

The phase-1 miniport skeleton uses:

- `ndis.h`

This header provides NDIS miniport types, OID constants, and API declarations.

## Required build macros

The `driver/src/sources` file defines:

- `NDIS50`
- `BINARY_COMPATIBLE`

These keep the build aligned with an NDIS 5.0 miniport target.

## NDIS APIs currently used

Current skeleton code uses these NDIS entry/utility APIs:

- `NdisMInitializeWrapper`
- `NdisMRegisterMiniport`
- `NdisTerminateWrapper`
- `NdisAllocateMemoryWithTag`
- `NdisFreeMemory`
- `NdisZeroMemory`
- `NdisMoveMemory`
- `NdisMSetAttributesEx`

## Initialization mode decision (DESERIALIZE flag)

For this conservative bring-up phase, `NdisMSetAttributesEx` is called with
attribute flags set to `0` (no `NDIS_ATTRIBUTE_DESERIALIZE`).

Reason: this keeps synchronization/dispatch behavior in the simpler default
model while the driver is still hardware-free and only servicing basic OIDs.
Additional flags can be enabled later when real TX/RX and interrupt paths are
implemented.

## e100bex influence (high-level, no code copy)

The `e100bex` sample was used as a structural guide for:

- separating `DriverEntry`, initialize/halt, and OID query logic,
- using a switch-based OID dispatch with explicit buffer-length handling,
- maintaining per-adapter state in a single adapter context.

This skeleton intentionally keeps only the minimal subset required for phase-1
and omits hardware/resource handling complexity from the sample.


## PCI interface type choice

`NdisMSetAttributesEx` now uses `NdisInterfacePci` (matching the PCI Ethernet
miniport style used in `e100bex`) instead of a generic placeholder interface
type.


## Link speed placeholder and units

`OID_GEN_LINK_SPEED` is reported in units of 100 bps. The phase-1 placeholder
value is `100000`, which corresponds to 10,000,000 bps (10 Mbps).


## Placeholder MAC

A named constant (`g_L2PlaceholderMac`) is used for the phase-1 locally
administered placeholder MAC address (`02-00-00-00-00-01`) for OID responses.

Read-only hardware MAC extraction is now implemented separately for diagnostics
and validation, but OID responses intentionally continue using the placeholder
until a later enablement phase.


## Miniport build defines required for `ndis.h`

Build errors around `NDIS_MINIPORT_CHARACTERISTICS`, `NdisMInitializeWrapper`,
`NdisMRegisterMiniport`, and `NdisMSetAttributesEx` were caused by missing
miniport-specific preprocessor defines in the build `sources` file.

Following the `e100bex` pattern, the build now explicitly defines:

- `NDIS_MINIPORT_DRIVER=1`
- `NDIS50_MINIPORT=1`

`BINARY_COMPATIBLE` is still kept as before. These defines ensure `ndis.h`
exposes NDIS 5 miniport declarations rather than a more generic NDIS surface.


## Phase-2 hardware bring-up scaffolding

Added `driver/include/l2hw.h` and `driver/src/l2hw.c` with conservative helpers:

- `L2ReadReg32`
- `L2WriteReg32`
- `L2HwReset`
- `L2ReadPermanentMac`
- `L2HwInitialize`

`L2MapHardwareResources` and `L2HwInitialize` remain in-tree for later phases,
but the current `L2MiniportInitialize` path intentionally does not call them to
keep startup hardware-free and reduce Win98 hang risk.

Minimal register constants were imported from Linux reference material for only
reset/control and station-address reads:

- master control register (`0x1400`) and soft reset bit (`0x1`)
- MAC station-address base register (`0x1488`, plus `+4` for upper bytes)

### Still stubbed in phase 2

- PHY initialization
- interrupts
- TX/RX datapath

### Next bring-up step

After proving startup stability in ultra-safe mode, the next step is staged
introduction of tightly scoped read-only register validation before any MMIO
writes are re-enabled.


## Ultra-safe bring-up mode (Win98/ME hang mitigation)

Due to observed system hangs during `MiniportInitialize`, the current phase is
restricted to **hardware-free successful initialization**. The driver keeps hardware inactive and performs only software setup plus
optional resource discovery metadata collection during `MiniportInitialize`.

And explicitly does **not** do:

- any MMIO register writes
- hardware reset sequencing
- PHY access
- interrupt enablement
- hardware-state polling loops

`HardwareReady` remains `FALSE` in this revision, so OID hardware status stays
`NdisHardwareStatusNotReady` until a later validated bring-up phase.

MMIO/resource diagnostic fields remain in adapter context for future phases, but
are not populated by the hardware-free initialization path in this revision.


## Miniport handler contract alignment (e100bex-style)

To better match the base NDIS 5 miniport contract used by `e100bex`, the
registration table now includes conservative diagnostic stubs for handlers that
are commonly present for minimal serialized initialization:

- `CheckForHangHandler`
- `ResetHandler`
- `SendHandler` (serialized miniport style)

Current mode remains **serialized** (`NdisMSetAttributesEx` flags = `0`, no
`NDIS_ATTRIBUTE_DESERIALIZE`), which is consistent with using `SendHandler`
instead of `SendPacketsHandler` for this minimal bring-up phase.

All newly added handlers are hardware-free stubs and do not touch MMIO,
interrupts, PHY, or datapath setup.


## Win98 successful-init contract diagnostics

Current observations during Win98 bring-up:

- `NdisMSetAttributesEx` followed by immediate `NDIS_STATUS_FAILURE` is stable.
- Crashes occur only when `MiniportInitialize` returns success and NDIS starts
  interacting with the adapter contract.

To improve contract completeness while staying hardware-free, the miniport now
adds serialized diagnostic stubs for:

- `CheckForHangHandler`
- `ResetHandler`
- `SendHandler`

`ReturnPacketHandler` was removed for this conservative serialized path to keep
only the minimal set required for the current no-receive/no-indication phase.

The driver now always follows the stable success-return path after
`NdisMSetAttributesEx` (no failure-injection switch).


## Resource-discovery phase (with optional mapping-only MMIO)

A compile-time switch controls conservative resource discovery:

- `L2_ENABLE_RESOURCE_DISCOVERY=1` (current default in `driver/src/sources`)
- `L2_ENABLE_MMIO_MAPPING=1` (enables mapping-only MMIO after discovery)

When enabled, `MiniportInitialize` calls `NdisMQueryAdapterResources` and
records resource metadata only:

- `ResourceCount`
- `SelectedResourceIndex`
- `SelectedResourceType`
- `SelectedResourceLength`
- `MmioPhysicalBaseLow`
- `MmioPhysicalBaseHigh`

In this phase the driver still does **not**:

- read/write registers
- reset hardware
- touch PHY
- enable interrupts

If resource discovery fails or no usable memory resource is found, initialization
fails conservatively.


## Mapping-only MMIO phase

Resource discovery is now followed by **MMIO mapping only** in
`MiniportInitialize`:

- `NdisMQueryAdapterResources` (discover)
- `NdisMMapIoSpace` (map selected memory resource)

Recorded fields now include:

- `Registers`
- `MemoryLength`
- `MmioMappingSucceeded`
- `MmioPhysicalBaseLow` / `MmioPhysicalBaseHigh`

This phase still keeps hardware inactive except for tightly scoped controlled
read-only probes:

- fixed-set sanity register reads
- station-address register reads for MAC extraction
- no register writes
- no reset
- no PHY
- no interrupts

`HardwareReady` remains `FALSE` by design.


## First read-only MMIO sanity check

After successful MMIO mapping, the driver now performs a small fixed set of
read-only sanity accesses and records:

- `SanityReadOffsets[3]`
- `SanityReadValues[3]`
- `SanityReadSuccessCount`
- `SanityReadSucceeded`

Chosen conservative read candidates from Linux `atl2` register groups:

- `REG_IDLE_STATUS` (`0x1410`) — block idle/status register
- `REG_STS_RX_PAUSE` (`0x1700`) — RX statistics counter
- `REG_STS_RXD_OV` (`0x1704`) — RX descriptor overflow statistics

These are status/counter style reads and avoid reset/control, PHY/MDIO,
and interrupt acknowledge paths.

Validation rule: if all performed sanity reads return `0xFFFFFFFF`, the
sanity check is treated as failed (`SanityReadSucceeded = FALSE`).

All register writes remain disabled.


## Read-only MAC extraction

After successful MMIO mapping and sanity reads, the driver performs read-only
MAC extraction from Linux `atl2` station-address registers:

- `REG_MAC_STA_ADDR` (`0x1488`) for MAC bytes 0..3
- `REG_MAC_STA_ADDR + 4` (`0x148C`) for MAC bytes 4..5

Safety rationale:

- read-only accesses only
- no register writes
- no reset sequencing
- no enabling/control bits
- no PHY/MDIO access
- no interrupt enable/ack writes

Validation rule:

- if extracted MAC is all `00` or all `FF`, it is invalid and
  `MacReadSucceeded = FALSE`
- otherwise `MacReadSucceeded = TRUE`

The extracted MAC is stored in `PermanentMac[6]` for diagnostics only in this
phase. OID permanent/current address reporting remains on the placeholder MAC.


## Initialization debug output (DebugView)

Initialization-only debug lines are emitted through `DBGPRINT` so bring-up can
be observed in DebugView on Windows 98 without changing hardware behavior:

- MMIO mapping:
  - `[L2] MMIO mapped: phys=%08X:%08X len=%u virt=%p`
- each sanity read:
  - `[L2] SANITY read: reg=0x%X val=0x%08X`
- MAC extraction:
  - `[L2] MAC: %02X:%02X:%02X:%02X:%02X:%02X`
  - `[L2] MAC INVALID` (when all `00` or all `FF`)

This output is limited to a few lines during initialization and uses no dynamic
allocation, delay loops, or additional hardware writes.
