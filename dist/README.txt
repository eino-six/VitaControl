VitaControl for 8BitDo Ultimate controllers (PS Vita)
=====================================================

PRIMARY BUILD
-------------
File: vitacontrol.skprx
Size: 14880 bytes
SHA-256: 388ED6300B5598B2DFC502837885E7E265019078BE02F88C7AAD1B7E979888A1

Tested on a physical 8BitDo Ultimate 2.4G over Bluetooth. This build
supports BOTH:

  1. Switch Pro mode (VID 057E, PID 2009), including the fork's 0x3F and
     0x21 report handling.
  2. 8BitDo DirectInput mode (VID 2DC8, or a device name containing
     "8BitDo"/"Ultimate"), including the Ultimate 2.4G over Bluetooth.

Changes in this build:

  - Fixed the inverted X/Y face buttons on the DirectInput path.
    Physical X now reports Triangle and physical Y reports Square.
  - Added optional button remapping through ux0:data/vitacontrol-8bitdo.cfg.

It also writes a diagnostic log:

  ux0:data/vitacontrol-8bitdo.log

BUTTON REMAPPING
----------------
Create ux0:data/vitacontrol-8bitdo.cfg. Each line moves one button to another.
For example, to make the physical L1 bumper act as Vita L2 in Adrenaline:

  # ux0:data/vitacontrol-8bitdo.cfg
  map L1 L2

Supported names: CROSS, CIRCLE, SQUARE, TRIANGLE, L1, R1, L2, R2, L3, R3,
SELECT, START, PS, UP, DOWN, LEFT, RIGHT, EXT1. Names are case-insensitive,
'#' starts a comment, and up to 16 mappings are read. A mapping moves the
button, so add both directions to swap two buttons. The file is loaded when
the controller connects; reconnect it after editing the file. The mapping
applies in every app while the controller is connected; delete or rename the
file to restore the default buttons.

SWITCH-ONLY FALLBACK
--------------------
File: vitacontrol-switch-only.skprx
Size: 10944 bytes
SHA-256: 56C0CD308E19AFF2421EE8A8903931CBAE000B6B0AD3FC8C074900526F5AFC43

This is the hamadrehman/VitaControl release. It only supports controllers that
present as a Switch Pro Controller. Use it if your Ultimate Bluetooth model is
in S mode and the primary build gives you trouble.

INSTALL
-------
1. Back up ur0:tai/config.txt.
2. Copy vitacontrol.skprx to ur0:tai/vitacontrol.skprx.
3. Add this line under *KERNEL in ur0:tai/config.txt:

   ur0:tai/vitacontrol.skprx

4. Remove/disable other controller plugins (ds34vita, ds4vita, etc.).
5. Reboot the Vita.

CONTROLLER MODE
---------------
8BitDo Ultimate Bluetooth (S/D/X switch):
  - Set the switch to S.
  - Hold the pairing button ~3 seconds until the LED blinks fast.
  - Pair in Vita Settings > Bluetooth.

8BitDo Ultimate 2.4G (D/X switch only):
  - Set the switch to D.
  - Turn it on in Bluetooth mode: hold the profile button ~5 seconds until it
    vibrates once (two vibrations = 2.4G dongle mode).
  - Hold the pairing button ~3 seconds until the LED blinks fast.
  - Pair in Vita Settings > Bluetooth.
  - The Vita may show "Pro Controller" or "8BitDo Ultimate ..."; this build
    supports both identities.

Do NOT try to set Gamepad Mode = Switch Pro in the official software for the
2.4G model. It is not offered; the documented Switch mode is for the 2.4G USB
receiver. Bluetooth identity varies by firmware, and both paths are handled.

Remove the old Bluetooth pairing before re-pairing in a different mode.

IF IT STILL DOES NOT WORK
-------------------------
Open ux0:data/vitacontrol-8bitdo.log in VitaShell. It records the device
VID/PID/name and the first raw input reports. The first 20 "R:" lines are
enough to diagnose a report-layout mismatch.

Full guide: INSTALL.md
