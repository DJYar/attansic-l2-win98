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

Read-only hardware MAC extraction is implemented with the register byte order
used by Linux `atl2`.  A valid unicast address replaces the placeholder for
`OID_802_3_CURRENT_ADDRESS` and `OID_802_3_PERMANENT_ADDRESS`; invalid, zero,
all-ones, or multicast values leave the locally administered placeholder active.


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

Startup stability and read-only register validation have been proven on the
target.  The current step is the bounded soft-reset validation described below.


## Earlier ultra-safe bring-up mode (Win98/ME hang mitigation)

Due to observed system hangs during `MiniportInitialize`, the initial phase was
restricted to **hardware-free successful initialization**. The driver kept hardware inactive and performed only software setup plus
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

All register writes remained disabled in this stage.


## Read-only MAC extraction

After successful MMIO mapping and sanity reads, the driver performs read-only
MAC extraction from Linux `atl2` station-address registers:

- `REG_MAC_STA_ADDR` (`0x1488`) for MAC bytes 2..5
- `REG_MAC_STA_ADDR + 4` (`0x148C`) for MAC bytes 0..1

The actual register layout follows the Linux `LONGSWAP`/`SHORTSWAP` decoding:

- `0x1488` contains MAC bytes 2..5 in network order
- the low 16 bits of `0x148C` contain MAC bytes 0..1 in network order

Safety rationale:

- read-only accesses only
- no register writes
- no reset sequencing
- no enabling/control bits
- no PHY/MDIO access
- no interrupt enable/ack writes

Validation rule:

- if extracted MAC is all `00`, all `FF`, or multicast, it is invalid and
  `MacReadSucceeded = FALSE`
- otherwise `MacReadSucceeded = TRUE`

The extracted MAC is stored in `PermanentMac[6]`.  Once validated, it is also
used for permanent/current Ethernet address OID reporting; otherwise the
placeholder remains active.


## Win98 protocol binding compatibility

The binary registers an NDIS 5.0 miniport, but the Win98/Memphis INF binding
surface must use the legacy `ndis3` upper range.  This follows the Win9x section
of the DDK `NET557ex.INF` sample:

- `Ndi\\Interfaces\\UpperRange` and `DefUpper` are `ndis3`
- `NDIS\\MajorNdisVersion` is `3`
- `NDIS\\MinorNdisVersion` is `0x0A`
- `Ndi\\Install` registers the `ndis3` install section

Advertising `ndis5` here prevents Win98 TCP/IP from offering or creating a
binding even though the miniport itself loads successfully.


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


## Bounded soft-reset validation stage

After successful read-only MMIO and MAC validation, the current diagnostic
build enables one hardware write with `L2_ENABLE_SOFT_RESET=1`:

- write `MASTER_CTRL_SOFT_RST` to `REG_MASTER_CTRL` (`0x1400`)
- wait 1 ms
- poll read-only `REG_IDLE_STATUS` (`0x1410`) at most ten times, 1 ms apart

The result is retained in `ResetAttempted`, `ResetSucceeded`,
`ResetIdleStatus`, and `ResetPollCount`.  A timeout is diagnostic and does not
fail miniport initialization, allowing Win98 to boot and expose debug output.
`HardwareReady` remains `FALSE`; PHY, DMA, interrupts, TX, and RX remain
disabled.


## PHY enable and read-only MDIO probe stage

After the bounded reset was validated on the target, the next diagnostic build
enables `L2_ENABLE_PHY_PROBE=1` and performs:

- a 16-bit write of `1` to `REG_PHY_ENABLE` (`0x140C`) before reset
- the already validated bounded soft reset
- MDIO reads of `BMCR`, `BMSR` twice, `PHYSID1`, and `PHYSID2`

Each MDIO transaction polls `REG_MDIO_CTRL` at most ten times with a 2 us stall.
The second `BMSR` read handles its latch-low link-status behavior.  This stage
does not write any PHY register, restart autonegotiation, configure MAC/DMA, or
enable interrupts.  Probe results are emitted as a single debug line and stored
in the adapter context; `HardwareReady` remains `FALSE`.


## PHY wake and autonegotiation stage

When a connected cable still reported `BMSR` link-down, the next build added
the Linux `atl2` PHY configuration sequence before MAC reset:

