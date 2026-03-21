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
administered placeholder MAC address (`02-00-00-00-00-01`) to keep intent
explicit until hardware MAC retrieval is implemented.


## Miniport build defines required for `ndis.h`

Build errors around `NDIS_MINIPORT_CHARACTERISTICS`, `NdisMInitializeWrapper`,
`NdisMRegisterMiniport`, and `NdisMSetAttributesEx` were caused by missing
miniport-specific preprocessor defines in the build `sources` file.

Following the `e100bex` pattern, the build now explicitly defines:

- `NDIS_MINIPORT_DRIVER=1`
- `NDIS50_MINIPORT=1`

`BINARY_COMPATIBLE` is still kept as before. These defines ensure `ndis.h`
exposes NDIS 5 miniport declarations rather than a more generic NDIS surface.
