# 0.4.2 — HP2 menu clipping correction (phone test candidate)

The 0.4.1 phone report showed only part of the Chamber of Secrets menu: a narrow crop of the title and New Game button, with the remaining screen black. HP2 uses its native desktop front end; TPS uses a different in-game menu path.

The HP2 front end changes Canvas drawing scale to fit Android, but inherited widget clips stayed in the old logical coordinate system. This cropped the artwork and lower controls. Canvas now converts existing clips when scale changes, preserving their physical screen bounds and restoring them when leaving the menu. The existing inverse touch mapping remains paired with the displayed menu.

A C++ regression checks the reported 1536×687 surface at density scales 1, 2, 2.75 and 3, verifies the full menu fits the physical clip, and verifies restoration. It passes. A native Android rebuild and complete signed Both Games APK were produced locally as 0.4.2 / code 43 using the existing certificate.

Phone acceptance remains pending: verify the complete title, New/Load/Options, six slots, Back/OK and touch selection. This fixes the clipping calculation; it does not claim a TPS-style replacement menu or a phone-confirmed result. Both games remain bundled; optional HD packs remain separate. Earlier Lumos, red-wall, mirror/window and original Controls-row limitations remain open.

APK: **718,674,960 bytes**, SHA-256 `f408e04ee8c753d53ec3c8c487429a66094265fe84824da1def675d1e4eac9f5`. Native and Android Java compilation, every bundled game-file hash, APK signature, ZIP integrity and 16 KiB alignment passed. Both games are included: 339 HP1 files and 682 HP2 files. Publication as Latest is explicitly approved by the user; physical-device acceptance remains pending.