- select PHY debug register zero and clear its `0x1000` power-save bit if set
- advertise 10/100 half/full plus symmetric/asymmetric pause (`0x0DE0`)
- write `BMCR_RESET | BMCR_AN_ENABLE | BMCR_AN_RESTART` (`0x9200`)

The PHY link-change interrupt write is deliberately omitted.  The serialized
`CheckForHang` callback reads `BMSR` twice and periodically logs link and
autonegotiation state.  It does not report media connected to NDIS yet because
MAC/DMA and TX/RX are still disabled.

After link and autonegotiation were confirmed on the target, the status poll
was extended to read PHY-specific status register `0x11`.  Its resolved,
speed, and duplex bits update the cached NDIS link-speed value (in 100 bps
units), while media state deliberately remains disconnected until the datapath
is operational.


## DMA allocation-only stage

The next build allocates one uncached NDIS shared-memory block and lays out the
three ATL2 rings using the same sizes and physical alignment as Linux `atl2`:

- 8192-byte TX data ring, aligned to 8 bytes
- 64 four-byte TX status entries, aligned to 8 bytes
- 64 1536-byte RX descriptors, positioned so descriptor byte 8 is aligned to
  128 bytes as required by the hardware layout

The computed physical addresses are logged and validated to remain within the
single 4-GiB address window selected by ATL2's shared high-address register.
Allocation failure remains non-fatal for this diagnostic stage.  No DMA base,
ring-size, mailbox, MAC-control, or interrupt register is written, and
`HardwareReady` and NDIS media state remain false.


## Ring programming and readback stage

After the target confirmed the shared allocation and all three alignments, the
next diagnostic build writes only the descriptor address, size, and zeroed
mailbox-index registers.  It immediately reads each register back and requires
an exact match.  The DMA read/write enable registers, MAC control, interrupt
mask, and interrupt status remain untouched, so the controller cannot consume
the rings yet.  A readback mismatch remains non-fatal and leaves
`DmaRingsProgrammed` false.


## DMA enable with MAC disabled

After register readback was proven, the next diagnostic build enables the DMA
read and write engines while keeping both MAC directions and all interrupts
disabled.  The driver verifies the enable-bit readback and waits at most 10 ms
for the DMA bits in `IDLE_STATUS` to clear.  Failure immediately disables both
engines.  The halt path also disables both engines and waits for idle before it
releases shared ring memory.  No packets are submitted or indicated in this
stage, so NDIS media state and `HardwareReady` remain false.

Before DMA enable, the current build also writes and reads back the Linux ATL2
static MAC defaults: station address, empty multicast hash, IPG/IFG,
half-duplex timing, 1522-byte hardware frame limit, TX cut-through threshold,
flow-control watermarks, and timer preload values.  `MAC_CTRL` is explicitly
written as zero and verified, so this configuration step cannot start TX or RX.


## TX-MAC enable with an empty transmit ring

Once link resolution, static configuration, and idle DMA were independently
validated, the next diagnostic build enables only the TX half of `MAC_CTRL`.
The value includes the PHY clock, resolved duplex, CRC insertion, padding,
seven-byte preamble, and the Linux retry-buffer setting.  The RX enable bit and
flow-control bits remain clear, and the NDIS send handler still rejects every
packet, so the TX ring and mailbox remain empty.  On halt, MAC control is
cleared and read back before the DMA engines are stopped.


## One-frame TX self-test

After TX-only MAC enable was validated, the next build submits exactly one
60-byte broadcast Ethernet frame with experimental EtherType `0x88B5`.  The
frame is copied behind a four-byte ATL2 TX header, the mailbox is advanced to
16 dwords, and the first TX-status entry is polled for at most 10 ms in 100 us
steps.  Success requires the hardware update bit, success bit, and reported
length of 60.  The normal NDIS send handler remains disabled, so no other
packet can enter the ring during this test.


## RX polling self-test

After the one-frame TX status was validated, the next build enables the RX MAC
for station-address unicast and broadcast traffic.  It does not indicate data
to NDIS yet.  The serialized hang-check callback polls at most 64 descriptors,
logs the first frame's status and Ethernet header, clears each consumed update
flag, and advances the RX mailbox index.  This bounds work per callback and
prevents the diagnostic ring from remaining full while its format is tested.


## Minimal polled NDIS datapath

