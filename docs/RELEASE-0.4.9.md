# HP:Trilogy Mobile 0.4.9

## PoA startup

- Fix the 0.4.8 phone failure: directory iteration of missing `TestMaps` aborted startup after package discovery.
- The real EU Default.ini includes developer-only TestMaps and optional Saves entries; neither is required to be present to scan retail game data.
- Only register map folders that exist as directories. Use error-code directory iteration when scanning maps, so an unavailable or removed directory is reported and skipped.
- Keep actual file-processing errors visible rather than silently dropping invalid packages.
- The report establishes progress beyond the 0.4.7 Core-path failure: 9 paths and 109 packages were discovered. This does not establish successful Core deserialization or gameplay.

## Android settings correction

- Remove the standalone launcher settings button and its separate GameProfileUI dialogs.
- Retain the existing custom Android Display / Controls / Game panel for HP3. No replacement launcher settings design is introduced.
- Keep supported FPS/FOV/render and touch settings and independent game profiles. UE1-only first-person, auto-jump and HUD-scale modifications remain unavailable for PoA.

Version 0.4.9, Android code 50, package com.tranquil.hp1native. All three game payloads and permanent signing certificate retained. Phone acceptance remains open.

## Verification

- 101 compatibility regressions and 28 release checks passed.
- Seven production directory-visitor cases passed: missing folder, normal folder, non-directory, empty folder, broken symlink, removed folder and preserved file-processing exceptions.
- The production directory visitor also scanned the actual EU data: Maps contains 34 files; absent TestMaps and Saves folders were skipped without throwing. This is not native game execution.
- Android API 36 Java compilation, ARM64 native build, 16 KiB alignment, permanent certificate and all three payload hashes passed.
- The signed APK contains the new directory scanner and excludes the retired GameProfileUI class.

## Verified build evidence

- ARM64 native build and Android API 36 Java compilation passed.
- 101 PoA regressions, 28 release checks, 13 installer checks, 12 ZIP importer checks and 22 launcher/path/timing checks passed.
- All three payloads checked by size and SHA-256 after signing.
- Four Android native libraries are AArch64 with 16 KiB LOAD alignment; packaged library alignment checked.
- APK certificate matches the permanent 0.4.2 pin.
- Original PC game libraries under assets are data, not Android native libraries.
- No Android device execution was performed.

Published 5 November 2026 as [Latest v0.4.9](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.9). GitHub byte counts and SHA-256 digests match all four local release artifacts. Phone gameplay acceptance remains open.
