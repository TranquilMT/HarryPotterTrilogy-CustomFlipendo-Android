# Harry Potter TQ — Custom Flipendo Android

Native Android work on Philosopher’s Stone and Chamber of Secrets, with separate research into Prisoner of Azkaban.

## Download and installation

[0.4.2 Both Games menu-fix release](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.2) includes **both HP1 and HP2 game payloads** in `Harry-Potter-TQ-0.4.2-Both-Games.apk`: **718,674,960 bytes (718.7 MB)**. Version name **0.4.2**, version code **43**, package `com.tranquil.hp1native`. The earlier 9.9 MB engine-only build is superseded.

Install over the existing app without uninstalling to retain app data. The permanent signing certificate is retained. Updating on a physical device and gameplay acceptance still need testing. The release includes full source and SHA-256 checksums. HD textures are separate optional downloads.

| Game | Current status |
| --- | --- |
| Philosopher’s Stone / Sorcerer’s Stone | Previously reported playable; regression testing of 0.4.1 remains pending. |
| Chamber of Secrets | Story, six save slots, shops, duels, Russian text, subtitles and gamepad support previously reported working. Substantial menu, save and settings changes in 0.4.1 require phone testing. |
| Prisoner of Azkaban | Disabled and not playable. UE2 compatibility research only. |

## 0.4.2 — Chamber of Secrets menu correction

Fix the startup menu cropping reported in 0.4.1: Canvas now preserves physical widget clipping boundaries when the Android menu changes drawing scale. A regression checks the reported 1536×687 surface at four display densities. Local ARM64/Java builds, both game payload hashes, signature and alignment checks pass. Phone testing of the complete menu and touch selection remains pending. All 0.4.1 updates below are retained. See [0.4.2 details](docs/RELEASE-0.4.2.md).

## Philosopher’s Stone — 0.4.1 updates

- Bundled original game payload and configuration files, with file-by-file verification before signing.
- Shared renderer correction groups mirror surfaces only when their oriented reflection planes match.
- Shared HD installer bounds progress callbacks, reports copying, manifest saving and cleanup, and rolls back interrupted installation changes.
- Existing missing/empty configuration repair remains available; existing nonempty configurations and saves are preserved.

Check launch, menus, an existing save, controls and reflections when testing this release. No new complete-playthrough claim is made.

## Chamber of Secrets — 0.4.1 updates

### Menus and controls

- Enable the game’s existing New Game / Load Game / Options front end on Android. Previously, Android startup skipped it.
- Scale the native menu and touch coordinates together for landscape screens. Confirm replacing an occupied game within the same native window.
- Add an in-game menu with Resume, Save Game, Load Game, Game Book, Options, Return to Main Menu and Quit to Launcher. Save and Load use six-slot dialogs with overwrite confirmation.
- Move slot selection away from the launcher and remove the HP2 quick-save toolbar button. Normal Play opens the game menu; internal restarts retain the selected saved slot. Gamepad Y opens the menu, and Android dialogs can receive gamepad navigation.
- Connect front-end Display and Controls buttons to Android settings while retaining the game’s Sound page.
- Restore the game-book menu launch path and prevent a duplicate start menu when loading a selected save.
- Preserve the original `Help` directory during ZIP/ISO imports, including menu artwork and sounds. Readable fallback menu buttons support older imports missing those assets.

### Saves and persistent objects

- Honor the requested load slot, validate slot bounds, remember the active slot across internal restarts, and notify the interface after saving.
- Detect real save files, including case variations; empty files and thumbnails alone no longer count as saved games.
- Preserve the six existing flat save files. Starting a new game clears only its selected slot and that slot’s persistent snapshot.
- Isolate future persistent-world snapshots by save slot. Legacy saves retain a shared-snapshot fallback until saved again; historical world states already overwritten by old versions cannot be reconstructed.
- Restore spawned actors using already loaded classes when imported configuration omits their package. Ambiguous matches are rejected. This addresses a possible cause of missing spawned beans, but travel and save/load behavior still need gameplay verification.

### Sound and rendering

- Route cutscene mute/unmute commands to the actual music handler instead of a duplicate no-op handler.
- Save volume settings consistently to both relevant configuration sections, persist them, and reject malformed/non-finite values while keeping defaults.
- Query the actual fullscreen state instead of returning a hardcoded value.
- Change the wand-effect origin from a bounding-box corner to the far end-cap centre and compensate for rendered wand length. This is a **candidate Lumos alignment fix**, awaiting comparison in gameplay.
- Include the shared mirror-plane grouping correction. The opening Privet Drive mirror/window glitch remains unresolved.

## Shared installer and packaging

HD progress updates are bounded to avoid flooding the UI queue. Progress now covers extraction/verification, copying, manifest writing and cleanup. Interrupted commits restore prior texture files and manifests in host tests. The reported phone ANR has not been reproduced or conclusively verified fixed.

Release packaging now requires **both** game-data inputs. It validates core packages and configuration, writes both installation stamps, and compares every embedded payload file’s SHA-256. Missing games, engine-only packages, duplicate paths, missing required files, HD payloads and signing material fail the release checks. All four native libraries were rebuilt for this release; the corrected bundled APK preserves those binaries.

## Optional HD textures

The [v0.3.3 release](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.3.3) retains `HP1-HD-Refresh-All-Modules-v0.2.1-Compatible.zip`, containing all six HP1 modules (3,505 textures). Install the ZIP directly through **Philosopher’s Stone → HD textures → INSTALL HD PACK ZIPS**; do not extract it. Use the corrected compatible ZIP instead of older HP1 v0.2 packs. Keep the installation screen open.

HD selection and installation remain game-specific. This HP1 download is not an HP2 pack. No HD textures are embedded in the 0.4.1 APK.

## Known issues and remaining testing

- HP2 opening Privet Drive mirror/window flicker and some red translucent walls remain open.
- The original game-book Options → Controls page still has empty rows. The Android Controls page is available, but does not repair those rows.
- Verify Lumos tip alignment, lingering effects, spawned beans across level travel, hub returns and different save slots.
- Verify all six saves, overwrite confirmation, old-save loading, new games, audio persistence, touch/gamepad navigation and background/resume on a phone.
- Full 43-map progression, duels, Quidditch and broader compatibility require further testing.
- Prisoner of Azkaban remains disabled; parser, bytecode and native-function research does not establish playability.

## Source, roadmap and validation

See the [roadmap](HP-TPS-ANDROID-ROADMAP-TODO-v2.md), [0.4.1 release notes and phone checklist](docs/RELEASE-0.4.1.md), [HP2 findings](native-runtime/docs/re/hp2/android-local-findings.md), and [build evidence](native-runtime/android/verification/).

Local native and Android Java compilation passed. Host validation includes 10 C++ tests, 74 existing Java checks, new save/import/progress/rollback suites and 13 Python release checks. APK signature, ZIP integrity, alignment, ARM64 architecture and 16 KiB ELF alignment passed. Both bundled game inventories and all payload hashes were verified. These checks do not replace physical-device playtesting.

Build instructions are in the release notes. The full source archive includes pinned SDL sources; it excludes SDKs, original game assets, HD packs, signing keys and passwords. Keep signing material private and retain the existing certificate for updates.