After both raw directions were proven, the next build reports media connected,
copies each `MiniportSend` packet into the ATL2 byte ring, and synchronously
waits at most 10 ms for its TX status.  Receive descriptors with valid length
and status are indicated to NDIS as complete Ethernet lookahead data before the
descriptor is released.  RX is still serviced by the low-frequency serialized
hang-check callback, so this stage is intended to validate TCP/IP integration,
not performance.  Interrupts remain disabled.


## Periodic RX service

Once the polled datapath completed DHCP without errors, RX servicing was moved
from the roughly two-second hang-check cadence to an NDIS miniport periodic
timer with a 10 ms requested period.  The timer is initialized after miniport
attributes are registered, starts only after the full datapath reports ready,
and is cancelled before MAC/DMA shutdown and ring release.  Hardware
interrupts remain disabled in this stage.


## Masked interrupt registration

After periodic RX service was proven under real ICMP and HTTP traffic, resource
discovery was extended to retain the PCI interrupt vector, level, sharing, and
trigger mode.  The driver explicitly writes and verifies `IMR=0`, then registers
dummy ISR/DPC callbacks with NDIS.  The ISR never claims an interrupt and all
controller interrupt sources remain masked, so the 10 ms timer continues to
service the operational datapath.  Halt deregisters the IRQ before hardware
shutdown.


## RX-only hardware interrupt stage

After masked IRQ registration was proven on the target, only the receive-status
update source (`0x00020000`) is unmasked.  The shared, level-triggered ISR
rejects unrelated IRQ 11 activity, acknowledges the ATL2 source with the
documented interrupt-disable bit, and queues the miniport DPC.  The DPC drains
RX and writes zero to ISR to re-enable delivery.  The 10 ms timer remains active
as a recovery fallback, while TX completion, PHY, and error interrupts remain
masked.

After sustained web-radio traffic confirmed one-for-one ISR/DPC delivery, the
periodic receive timer was reduced from 10 ms to a 250 ms watchdog.  Normal RX
is interrupt-driven; the watchdog records any descriptors it has to recover.
Per-drain logging is limited to 1024-packet progress milestones so debug output
does not itself become a load during sustained traffic.

The PHY poll now also tracks cable transitions after the datapath is ready.
Loss of link is reported to NDIS with `NDIS_STATUS_MEDIA_DISCONNECT`; a later
resolved link reports `NDIS_STATUS_MEDIA_CONNECT`.  MAC, DMA, rings, and the RX
interrupt remain configured across the transition, so traffic can resume
without reinitializing the adapter.


## Standard NDIS statistics and packet-filter validation

The miniport now exposes the five baseline general-statistics OIDs using its
software datapath counters:

- `OID_GEN_XMIT_OK`
- `OID_GEN_RCV_OK`
- `OID_GEN_XMIT_ERROR`
- `OID_GEN_RCV_ERROR`
- `OID_GEN_RCV_NO_BUFFER`

Malformed or hardware-rejected RX descriptors increment the receive-error
counter.  The fixed receive ring does not currently expose a reliable software
signal for allocation failure, so the no-buffer counter remains zero.  A final
statistics line is emitted during halt before the hardware state is cleared.

Packet-filter requests are limited to directed, multicast, and broadcast
traffic.  Unsupported modes such as promiscuous or all-multicast are rejected
instead of being silently reported as active.  This step does not alter the
already validated MAC, DMA, or interrupt register programming.

The checked build was validated on the Eee target after a cold miniport load.
Both the USB management link and the wired L2 link returned after reboot, and
the wired adapter retained its DHCP address.  A simultaneous stress pass of
2,000 1400-byte echo requests to the adapter and 30 outbound Internet echo
requests completed with zero packet loss.  Windows `netstat -e` consumed the
new statistics OIDs and reported zero receive errors and zero discarded
packets.  The validated binary is 16,749 bytes with SHA-256
`3bc759dfc68b4adc3b2300c8069a29edf12d339791cb3a5454b6fbcd0e527bca`.


## Single-outstanding asynchronous TX

The synchronous 10 ms status poll in `MiniportSend` has been replaced by a
single-outstanding asynchronous path.  A submitted packet is retained with
`NDIS_STATUS_PENDING`, and the transmit-status update interrupt (`0x00010000`)
is enabled alongside the already proven receive-status update interrupt.
The miniport DPC validates the hardware status and packet length, advances the
status ring, and calls `NdisMSendComplete`.

