# Build Slisk and distribute the complete corresponding source

Slisk is a GNU GPLv3-only combined work. This document describes how to reconstruct the exact third-party input and build from the versioned repository. Keep an immutable Git tag corresponding to every publicly distributed executable.

## Inputs

- Slisk repository source: https://github.com/flodmarkss/Slisk (checkout the exact release tag / Git commit)
- Control4Free source: https://github.com/MoHadiShibli/Control4Free
- **Exact Control4Free commit:** `5a8f9e3d0e75378644e7f7b2df6176c386a24da9`
- The repository's `.github/workflows/slisk-build.yml` and `systempayload/Makefile` are part of the corresponding build scripts
- Linux, Docker, Git and a GitHub Actions-like execution environment

## Obtain complete third-party source

Run in the checked-out Slisk repository:

```sh
cd systempayload
git clone https://github.com/MoHadiShibli/Control4Free.git upstream
git -C upstream checkout --detach 5a8f9e3d0e75378644e7f7b2df6176c386a24da9
git -C upstream rev-parse HEAD
```

The printed SHA must match exactly. Do not replace it with whatever is current on Control4Free's default branch.

## Build and patch

The canonical and tested build recipe is `.github/workflows/slisk-build.yml`: it patches the pinned Control4Free logging paths/header/prefix and Docker image URL, builds the SDK container from `systempayload/upstream/docker/Dockerfile`, and invokes `make -j2` inside it with `systempayload/Makefile`. The workflow creates `systempayload/build/slisk-x360-directusb.elf` and publishes a build artifact.

These modifications are documented in [THIRD_PARTY.md](../systempayload/THIRD_PARTY.md). The original upstream GPLv3 notices must accompany source redistribution.

To reproduce locally, follow the workflow's exact steps instead of running `make` before applying those patches. The Docker image source mirror may evolve; record it for a release if reproducibility matters.

## GPLv3 release source package checklist

When distributing an ELF to the public, offer **the complete corresponding source** with the binary. A practical package or source download should include:

1. The exact Slisk source commit/tag, all `systempayload/src`, tests, headers, build `Makefile`, and `.github/workflows/` scripts.
2. The pinned Control4Free source tree **including copyright headers and upstream LICENSE**, plus the build-time modification instructions or the patched files.
3. Root `LICENSE` (GPLv3), `systempayload/THIRD_PARTY.md`, this document and `docs/MODIFICATIONS.md`.
4. The installation/build information necessary to regenerate that exact ELF, not a moving `main` branch or an unrelated SDK version.

A link to the Slisk repository **alone is not sufficient** if it omits the Control4Free code corresponding to the distributed executable. For a public GitHub Release, publish a complete matching source archive next to the ELF, or an equivalent GPLv3-compliant source provision. Do not claim a full source bundle exists until it has actually been packaged and inspected.
