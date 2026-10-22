VitaControl for 8BitDo Ultimate controllers (PS Vita)
=====================================================

PRIMARY BUILD
-------------
File: vitacontrol.skprx
Size: 13328 bytes
SHA-256: 3D4350760712DEDC13148F0122157B9950777E3DE5E792E7B19FA625DB33C9DA

Tested on a physical 8BitDo Ultimate 2.4G over Bluetooth. This build
supports BOTH:

  1. Switch Pro mode (VID 057E, PID 2009), including the fork's 0x3F and
     0x21 report handling.
  2. 8BitDo DirectInput mode (VID 2DC8, or a device name containing
     "8BitDo"/"Ultimate"), including the Ultimate 2.4G over Bluetooth.

It also writes a diagnostic log:

  ux0:data/vitacontrol-8bitdo.log

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