Only one packet is admitted at a time during this first interrupt-driven TX
stage.  A second send receives `NDIS_STATUS_RESOURCES`; completion notifies
NDIS through `NdisMSendResourcesAvailable`.  The existing 250 ms timer also
checks pending TX status as an interrupt-loss fallback.  Four missed timer
ticks fail the packet and stop the TX path rather than reusing an uncertain
ring position.  Halt completes any remaining packet with failure before ring
memory is released.

The checked async-TX build was validated across two target boots.  Both wired
and USB links returned, and EeeAgent 0.4 started automatically through the
pre-logon `RunServices` key.  Four simultaneous streams delivered 8,000
1400-byte echo requests with zero loss while the Eee sent Internet echo
requests.  Average wired round-trip time remained about 1.03 ms, which also
confirms that TX completion came from the hardware interrupt rather than the
250 ms watchdog.  Windows reported zero receive errors and zero discarded
packets after the run.  The validated 17,325-byte binary has SHA-256
`63baef7b2bb820330280bec8adfdcbfdf2f71e7239a0ca0604adfa2f0ef56379`.


## Selective multicast receive filter

The miniport now implements `OID_802_3_MULTICAST_LIST` and advertises space
for 32 Ethernet multicast addresses.  Lists with a non-six-byte-aligned size,
too many entries, unicast entries, or the broadcast address are rejected.
Packet-filter and address-list requests received before RX startup are retained
and applied once the MAC is ready.

The two 32-bit receive-hash registers are built with the same little-endian
Ethernet CRC and bit reversal used by the Linux ATL2 driver.  The hash registers
are cleared when multicast is disabled, and the broadcast bit follows the NDIS
packet filter.  Promiscuous and all-multicast MAC bits are explicitly cleared;
those unsupported NDIS modes continue to be rejected.  Register writes are
read back before an OID request is completed successfully.

`tools/mcasttest.dpr` is a Delphi 6 / Winsock 1.1 target-side diagnostic.  It
joins `239.255.66.66` specifically on the wired address and waits for UDP port
50000, forcing Windows to program the multicast-list OID without depending on
whether the Windows 98 `ping` utility answers multicast echo requests.

The checked build acquired its wired DHCP lease automatically after reboot.
The multicast diagnostic received the first 20-byte datagram sent from
`192.168.0.197` to `239.255.66.66:50000`.  During the following bidirectional
regression, four simultaneous streams delivered 2,000 1400-byte echo requests
with zero loss while Windows completed 25 of 25 Internet echo requests.
Receive errors and discards remained zero, and the pre-existing aggregate
three transmit errors did not increase.  The validated 18,413-byte binary has
SHA-256
`5f4aff79b5eed35207d5e4e35261e60904f168166656dcdd1a9b6da83e12722d`.


## Bounded asynchronous TX queue

The interrupt-driven transmit path now uses the full 64-entry hardware status
ring instead of allowing only one outstanding packet.  Separate producer and
consumer indices track cleared status slots, while byte-ring write and reclaim
offsets prevent an uncompleted frame from being overwritten.  One status slot
and one byte are always reserved to distinguish full and empty rings, so at
most 63 packets may be outstanding; large Ethernet frames are normally limited
first by the 8 KiB TX data ring.

Each pending status slot retains its NDIS packet, expected length, and watchdog
start tick.  The TX DPC drains completed statuses in order and returns resources
to NDIS after it has reclaimed ring space.  A watchdog timeout now fails every
queued packet and permanently stops the TX path because the DMA consumer
position is no longer safe to infer.  Halt likewise completes every retained
packet before the rings are released.

The checked queue build acquired DHCP automatically and preserved the selective
multicast test.  Four simultaneous streams delivered 8,000 1400-byte echo
requests in approximately four seconds with zero loss and average round-trip
times between 0.99 and 1.17 ms.  Windows simultaneously completed 30 of 30
Internet echo requests; receive errors and discards remained zero, and the
aggregate transmit-error count did not increase.  The validated 18,925-byte
binary has SHA-256
`1d331fc9abfc8b26c14c15ee9c2f68cc5abcd16827493a925dcdaac50d29b284`.


