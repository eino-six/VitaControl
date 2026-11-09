# 8BitDo Ultimate on PS Vita - install and pairing guide

This package contains two VitaControl builds:

| File | What it supports |
| --- | --- |
| `vitacontrol.skprx` | **Recommended.** Switch Pro mode (`057E:2009`) **and** 8BitDo DirectInput mode, including the Ultimate 2.4G over Bluetooth. |
| `vitacontrol-switch-only.skprx` | The `hamadrehman/VitaControl` release. Switch Pro emulation only. |

In the source tree these are in `dist/`; if you downloaded the zip, they are at
the top level of the extracted folder.

Primary build SHA-256:

```
388ED6300B5598B2DFC502837885E7E265019078BE02F88C7AAD1B7E979888A1
```

---

## What the DirectInput build adds

The stock VitaControl only creates a driver for a fixed list of VID/PID pairs.
If a controller pairs but does not match that list, it never produces input.
The 8BitDo Ultimate 2.4G in Bluetooth mode is one of those devices.

This build adds:

- The fork's `0x3F` simple report and `0x21` subcommand reply handling for
  8BitDo pads that emulate a Switch Pro Controller.
- An 8BitDo DirectInput parser for the two report layouts used over Bluetooth:
  - **Enhanced report:** `[0x01, hat, LX, LY, RX, RY, RT, LT, buttons0, buttons1, ...]`
  - **Legacy report:** `[buttons0, buttons1, hat, LX, LY, RX, RY, ...]`
  - Enhanced reports are also accepted if the host strips the report ID.
- Matching by 8BitDo's VID (`2DC8`) **and** by device name (`8BitDo` or
  `Ultimate`). The Bluetooth PID can differ from the USB PID, so the name is a
  fallback when the controller spoofs another vendor's VID/PID.
- A diagnostic log at `ux0:data/vitacontrol-8bitdo.log`.

The 2.4G's Bluetooth identity can vary by firmware: some units identify as a
Switch Pro Controller (`057E:2009`), while others identify as an 8BitDo
DirectInput device (`2DC8:*`). The primary build handles both paths. The
DirectInput layout is the same format used by SDL's 8BitDo driver. If your unit
uses a slightly different layout, the log records the raw reports so the
mapping can be corrected.

---

## 1. Choose the controller mode

### A. 8BitDo Ultimate Bluetooth (the model with an S / D / X switch)

1. Move the mode switch to **S**.
2. Turn the controller on.
3. Hold the small **pairing button** for about 3 seconds, until the LED blinks
   rapidly.
4. Pair it on the Vita (section 3 below).

This presents itself as a **Switch Pro Controller** (`057E:2009`) and uses the
fork's Switch driver. This is the known-good path for this model.

### B. 8BitDo Ultimate 2.4G (the model with only D / X on the switch)

The 2.4G has two different "Switch" paths, and they are easy to confuse:

- Its **documented Switch mode** (`Minus + LB` for 5 seconds, with the 2.4G
  receiver in the Switch dock) is for the **2.4G receiver only**. The official
  Ultimate Software does not expose Switch mode for this model.
- Its **Bluetooth mode** (profile button for 5 seconds, one vibration) may
  identify as a Switch Pro Controller on some firmware, or as an 8BitDo
  DirectInput device on others. The primary build supports both.

To pair over Bluetooth:

1. Move the mode switch to **D**.
2. Turn the controller on in **Bluetooth** mode: hold the **profile button**
   for about 5 seconds until it vibrates **once**. (Two vibrations means 2.4G
   dongle mode; repeat the step if that happens.)
3. Hold the **pairing button** for about 3 seconds until the LED blinks rapidly.
4. Pair it on the Vita (section 3 below).
5. Check what the Vita calls it:
   - `Pro Controller` means it is using the Switch path (`057E:2009`).
   - `8BitDo Ultimate ...` means it is using the DirectInput path (`2DC8:*`).

Do **not** try to force Gamepad Mode = Switch Pro in the official Ultimate
Software for this model. It is not offered, and the documented Switch mode is
for the 2.4G receiver. Some third-party config tools can write that setting,
but it is not needed for the Bluetooth path handled here.

> If your 2.4G unit is already paired to the Vita in another mode, remove the
> old pairing first. The Bluetooth identity can change with the mode switch or
> a firmware update.

---

## 2. Install the plugin on the Vita

1. Back up the existing `ur0:tai/config.txt`.
2. Copy `vitacontrol.skprx` (from `dist/` or the zip root) to
   `ur0:tai/vitacontrol.skprx`, replacing the old VitaControl file if present.
3. Open `ur0:tai/config.txt` and make sure this line is under `*KERNEL`:

   ```
   ur0:tai/vitacontrol.skprx
   ```

   Example:

   ```
   *KERNEL
   ur0:tai/vitacontrol.skprx
   ```

   If your setup uses `ux0:tai/`, put the file and the line there instead. Do
   not list it under both. Remove any duplicate/old VitaControl entries.
4. Remove or disable any other controller plugin that hooks the same functions
   (`ds34vita`, `ds4vita`, `ds3vita`, etc.) to avoid conflicts.
5. Reboot the Vita. Kernel plugins are only loaded at boot.

### Copying the file

Any of these work:

- **VitaShell USB:** open VitaShell, press `SELECT` to enter USB mode, connect
  the Vita to the PC, and copy the file to `ur0:tai/`.
- **VitaShell FTP:** press `SELECT` to start FTP, connect with FileZilla/WinSCP,
  and copy to `ur0:tai/`.
- **SD card:** if you use `ux0:tai`, copy to the SD card and use VitaShell to
  move it into `ux0:tai/`.

