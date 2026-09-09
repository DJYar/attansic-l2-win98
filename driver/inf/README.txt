Attansic L2 Fast Ethernet Adapter driver for Windows 98 SE / Windows ME
=======================================================================

Validated hardware
------------------
PCI\VEN_1969&DEV_2048&SUBSYS_82331043 (ASUS Eee PC)

Files
-----
attl2mp.inf  Windows 98/ME installation information
l2ndis.sys   NDIS miniport driver

Installation
------------
In Device Manager, select the Ethernet controller, choose Update Driver, and
point the wizard to the directory containing these files. Reboot when Windows
requests it.

Existing test installation
--------------------------
If this driver is already installed, l2ndis.sys can be replaced in
C:\WINDOWS\SYSTEM while the old driver is not active, followed by a reboot.
Keep a copy of the previous working file before replacement.

Recovery
--------
If Windows cannot start normally after an update, boot to Command Prompt Only
and restore the previous l2ndis.sys to C:\WINDOWS\SYSTEM\l2ndis.sys.

The release package uses the free DDK configuration without diagnostic debug
output. Runtime reset and software PHY link-flap self-test triggers are
disabled.
