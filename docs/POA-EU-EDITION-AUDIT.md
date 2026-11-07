# PoA EU edition audit

The active baseline is the supplied EU game payload bundled in 0.4.9, verified against
APK SHA-256 `b2767fed01aedcd1c6ce3064d52cfdf910dfaebc81dabd984dd05f707a257c8d`.

## Startup failure and repair

- The EU `Engine.u.ReachSpec` class has `ClassConfigName=UnrealEd`.
- The supplied installation contains `System/DefUnrealEd.ini`, and no generated `System/UnrealEd.ini`.
- Class defaults request the generated configuration while the engine loads classes.
- The launcher now copies the selected installation's template into the missing configuration before native startup. Existing files are preserved, case differences are respected, and the copy uses a temporary file.
- Thirteen Java checks cover template copying, customized configuration preservation, concurrent calls, missing templates, invalid destinations and temporary-file cleanup.

## Edition checks

- Reviewed the current launcher, installation, ZIP import, package checks, release inventories, executable detection, production engine and compatibility sources for regional paths and executable/package assumptions.
- Current game detection uses `System/hppoa.exe`. Required EU scripts exclude `KWGame.u`.
- The EU data has 11 script packages. Audited 1,388 classes, 6,121 functions, 406 states and 102 structs, including 6,552 script bodies.
- Audited 420 array length expressions, nine insertions, 27 removals, 41 delegate declarations, 61 delegate calls and 176 assignments in the EU scripts.
- The EU startup configuration selects `HP_preamble.unr` and `HP3_Adv1Express.unr`.
- Added `hp3-pc-eu.json` with the actual executable, script and startup configuration hashes. The profile generator uses this EU reference by default.
- Retained the NA reference and older inspection reports as historical evidence. Their counts and hashes are separate from the EU baseline.
- Updated the launcher fixture to exercise a PoA folder without `KWGame.u`.

## Open compatibility work

- The EU configuration requests `KWGame.KWConsole` from `KWGame.dll`. The offline audit records this as an external native dependency. Its Android implementation remains pending.
- A successful metadata audit does not verify native calls, rendering, animation, physics, save/load or gameplay.
- The next phone test must confirm startup passes `UnrealEd.ini` and capture the next engine result.

## Validation

- The complete compatibility suite passed 103 tests.
- The Android release checks passed 28 tests.
- The Java configuration repair passed 13 checks and the existing three-game launch fixture passed.
