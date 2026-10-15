# Third-party notices

This directory produces a **standalone** PS4 ELF using virtual controller
routines from **MoHadiShibli/Control4Free** (`src/log.c`, `src/vda.c`,
`src/klog_line.c`, `src/feedback.c`, and corresponding headers), pinned at
commit `5a8f9e3d0e75378644e7f7b2df6176c386a24da9`.

Upstream: https://github.com/MoHadiShibli/Control4Free
Copyright belongs to the original contributors.
License: GNU General Public License version 3 (`GPL-3.0`). The built ELF and
this systempayload subproject are distributed under GPL-3.0-only.

At runtime the ELF **does not** require Control4Free to be installed or running.
It uses Sony's on-console virtual controller service. No socket server or
network forwarding is used for Xbox input.

The user's repository root may contain a different historical license; for
this independently built subproject the terms above govern this binary.