## Bounded autonomous telemetry

The 250 ms watchdog now emits one compact telemetry record approximately every
20 seconds after the datapath becomes ready.  It reports link state, TX/RX
success and error counts, current and maximum TX queue depth, ISR/DPC counts,
TX interrupt and watchdog completions, TX timeouts, and RX watchdog recovery.
The interval is deliberately coarse so checked builds remain useful during
long-running traffic without debug output becoming a performance load.

On the target, DebugView 4.62 is started from `RunServices` with a two-megabyte
wrapping log, and EeeAgent 0.5 can retrieve the file or launch diagnostics
without inheriting its command-output capture handle.  This provides unattended
post-login diagnostics even when the Windows network-password dialog remains
open.

After a reboot, the first autonomous log showed a real maximum TX queue depth
of four packets, equal ISR/DPC counts, zero TX watchdog completions, zero TX
timeouts, and zero RX watchdog recoveries.  A subsequent four-stream run
delivered 8,000 1400-byte echo requests in about four seconds with zero loss.
Telemetry then reported `tx=8020/0`, `max=4`, `irq=16010/16010`,
`txirq=8020`, `txwd=0`, `txto=0`, `rx=8037/0`, and `rxwd=0/0`.
The validated 19,181-byte binary has SHA-256
`afa678e8c435be7af87a3e262ec8663eb0e5680b2b3dabfef4786d075aa9f185`.


## Asynchronous runtime reset

`MiniportReset` is no longer a success-returning stub.  For a fully initialized
adapter it masks interrupts, stops new datapath activity, completes retained TX
packets with failure, disables MAC/DMA, starts the hardware soft reset, and
returns `NDIS_STATUS_PENDING`.  The existing 250 ms miniport timer advances a
bounded reset state machine, so the reset callback itself does not spin while
waiting for hardware or PHY state.

After the controller reports idle, the driver clears the existing shared DMA
allocation without freeing it, restores descriptor registers and static MAC
configuration, reenables DMA, and waits for PHY resolution.  It then repeats
the validated TX self-test, enables RX, restores the saved NDIS packet filter
and multicast hash, and rearms RX/TX interrupts before calling
`NdisMResetComplete`.  Idle and link waits have fixed limits, and a failure in
any restore step completes the reset with `NDIS_STATUS_HARD_ERRORS`.

A dedicated checked self-test build asked NDIS to invoke exactly one reset
after autonomous logging was active.  The target log captured every restore
stage, including two multicast addresses and their hash, and ended with
`resets=1 failures=0`.  During the approximately 1.67-second PHY/reset window,
a continuous 4 Hz echo stream lost 7 of 240 packets and then recovered without
reboot or DHCP renewal.  Internet echo completed 5 of 5 afterward, with zero
receive errors and discards.  The production build has no automatic reset
trigger; after a clean reboot it completed a four-stream 2,000-packet
regression with zero loss and continued to report `reset=0/0`.  The validated
production binary is 20,653 bytes with SHA-256
`aa169445b6b3ebc194cdee2a47f3c6757903ba84f20651a02a4bda4d6a42f8e9`.


## Warm-shutdown quiesce

The miniport now registers an NDIS adapter shutdown handler after successful
initialization and deregisters it before `MiniportHalt` releases the adapter.
The handler does not free resources.  It calls the same bounded hardware
quiesce helper used by the halt path: mask controller interrupts, disable both
MAC directions, stop read and write DMA, and poll DMA idle for at most 10 ms.
This prevents the controller from retaining bus-master activity into a warm
restart while keeping shutdown work independent of NDIS resource teardown.

The checked production build completed three consecutive agent-requested warm
restarts.  After every restart the wired adapter returned with its permanent
address and DHCP lease and immediately reached the Internet.  A four-stream
2,000-packet, 1400-byte regression completed with zero loss; telemetry reported
`tx=2034/0`, `irq=4057/4057`, `txwd=0`, `txto=0`, and `rx=2047/0`.  A final
post-restart check completed 100 of 100 large local echo requests and 5 of 5
Internet requests.  DebugView append logging preserved four distinct boot
sessions.  One intermediate boot displayed four Windows aggregate transmit
errors instead of the usual three, while driver telemetry stayed at zero; the
exact final build returned to three, so this remains an observation for later
soak testing rather than a demonstrated driver error.  Its final 2,000-packet
regression also had zero loss and zero RX errors; the bounded RX watchdog
recovered one descriptor (`rxwd=1/1`) during that burst.

