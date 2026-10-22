# VitaControl
Use bluetooth controllers on your Vita!

> **8BitDo build:** This fork adds DirectInput support for the 8BitDo Ultimate
> 2.4G over Bluetooth, plus the Switch-clone `0x3F`/`0x21` handling from the
> `hamadrehman/VitaControl` fork. See [INSTALL.md](INSTALL.md) and the prebuilt
> files in `dist/`.

### Overview
VitaControl is a plugin that aims to support a wide variety of bluetooth controllers on the PlayStation Vita. It uses an
object-oriented design, which makes it easy to add support for new controllers. VitaControl is based on
[ds34vita](https://github.com/MERLev/ds34vita) by MERLev, which is based on [ds3vita](https://github.com/xerpi/ds3vita)
and [ds4vita](https://github.com/xerpi/ds4vita) by xerpi.

### Downloads
The latest build of VitaControl is automatically provided via GitHub Actions, and can be downloaded from the
[releases page](https://github.com/eino-six/VitaControl/releases). The original upstream project is
[Hydr8gon/VitaControl](https://github.com/Hydr8gon/VitaControl).

### Usage
Place `vitacontrol.skprx` in the `ur0:tai/` directory on your Vita. Open `config.txt` in the same folder and add
`ur0:tai/vitacontrol.skprx` under the `*KERNEL` header. Reboot the Vita and pair your controllers through the Settings
app! If you have issues with a controller when it's first paired, it might work after another reboot.

### Supported Controllers
* Sony DualShock 3 Controller
* Sony DualShock 4 Controller
* Sony DualSense Controller
* Sony DualSense Edge Controller
* Microsoft Xbox One Controller
* Nintendo Switch Pro Controller
* 8BitDo Ultimate 2.4G Controller (Bluetooth DirectInput mode)

### 8BitDo Ultimate 2.4G Support

This fork keeps the upstream controller drivers and the
`hamadrehman/VitaControl` Switch-clone fixes (`0x3F` simple reports and `0x21`
subcommand replies), and adds a DirectInput driver for the 8BitDo Ultimate 2.4G
over Bluetooth.

The 2.4G can identify itself in more than one way depending on firmware. The
build handles both:

* `057E:2009` Switch Pro identity — uses the Switch driver.
* 8BitDo VID `2DC8` or a device name containing `8BitDo`/`Ultimate` — uses the
  new DirectInput parser.

It supports both common 8BitDo Bluetooth report layouts:

```text
Enhanced: 01 hat LX LY RX RY RT LT buttons0 buttons1 ...
Legacy:   buttons0 buttons1 hat LX LY RX RY ...
```

The parser also accepts the enhanced layout if the host strips the report ID.
It maps the D-pad, both sticks, face buttons, shoulders, Start/Select, L3/R3,
PS/Home, and digital L2/R2 triggers.

The fork also writes a diagnostic log to `ux0:data/vitacontrol-8bitdo.log` with
the device VID/PID/name, the detected report layout, and the first 200 distinct
raw reports. See [INSTALL.md](INSTALL.md) for pairing, installation, rollback,
and log details.

Prebuilt files are in `dist/`.

### Contributing
Pull requests are welcome for this project. I can add support for controllers that I have access to and am interested
in, but anything else is up to the community. If you know your way around code and have a controller that you'd like to
support, it shouldn't be too hard to add. You can check if projects such as
[MissionControl](https://github.com/ndeadly/MissionControl) or [SDL](https://github.com/libsdl-org/SDL) have drivers for
the controller that you can use as a reference.

### Building
To build VitaControl, you need to install [Vita SDK](https://vitasdk.org). With that set up, run
`mkdir -p build && cd build && cmake .. && make -j$(nproc)` in the project root directory to start building.

### Other Links
* [Hydra's Lair](https://hydr8gon.github.io) - Blog where I may or may not write about things
* [Discord Server](https://discord.gg/JbNz7y4) - A place to chat about my projects and stuff
