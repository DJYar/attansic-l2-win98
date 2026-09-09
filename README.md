<h1>
  <img src="https://raw.githubusercontent.com/alexh/vintage-icons/main/static/icons/network_cool_two_pcs_0.png" width="48" alt="Network computers" align="absmiddle">
  Attansic L2 driver for Windows 98 SE / ME
</h1>

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

## <img src="https://raw.githubusercontent.com/alexh/vintage-icons/main/static/icons/installer_generic_old_0.png" width="24" alt="Installer" align="absmiddle"> Install

Download the release archive and, when Windows asks for a driver, point the
hardware wizard at the extracted directory.  The package contains:

- `attl2mp.inf`
- `l2ndis.sys`
- `README.txt`

Windows 98 can aggressively reuse an older `OEM*.INF` and its compiled PNF
cache.  If an earlier development build was installed, follow the clean-install
notes in [docs/porting_notes.md](docs/porting_notes.md#clean-free-rc-installation-on-windows-98-se).

## <img src="https://raw.githubusercontent.com/alexh/vintage-icons/main/static/icons/directory_control_panel_2.png" width="24" alt="Control Panel" align="absmiddle"> Build

The source uses the Windows 2000 DDK-style `build` environment and Visual C++
6.  Microsoft DDK and compiler files are not included in this repository.
See [docs/build.md](docs/build.md) for the expected layout and commands.

## <img src="https://raw.githubusercontent.com/alexh/vintage-icons/main/static/icons/msg_information_0.png" width="24" alt="Information" align="absmiddle"> Status

This is hardware-specific, independently developed legacy-system software.
Keep a recoverable Windows installation while testing it on additional L2
devices or Windows ME configurations.

---

<p align="center">
  <img src="https://raw.githubusercontent.com/alexh/vintage-icons/main/static/icons/windows_0.png" width="32" alt="Windows logo" align="absmiddle">
  <strong>Designed for Windows&trade; 98</strong>
  <br>
  <sub>Icons from <a href="https://github.com/alexh/vintage-icons">alexh/vintage-icons</a>.</sub>
</p>