The validated 20,941-byte production binary has SHA-256
`2adee644eae560bb33d5aea9e212087db7bf2ad58044457cabdb1803429fe79b`.


## Repeated runtime-reset soak

The checked reset trigger can now issue a compile-time bounded sequence rather
than only one request.  Production keeps `L2_ENABLE_RESET_SELFTEST` disabled;
the additional request counter and timing constants are inert unless a checked
test build explicitly enables the trigger.

A dedicated build requested 20 asynchronous resets, beginning after 20 seconds
and then at approximately 10-second intervals.  All 20 completed successfully
with `failures=0`.  Every cycle restored PHY negotiation, the DMA rings, the TX
self-test, both MAC directions, two multicast addresses and their hash filter,
and RX/TX interrupts.  Final soak telemetry showed equal ISR/DPC counts, zero
TX errors, zero TX watchdog completions, zero TX timeouts, zero RX errors, and
zero RX watchdog recoveries.

A simultaneous 4 Hz stream sent 1,000 1400-byte echo requests over 250.9
seconds.  It received 866 and lost 134 (13.4 percent), averaging 6.7 missed
requests per roughly 1.67-second reset window, with no loss outside the reset
windows.  The maximum RTT of a delivered request was 12.28 ms.  The link and
Internet remained usable immediately after the final cycle without reboot or
DHCP renewal.

The trigger-disabled production rebuild was then restored and rebooted.  It
completed a four-stream 2,000-packet regression with zero loss and reported
`tx=2016/0`, `irq=4029/4029`, `rx=2017/0`, `rxwd=0/0`, and `reset=0/0`.
Internet echo completed 5 of 5 and Windows reported zero receive errors.  This
validated 20,941-byte production binary has SHA-256
`8c87517e68de93da0cd683a43f32f9f9036ed6fa40229f5357baf7f79e1ef8ea`.


## Software PHY link-flap test

A compile-time-disabled checked self-test now exercises the media-status path
without requiring a physical cable operation.  An initial experiment used the
standard MII BMCR power-down bit.  It stopped packet traffic, but this embedded
Attansic PHY continued to return the previous link-up BMSR/PSSR values.  The
bounded test therefore timed out, restored autonegotiation, and correctly
recorded `phytest=0/1`; 46 of 240 echo requests were lost during that deliberate
9.5-second timeout.  This method is not used by the final test path.

The working path uses the Attansic L2 force-power-saving debug-register
sequence also present in the Linux `atl2_force_ps` implementation: set debug
page 0 bit `0x1000`, program page 2 to `0x3000`, and clear page 3.  The existing
PHY configuration path reverses the power-saving state and restarts autoneg.
This time BMSR changed from `0x796D` to `0x7949`; the miniport indicated
`NDIS_STATUS_MEDIA_DISCONNECT`, reenabled and configured the PHY, resolved
100-full link, and indicated `NDIS_STATUS_MEDIA_CONNECT` about 1.91 seconds
later.  The test ended with `phytest=1/0`, no reset, no TX/RX errors, equal
ISR/DPC counts, and no watchdog intervention.  A simultaneous 240-packet 4 Hz
stream lost 16 requests only around the deliberate link transition and then
continued normally without DHCP renewal.

The trigger-disabled production rebuild was restored and rebooted.  It passed
another four-stream 2,000-packet regression with zero loss and reported
`tx=2016/0`, `irq=4028/4028`, `rx=2014/0`, `rxwd=0/0`, `reset=0/0`, and
`phytest=0/0`.  Internet echo completed 5 of 5 and Windows reported zero
receive errors.  This validated 21,069-byte production binary has SHA-256
`8ecab6527b896827319d4f0a71a181e0a571f5632eacfee1afed1935c808d061`.


## Physical cable-flap soak

The trigger-disabled production build was exercised with five manual cable
removal/insertion cycles while large echo requests ran continuously.  Every
physical removal changed BMSR from `0x796D` to `0x7949`, cleared resolved link
state, and caused an NDIS media-disconnect indication.  Every insertion
eventually resolved at 100 Mbps full duplex and caused an NDIS media-connect
indication without reset, reboot, or DHCP renewal.

