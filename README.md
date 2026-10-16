# Slisk — Xbox 360 Wireless Controllers on PS4

**Free and open-source PS4 GoldHEN ELF payload (GNU GPL v3).**

Slisk lets you use **up to three wireless Xbox 360 controllers** through a Microsoft Xbox 360 USB wireless receiver (USB VID:PID `045e:0719`) alongside a DualShock 4 on a jailbroken PlayStation 4. No intermediary PC or installed Control4Free application is required.

## Features

- DualShock 4 as player 1; three Xbox 360 wireless controllers as players 2, 3 and 4
- Independent buttons, analog sticks, LEDs for players 2–4, and rumble feedback
- Wireless controller reconnection after power-off (available in the experimental branches, not yet promoted to stable `main`)
- GoldHEN PayLoader-compatible standalone `.elf`
- Xbox connection diagnostic log at `/data/slisk_x360/slisk.log`
- No 20-minute or 120-second inactivity cutoff in experimental builds (stable `main` retains the earlier safety timeouts)

## Current status — October 2026

Three Xbox 360 controllers, correct LEDs, rumble, and reconnection were confirmed by a tester on a real PS4. The current stable `main` branch preserves the earlier hardware-tested multi-controller version. Reconnect, unlimited-session and duplicate-account retry changes exist on separate development branches; **the account-collision fix still requires hardware verification**. Do not treat development artifacts as a formally released or universally tested product.

**Important:** Choose distinct PS4 accounts or Guest sessions for each Xbox controller, not the account already using the DualShock 4. If a virtual controller becomes stuck, restart the PS4 before retrying.

## Requirements and basic use

- PS4 running compatible GoldHEN with PayLoader support and an active DualShock 4 user session
- Microsoft Xbox 360 wireless USB receiver, compatible wireless Xbox 360 controllers
- Load the GitHub Actions-built `slisk-x360-directusb.elf` with GoldHEN's PayLoader; press A on each Xbox controller when prompted and select a **different** PS4 user/Guest for each

For unattended launch, GoldHEN's PayLoader AutoRun Queue may be used when available in the installed GoldHEN version; verify normal operation manually first. Do **not** put this ELF in `plugins.ini` (PRX plugin mechanism).

## Source and reproducible builds

See [Build and source distribution](docs/BUILD-AND-SOURCE.md). The build uses the exact Control4Free revision `5a8f9e3d0e75378644e7f7b2df6176c386a24da9` from https://github.com/MoHadiShibli/Control4Free, linked into Slisk. The GitHub Actions workflow pins the revision, applies explicit Slisk logging modifications, builds a PS4 SDK container, compiles the ELF, and publishes an artifact.

## License and third-party attribution

Slisk and the combined ELF are distributed under **GNU General Public License version 3 only**; read [LICENSE](LICENSE). The virtual PS4 pad, logging, kernel log parsing and feedback components are derived from **Control4Free by MoHadiShibli and contributors**, under GPLv3. Preserve original copyright notices. See [Third-party notices](systempayload/THIRD_PARTY.md) and [Modification history](docs/MODIFICATIONS.md).

If redistributing an ELF, also provide the complete corresponding source code, build scripts, third-party sources and applicable notices in a GPLv3-compliant way. This repository's current development binaries are not yet packaged as a public release.

**Unofficial community project.** Not affiliated with, sponsored by or endorsed by Sony, Microsoft, or the Control4Free maintainers. Software is provided without warranty; PS4 system modifications carry risk.