---

## 3. Pair on the Vita

1. On the Vita: **Settings -> Bluetooth -> Manage Bluetooth Devices** (or
   "Add Device").
2. Make sure the controller is in pairing mode (LED blinking rapidly).
3. Select the controller when it appears (`Pro Controller` or
   `8BitDo Ultimate ...`).
4. If the Vita asks for a PIN, try `0000`.
5. After pairing, give it a few seconds. If inputs do not work immediately,
   reboot the Vita once with the controller off, then turn the controller on
   after the Vita has booted.

If you previously paired the controller in a different mode, remove that old
pairing on the Vita and on the controller first. Otherwise the Vita may keep
using the stale pairing.

---

## 4. Test

The controller should now move around the LiveArea. Open a game and verify:

- Left/right sticks and D-pad
- Face buttons
- L1/R1 and L2/R2
- Start/Select
- PS/Home button opens the system menu

Motion, touchpad, and rumble are not implemented for the DirectInput path.
They are not required for normal controls.

---

## 5. Optional: remap buttons

Create `ux0:data/vitacontrol-8bitdo.cfg` to change which Vita button a
controller button reports. Each line moves one button to another. For example,
to make the physical **L1** bumper act as **L2** in Adrenaline:

```
# ux0:data/vitacontrol-8bitdo.cfg
map L1 L2
```

Supported button names are `CROSS`, `CIRCLE`, `SQUARE`, `TRIANGLE`, `L1`,
`R1`, `L2`, `R2`, `L3`, `R3`, `SELECT`, `START`, `PS`, `UP`, `DOWN`, `LEFT`,
`RIGHT`, and `EXT1`. Names are case-insensitive, `#` starts a comment, and up
to 16 mappings are read.

A mapping **moves** the button: with `map L1 L2`, physical L1 reports L2 and no
longer reports L1. To swap two buttons, add both directions:

```
map L1 L2
map L2 L1
```

The file is loaded when the controller connects, so reconnect the controller
after editing it. It works for both the Switch Pro and DirectInput paths used
by the 8BitDo driver. The mapping applies in every app while the controller is
connected; delete or rename the file to restore the default buttons.

---

## 6. If it still does not work

### Read the diagnostic log

Use VitaShell to open:

```
ux0:data/vitacontrol-8bitdo.log
```

The log contains two kinds of lines:

```
DEVICE VID=2DC8 PID=3013 NAME=8BitDo Ultimate wireless
R: 01 08 7F 7F 7F 7F 00 00 00 00 00 00 00 00 00 00
```

- **No `DEVICE` line:** the plugin is not loaded, the controller did not
  connect, or the Bluetooth callback did not run.
- **A `DEVICE` line with the expected controller:** the plugin sees it. The
  VID/PID/name also tells us which driver path was chosen.
- **No `R:` lines after the device connects:** the controller is not sending
  input reports, or the read chain did not start.
- **`R:` lines but some buttons/sticks are wrong:** the report layout is
  slightly different. Send the first 20 `R:` lines, and the mapping can be
  corrected.

Delete the log before a retest if you want a clean record. Reconnecting the
controller starts a new log session.

### Check these in order

1. **Is the plugin actually loaded?**
   - Re-read `ur0:tai/config.txt`; the line must be under `*KERNEL`.
   - Reboot the Vita after copying.
   - Make sure no old `vitacontrol.skprx` entry remains earlier in the config.
2. **Is the controller in the right mode?**
   - Ultimate Bluetooth: switch on **S**, Vita shows `Pro Controller`.
   - Ultimate 2.4G: switch on **D**, Bluetooth mode (one vibration). The Vita
     may show either `Pro Controller` or `8BitDo Ultimate ...`; both paths are
     supported by the primary build.
   - Do not use **X** for the Vita; that is the XInput/Windows mode.
3. **Did you remove the old pairing?**
   - Remove the controller from the Vita's Bluetooth list, turn the controller
     off, then pair again.
4. **Is another plugin stealing the input?**
   - Temporarily disable other controller plugins and reboot.
5. **Try the switch-only fallback.**
   - If your unit really does present as `057E:2009`, install
     `vitacontrol-switch-only.skprx` instead and test.

---

## Rollback

- Remove the `ur0:tai/vitacontrol.skprx` line from `ur0:tai/config.txt`, or
  restore your backup of `config.txt`.
- Delete or replace `ur0:tai/vitacontrol.skprx`.
- Reboot.

The original upstream VitaControl release is available at:

```
https://github.com/Hydr8gon/VitaControl/releases
```

---

## What was changed

Compared with upstream VitaControl, the primary build adds:

- `0x3F` simple report parsing for 8BitDo and other Switch Pro clones.
- `0x21` subcommand reply handling (some 8BitDo pads send this instead of
  switching to `0x30` reports).
- 8BitDo DirectInput report parsing for the Ultimate 2.4G over Bluetooth.
- Corrected X/Y face-button order on the DirectInput path (physical X reports
  Triangle, physical Y reports Square).
- Optional button remapping through `ux0:data/vitacontrol-8bitdo.cfg`.
- A `start()` hook so a DirectInput controller begins its read chain without a
  Switch-mode init write.
- 8BitDo VID/name matching so the Bluetooth PID does not have to be known.
- A diagnostic log with the device identity and raw input reports.
- A file-I/O stub and CMake 3.5 compatibility fix for the portable VitaSDK.

The build has been compiled with the VitaSDK and tested on a physical 8BitDo
Ultimate 2.4G over Bluetooth. The diagnostic log remains available so a
different firmware or report layout can be identified and corrected.
