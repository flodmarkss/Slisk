# Slisk modification history and origin

**Date recorded:** 2026-10-16. All changes described below are modifications/integrations for Slisk, not official Control4Free releases.

## Reused upstream code (Control4Free)

The pinned GPLv3 Control4Free revision `5a8f9e3d0e75378644e7f7b2df6176c386a24da9` provides the virtual PS4 device routines, feedback interfaces, kernel-log event parsing and file logging. The build-time modifications to its logging files and Dockerfile are itemized in [THIRD_PARTY.md](../systempayload/THIRD_PARTY.md). Those changes are performed automatically in the workflow, rather than rewriting upstream history.

## Slisk-specific developments (2026)

- Direct Microsoft Xbox 360 wireless USB receiver detection and USB interface/channel handling
- Xbox report parsing, button/analog mapping to PS4 virtual controller reports
- Distinct virtual device identities and user selection for Xbox players 2, 3 and 4 alongside DualShock 4 player 1
- Separate LED quadrant selection and vibration feedback per Xbox controller
- Reconnect after power-off, including neutral input during disconnection and restoration on reconnect
- Cleanup of all three virtual controllers at normal exit
- Removal of the 20-minute test cap and 120-second idle shutdown in current development branch
- Removal of obsolete `slisk-v092` diagnostic status files; new log name `/data/slisk_x360/slisk.log`
- Experimental duplicate-account recovery: prevent assigning player 2 to the DualShock 4 user's account, remove the rejected virtual pad and permit a fresh A-button attempt

The latter duplicate-account recovery is **not yet hardware-verified**. Other features were confirmed through console tests; see README for the boundary of verification.

## How changes are maintained

Slisk code and its offline tests reside in `systempayload/`. Upstream Control4Free source is checked out at a fixed commit during builds. The transformation steps are in `.github/workflows/slisk-build.yml`. Both the original third-party code and Slisk sources must accompany any GPLv3 source distribution. Preserve attribution and dates when modifying and redistributing.
