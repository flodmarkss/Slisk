# Slisk systempayload

This directory contains the standalone GoldHEN ELF that exposes up to three Xbox 360 wireless controllers as PS4 virtual controllers alongside a DualShock 4.

## Current development behavior

- Xbox 360 receiver via USB VID:PID `045e:0719`
- Three wireless Xbox 360 channels: PS4 players 2, 3, 4, separate user/Guest selection
- Face buttons, sticks, D-pad, shoulder buttons, LEDs and rumble
- Disconnect/reconnect while keeping player ownership (development branches)
- Development branches remove the 20-minute and 120-second test limits; stable `main` retains the earlier limits
- Logs: `/data/slisk_x360/slisk.log`
- Manual cancellation and virtual controller cleanup at normal exit

Three-controller support and reconnect were confirmed by a hardware tester on different builds. Stable `main` has not yet incorporated reconnection. The newest duplicate-PS4-account retry fix still needs console verification. Avoid selecting the DualShock 4 user for a virtual Xbox controller.

## Build

See [the full build and GPLv3 corresponding source guide](../docs/BUILD-AND-SOURCE.md). GitHub Actions fetches a pinned revision of GPLv3 **Control4Free** by **MoHadiShibli and contributors** and applies explicitly documented logging modifications. Its PS4 virtual-device code is compiled directly into the Slisk ELF. The console does not need the Control4Free app installed.

The build workflow creates `build/slisk-x360-directusb.elf` and a complete corresponding source archive. The latter should accompany any public executable distribution.

## License and notices

Combined ELF: **GNU GPL version 3 only**. See [LICENSE](../LICENSE), [third-party attribution](THIRD_PARTY.md) and [change history](../docs/MODIFICATIONS.md). Retain original third-party copyright notices. Do not redistribute ELF without GPLv3-compliant provision of its corresponding source.

## Installation

The ELF is launched by GoldHEN PayLoader, not by `plugins.ini`. Where supported, configure GoldHEN PayLoader AutoRun Queue after manually verifying a build. Test on your own responsibility: earlier development versions could hang or crash the console. This is unofficial community software with no warranty.
