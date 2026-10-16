# Slisk — Xbox 360 Wireless Controllers on PS4 🎮

**Free and open-source GoldHEN PS4 ELF payload · [Latest release: v1.1.6](https://github.com/flodmarkss/Slisk/releases/tag/v1.1.6) · GNU GPLv3**

Slisk lets you use **up to three wireless Xbox 360 controllers** on a jailbroken PlayStation 4 with a Microsoft Xbox 360 wireless USB receiver (VID:PID `045e:0719`). No PC intermediary or separately installed Control4Free application is needed.

## Download

**[Download Slisk v1.1.6 from GitHub Releases](https://github.com/flodmarkss/Slisk/releases/tag/v1.1.6)**

- [PS4 ELF — slisk-x360-directusb.elf](https://github.com/flodmarkss/Slisk/releases/download/v1.1.6/slisk-x360-directusb.elf)
- [Complete corresponding source — slisk-complete-source.tar.gz](https://github.com/flodmarkss/Slisk/releases/download/v1.1.6/slisk-complete-source.tar.gz)
- [SHA256 checksums](https://github.com/flodmarkss/Slisk/releases/download/v1.1.6/SHA256SUMS.txt)

## Features

- Three Xbox 360 wireless controllers in addition to the DualShock 4 (four physical controllers in total)
- Xbox buttons, sticks, separate rumble feedback and Xbox player LEDs for players 1–4 as applicable
- Reconnect Xbox controllers after powering them off and back on
- No automatic 20-minute session limit or 120-second inactivity shutdown
- **Exclusive player 1 handoff:** Xbox can take over the account previously used by DualShock 4. If DualShock 4 reclaims that account, Slisk releases the Xbox virtual pad; release and press Xbox A to choose a new account
- Other Xbox controllers may use their own distinct PS4 users / Guest sessions
- Log file: `/data/slisk_x360/slisk.log`

**Hardware-tested:** The maintainer verified the three-controller setup, rumble, reconnection, correct player LED, and exclusive Xbox/DS4 account handoff on a real PS4. This is community software, not a guarantee of compatibility with all firmware, receivers or games.

## Requirements

- Jailbroken PlayStation 4 with compatible GoldHEN and PayLoader
- Microsoft Xbox 360 wireless USB receiver compatible with Slisk (`045e:0719`)
- One to three paired wireless Xbox 360 controllers
- An active DualShock 4 account/session to start the payload

## Quick start

1. Download `slisk-x360-directusb.elf` from the [release page](https://github.com/flodmarkss/Slisk/releases/tag/v1.1.6).
2. Connect the Xbox 360 USB receiver and pair the Xbox controllers with it.
3. Sign in with DualShock 4 and start the ELF using GoldHEN PayLoader.
4. Press **A** on the Xbox controller when Slisk prompts you, then select a PS4 user or Guest.
5. To use Xbox as player 1, choose the account currently associated with DualShock 4. PS4 may disconnect the DS4 from this account. If you subsequently sign the DS4 back into the same account, Slisk should release the Xbox virtual controller so it can select a user again.

For other players, choose distinct accounts or Guest sessions. You may use GoldHEN's **PayLoader AutoRun Queue** after verifying manual startup. **Do not install the ELF in `plugins.ini`**; that is for PRX plugins, not standalone ELF payloads.

If a controller becomes unresponsive or the user session behaves unexpectedly, exit the payload or restart the console. Avoid experimenting with unsaved game progress.

## Build and source code

Slisk's [GitHub Actions build workflow](.github/workflows/slisk-build.yml) runs offline tests, compiles the PS4 ELF and packages the complete corresponding source code. The source archive includes Slisk, its build instructions and scripts, the pinned Control4Free source, and applicable notices.

See [Build and source distribution](docs/BUILD-AND-SOURCE.md) and [Modification history](docs/MODIFICATIONS.md).

## Credits and license

Slisk is developed collaboratively by **flodmarkss and OpenAI ChatGPT** (AI-assisted implementation, debugging and documentation), with real-hardware testing and project stewardship by flodmarkss.

The project incorporates GPLv3-licensed source from [Control4Free](https://github.com/MoHadiShibli/Control4Free), by **MoHadiShibli and contributors**, and upstream components credited in [Third-party notices](systempayload/THIRD_PARTY.md). All third-party authors retain their own rights and notices.

Distributed under **GNU GPL version 3**; see [LICENSE](LICENSE). Source and corresponding build information are provided with the release.

**Unofficial community project.** Not affiliated with or endorsed by Sony, Microsoft or Control4Free maintainers. Provided without warranty.
