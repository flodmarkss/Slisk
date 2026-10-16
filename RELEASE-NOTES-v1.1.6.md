# Slisk v1.1.6 — first public release 🎮

Slisk is an **unofficial free and open-source GoldHEN PS4 ELF** supporting Xbox 360 wireless controllers through a Microsoft USB wireless receiver (VID:PID `045e:0719`), without a PC.

## Features
- Up to **three Xbox 360 wireless controllers** alongside a DualShock 4
- Buttons, analog sticks, independent rumble, and correct Xbox player LEDs
- Wireless reconnection after turning controllers off
- Xbox controller can take over DualShock 4's account as player 1; if DualShock 4 reclaims that account, the Xbox virtual pad is released and can choose an account again
- No automatic 20-minute or idle-session shutdown
- Logs at `/data/slisk_x360/slisk.log`

## Downloads
- **`slisk-x360-directusb.elf`** — PS4 PayLoader payload
- **`slisk-complete-source.tar.gz`** — complete corresponding source for this exact build, including the pinned Control4Free implementation, build scripts and GPL notices
- **`SHA256SUMS.txt`** — file checksums

## Installation
1. Use compatible GoldHEN with PayLoader and an active PS4 user account.
2. Connect an Xbox 360 wireless USB receiver and pair controllers.
3. Load the ELF with GoldHEN PayLoader. Optional: add the ELF to PayLoader AutoRun Queue after verifying manual operation.
4. Press A on each Xbox controller to register a virtual controller and choose a PS4 account.
5. To reclaim player 1, sign the DualShock 4 back into that account; Slisk should allow Xbox to select an account again.

**Do not install the ELF as a PRX in `plugins.ini`.** The payload runs in a privileged console context. Earlier experimental builds caused console freezes; proceed cautiously. No warranty.

## Attribution and licensing
Slisk development was conducted collaboratively through **flodmarkss** and **OpenAI ChatGPT** (AI-assisted implementation, debugging and documentation). The Xbox/PS4 integration is built on GPLv3 **Control4Free**, maintained by **MoHadiShibli and contributors**, plus other upstream work noted in the corresponding source. Copyright in third-party code remains with the respective rights holders. The combined source and ELF are distributed under **GNU GPL version 3**; see [LICENSE](LICENSE), [third-party notices](systempayload/THIRD_PARTY.md) and [building/source instructions](docs/BUILD-AND-SOURCE.md).

This project is not affiliated with Sony, Microsoft or the upstream project's maintainers.
