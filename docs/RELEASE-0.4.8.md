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
