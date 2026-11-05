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
