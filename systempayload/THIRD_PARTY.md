# Third-party software and attribution

## Control4Free

- Project: **Control4Free**
- Maintainer / repository: **MoHadiShibli and contributors**
- Original source: https://github.com/MoHadiShibli/Control4Free
- Pinned source revision: `5a8f9e3d0e75378644e7f7b2df6176c386a24da9`
- License: **GNU GPL version 3**. Original license text is provided in the upstream `LICENSE`; the same GPLv3 terms appear in this repository's root [LICENSE](../LICENSE).
- Copyright: retained by the respective original authors and contributors; Slisk does not claim ownership of their work.

Slisk includes and builds the following upstream implementation modules (with relevant headers):

- `upstream/src/log.c`
- `upstream/src/vda.c`
- `upstream/src/klog_line.c`
- `upstream/src/feedback.c`

### Changes to upstream code at build time (2026-10)

The reproducible GitHub Actions workflow `.github/workflows/slisk-build.yml` applies these transformations to the pinned source before compilation:

1. In `upstream/include/c4f_log.h`, change the log directory from `/data/control4free` to `/data/slisk_x360` and the file basename from `control4free.log` to `slisk.log`.
2. In `upstream/src/log.c`, rename the start-of-run log header from `Control4Free` to `Slisk` and the log prefix from `[c4f]` to `[slisk]`.
3. In `upstream/docker/Dockerfile`, replace the Ubuntu registry image URL with a mirror to work around rate limits. This changes the build environment, not console-side functionality.

The original upstream notices remain applicable and must not be removed when redistributing source.

## Slisk-original integration

Slisk integrates a dedicated Xbox 360 USB wireless receiver backend, report decoding and PlayStation input mapping, three independent Xbox radio channels, player LEDs, rumble feedback, virtual-device user assignment, reconnect handling, diagnostics and cleanup. The Slisk-specific sources are in `systempayload/src/`. See [modification history](../docs/MODIFICATIONS.md).

The compiled ELF is a combined work incorporating GPLv3 third-party code. Therefore distribute the combined executable and its complete corresponding source **under GPLv3**, including necessary source and scripts. Control4Free does not need to be separately installed on the PS4.

## Additional components

A PS4 payload SDK and Ubuntu image are used to build the payload; they are not authored by Slisk and may have their own notices/terms. Review the build container and included dependencies before redistribution. Sony and Microsoft trademarks belong to their respective owners.