The first insertion produced two additional short down/up transitions before
settling; the miniport reported and recovered from all three transitions.  The
remaining four cycles had one disconnect and one connect each.  Measured time
from the driver's first disconnect indication to its final connect indication
was approximately 22.9, 9.5, 13.4, 19.1, and 7.6 seconds respectively.  These
intervals include the user's roughly five-second unplugged period and the
two-second CheckForHang polling granularity.

The first continuous stream sent 720 1400-byte requests across four cycles and
received 518, with 202 expected losses during unplugged/autoneg windows.  A
separate 240-request stream around the fifth cycle received 204.  After the
last recovery, Internet echo completed 10 of 10.  Final telemetry reported
`tx=2785/0`, `irq=5789/5789`, `txwd=0`, `txto=0`, `rx=2972/0`, and
`rxwd=0/0`; Windows reported zero receive errors and zero discards, while the
pre-existing aggregate count of three transmit errors did not increase.


## Release-candidate identity cleanup

The runtime identity no longer exposes the original bring-up label.  The
`OID_GEN_VENDOR_DESCRIPTION` response now matches the generic INF device name,
`Attansic L2 Fast Ethernet Adapter`, so tools such as WINIPCFG do not display a
phase-1 skeleton description.

`OID_GEN_VENDOR_ID` is now formed from the first three bytes of the adapter's
validated permanent MAC address with the required zero fourth byte, following
the NDIS sample-driver convention.  On the validated Eee PC this reports the
Attansic OUI `00-1F-C6` instead of the locally administered placeholder OUI.

The first reboot of this checked build showed the final description, the
correct permanent MAC, an active link, and a working DHCP lease.  The legacy
Win98 DHCP client initially installed a classful `/8` mask for the
`192.168.0.121` lease; `ipconfig /renew_all` immediately replaced it with the
DHCP-provided `/24` mask.  This matches the earlier observed need to renew the
lease after some boots and did not involve a driver reset or media transition.

After renewal, 100 consecutive 1400-byte echo requests to the gateway and 10
Internet echo requests completed without loss.  Windows interface statistics
still reported zero receive errors and zero discarded packets; the historical
aggregate of three transmit errors did not increase.


## Live disable/enable lifecycle test

Windows 98 Device Manager accepted `Disable in this hardware profile` without
requesting a reboot.  After the System Properties window closed, the adapter
disappeared from `ipconfig` and the route table while the independent USB
management interface remained active.  The driver log confirmed a real Halt:
the shutdown handler was deregistered, the polling timer was cancelled, the
interrupt was deregistered, and the old adapter context ended with
`tx_ok=180`, `tx_err=0`, `rx_ok=338`, `rx_err=0`, and `rx_no_buffer=0`.

Clearing the disable flag and closing System Properties caused a new
Initialize without reboot.  MMIO mapping, PHY negotiation, ring setup, and the
NDIS datapath all completed again.  DHCP immediately restored
`192.168.0.121/24`; 20 consecutive 1400-byte gateway echo requests and five
Internet echo requests completed without loss.  New-context telemetry showed
equal ISR/DPC counts, zero TX/RX errors, zero watchdog recovery, and no reset.

The aggregate Windows transmit-error counter increased by six exactly while
the adapter was administratively absent and then remained stable.  Since the
old miniport context logged `tx_err=0` at Halt and the new context also logged
zero errors, these were protocol-stack send attempts rejected during device
removal rather than hardware or driver transmit failures.


## Release-candidate mixed-traffic soak

The identity-cleaned checked RC was rebooted and used for approximately two
hours with continuous Winamp Internet radio, simultaneous browsing and general
Windows activity.  The physical cable was also removed and restored, and the
user verified traffic failover through the separate Raspberry Pi USB network.

The captured driver interval covers 6236 seconds.  Its final telemetry was
`tx=110462/0`, `pending=0`, `max=3`, `irq=288302/288302`, `txirq=110429`,
`txwd=33`, `txto=0`, `rx=180983/0`, `rxwd=36/50`, `reset=0/0`, and
`phytest=0/0`.  Thus every ISR had a matching DPC, no transmit timed out, no
driver TX/RX error occurred, and neither reset path was needed.  The 250 ms
fallback timer completed 33 transmits and recovered 50 receives across 36
polls, less than 0.03 percent of traffic in either direction.

