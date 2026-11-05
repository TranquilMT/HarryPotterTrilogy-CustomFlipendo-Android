# HP:Trilogy Mobile 0.4.8 — PoA startup hotfix

- Fix the 0.4.7 phone failure: `Could not find package Core`.
- PoA left the UE1 ancestry field unset. The package scanner incorrectly selected indexed `Paths[0]` configuration, while the actual EU Default.ini uses repeated `Paths=` entries.
- Choose the modern package-path format explicitly for PoA, and load its separate user/default-user files consistently.
- Add selected-game managed folder and key file checks to exported reports; retain existing log exports.
- Keep all three game payloads and the permanent signing certificate. Version code 49.
- This is a targeted startup fix. Rendering and gameplay remain unverified on Android.

## Additional engine work

- Recover the original Default.ini package list if an existing generated PoA configuration has no Paths entries. Preserve nonempty custom path lists.
- Recover missing PoA SavePath and CachePath from the original default configuration; keep existing custom values.
- Report path, package and map-folder discovery counts in the engine log.
- Stop with a specific configuration error if neither active nor default configuration provides any package paths.
- Keep all legacy UE1 configuration choices unchanged, with production-policy coverage for HP1/HP2, old Unreal, Rune and KHG.
- Validate the six EU script package tables: Core (865 exports), Engine (9,697), HGame (11,194), GUI (2,204), GamePlay (1,224), UWindow (3,459); all package version 129. This is a static check, not object execution.

## Settings and performance

- Add a Game settings button before launch for every game: Display, Controls, Diagnostics and performance presets. HP3 settings are accessible even when native startup fails.
- Keep independent settings profiles for all three games. Offer 35%/30 FPS battery, 50%/60 FPS balanced and 75%/60 FPS quality presets. These are user-selectable targets, not measured device benchmarks.
- Connect PoA’s FPS setting to native frame pacing independently of the UE1 mod registry, capped at 60 FPS for initial compatibility testing.
- Connect PoA’s FOV setting to the camera calculation and widescreen correction. Retain the shared runtime settings panel and include PoA aim sensitivity in controller navigation.
- Hide UE1-specific first-person, auto-jump and HUD-scale modifications in PoA until implemented.
- Static EU call audit: 6,121 functions decoded, zero decode errors. Metadata still has 431 unresolved relationships; complete corpus execution was withheld.

## Verified build evidence

- ARM64 native build and Android API 36 Java compilation passed.
- 100 PoA regressions, 28 release checks, 13 installer checks, 12 ZIP importer checks and 22 launcher/path/timing checks passed.
- All three payloads checked by size and SHA-256 after signing.
- Four Android native libraries are AArch64 with 16 KiB LOAD alignment; packaged library alignment checked.
- APK certificate matches the permanent 0.4.2 pin.
- Original PC game libraries under assets are data, not Android native libraries.
- No Android device execution was performed.

Published 5 November 2026. All four uploaded release artifacts match local byte counts and SHA-256 digests. Phone gameplay remains unverified.
