# Attansic L2 driver for Windows 98 SE / ME

An experimental NDIS miniport driver for the Attansic/Atheros L2 Fast
Ethernet controller in the original ASUS Eee PC 701.

The currently validated device is:

- PCI ID `VEN_1969&DEV_2048&SUBSYS_82331043`
- permanent MAC tested as `00-1F-C6-1F-F2-AB`
- 100 Mbps full-duplex operation on Windows 98 SE

The driver implements PCI/MMIO discovery, PHY negotiation and link changes,
interrupt-driven TX/RX, multicast filtering, watchdog recovery, reset and
shutdown handling.  The release candidate has completed mixed browser and
Internet-radio traffic, repeated cable removal/reconnection, runtime
disable/enable, and multi-hour checked-build soak tests without driver-level
TX/RX errors.

## Install

Download the release archive and, when Windows asks for a driver, point the
hardware wizard at the extracted directory.  The package contains:

- `attl2mp.inf`
- `l2ndis.sys`
- `README.txt`

Windows 98 can aggressively reuse an older `OEM*.INF` and its compiled PNF
cache.  If an earlier development build was installed, follow the clean-install
notes in [docs/porting_notes.md](docs/porting_notes.md#clean-free-rc-installation-on-windows-98-se).

## Build

The source uses the Windows 2000 DDK-style `build` environment and Visual C++
6.  Microsoft DDK and compiler files are not included in this repository.
See [docs/build.md](docs/build.md) for the expected layout and commands.

## Status

This is hardware-specific, independently developed legacy-system software.
Keep a recoverable Windows installation while testing it on additional L2
devices or Windows ME configurations.