The current session recorded three complete media down/up pairs.  Their
disconnect-to-connect intervals were approximately 3.8, 126.0, and 9.5
seconds; the long interval includes the deliberate cable-out/failover test.
All three returned at 100 Mbps full duplex without DHCP intervention.

At the end of the run Windows reported roughly 260 MB received, more than
180,000 received unicast packets, zero receive errors, and zero discarded
packets.  Ten 1400-byte gateway echo requests and ten Internet echo requests
then completed without loss.  The aggregate Windows transmit-error counter
was stable during the final checks; its increase while the cable was absent
was not reflected in the miniport's zero hardware-error count.


## Free release-candidate build

`tools/build-win10.cmd` now accepts an explicit `free` argument while retaining
checked as its default.  It selects `libfre` and verifies a newly produced
`objfre\i386\l2ndis.sys`, preventing an older checked output from hiding a
failed release build.

The first free RC built successfully with the archived Windows 2000 DDK and
VC6 toolchain.  It is 14,573 bytes and has SHA-256
`5e99af771d015d8b0c877394603bcf16a8cfebc9a4c0862a98ab60ceaf15d1a0`.
Static inspection confirms the final `Attansic L2 Fast Ethernet Adapter`
identity remains present while all `[L2]` checked-build format strings and the
`DbgPrint` import are absent.  Runtime reset and PHY self-test triggers remain
disabled exactly as in the validated checked RC.


## Clean free-RC installation on Windows 98 SE

A clean installation of the free RC exposed an important Win98 SetupX cache
behavior.  Removing the adapter from Device Manager or the Network control
panel did not prevent automatic reuse of an older INF.  SetupX had copied that
INF to `C:\WINDOWS\INF\OEM3.INF` and compiled it as `OEM3.PNF`; the cached
version still declared `UpperRange=ndis5`, `DefUpper=ndis5`, and
`Ndi\Install\ndis5`.  Backup INF files left under the driver source directory
and temporary copies in `C:\WINDOWS\TEMP` and the Internet Explorer cache also
remained eligible during automatic detection.

The reliable clean-install procedure was:

1. Remove the Attansic component from the Network control panel and decline
   the immediate reboot.
2. Preserve but rename the stale matching `OEM*.INF` and paired `OEM*.PNF` so
   SetupX cannot parse them, and move or rename every obsolete source INF.
3. Preserve and rename `C:\WINDOWS\INF\DRVDATA.BIN` and `DRVIDX.BIN`; Windows
   recreates both driver indexes on the next boot.
4. Reboot, point the hardware wizard explicitly at the release directory, and
   verify the newly created network-class instance before its requested final
   reboot.

The fresh instance contained `UpperRange=ndis3`, `DefUpper=ndis3`, and
`Ndi\Install\ndis3=L2.install`; the new `OEM3.INF` contained the same values.
After the final reboot, the free driver exposed MAC `00-1F-C6-1F-F2-AB`, DHCP
immediately supplied `192.168.0.121/24` with gateway `192.168.0.1`, and both
gateway and Internet echo tests completed without loss.  A gateway TTL of 64
confirmed that this validation traffic used the physical Attansic interface
rather than the independent Raspberry Pi USB management path.


## Free release-candidate runtime soak

The clean-installed free RC ran a continuous Winamp Internet-radio stream for
64 minutes.  During the run the Ethernet cable was removed long enough for the
player's receive buffer to drain.  Playback stopped when the buffer emptied,
then resumed automatically after the cable was reinserted; Winamp, the driver,
and the DHCP client required no manual restart or renewal.

Final Windows interface statistics reported 159,100,491 bytes and 113,845
unicast packets received, zero discarded packets, and zero receive errors.
The aggregate transmit-error count remained at its pre-test value of three.
TCP retransmissions also remained at the pre-test value of three, including
across the deliberate link interruption.  DHCP automatically renewed the same
`192.168.0.121/24` lease during the run.

After the stream test, ten consecutive gateway echo requests and ten Internet
echo requests completed without loss.  Gateway replies had TTL 64 and
0--1 ms latency, confirming direct use of the physical Attansic path.  This
test validates the stripped 14,573-byte free binary in the published RC2
archive; unlike the checked build, it contains no driver debug output.
