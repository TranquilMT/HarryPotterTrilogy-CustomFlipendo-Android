# 0.4.2 — HP2 menu clipping correction (phone test candidate)

The 0.4.1 phone report showed only part of the Chamber of Secrets menu: a narrow crop of the title and New Game button, with the remaining screen black. HP2 uses its native desktop front end; TPS uses a different in-game menu path.

The HP2 front end changes Canvas drawing scale to fit Android, but inherited widget clips stayed in the old logical coordinate system. This cropped the artwork and lower controls. Canvas now converts existing clips when scale changes, preserving their physical screen bounds and restoring them when leaving the menu. The existing inverse touch mapping remains paired with the displayed menu.

A C++ regression checks the reported 1536×687 surface at density scales 1, 2, 2.75 and 3, verifies the full menu fits the physical clip, and verifies restoration. It passes. A native Android rebuild and complete signed Both Games APK are being produced locally as 0.4.2 / code 43 using the existing certificate.

Phone acceptance remains pending: verify the complete title, New/Load/Options, six slots, Back/OK and touch selection. This fixes the clipping calculation; it does not claim a TPS-style replacement menu or a phone-confirmed result. Both games remain bundled; optional HD packs remain separate. Earlier Lumos, red-wall, mirror/window and original Controls-row limitations remain open.
