# HP:Trilogy Mobile

An Android ARM64 source port based on Custom Flipendo, bringing the EU PC versions of the Harry Potter trilogy into one launcher.

[Published releases — APK, full source and checksums](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases)

**0.4.14 is published and verified. [Download the signed three-game APK](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/download/v0.4.14/HP-Trilogy-Mobile-0.4.14-Three-Games.apk), [corrected full source](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/download/v0.4.14/HP-Trilogy-Mobile-0.4.14-Full-Source.zip), and [checksums](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/download/v0.4.14/HP-Trilogy-Mobile-0.4.14-SHA256.txt). Android version 0.4.14 / code 56 and the permanent certificate are verified.**

The ARM64 rebuild and signed-APK reconstruction workflows are available in Actions. The reconstruction verifies the base APK, exact signed output, package identity, certificate, alignment and uploaded asset hashes before publication. Private signing material is not uploaded.

## Current source compatibility work

The source branch includes the eleven audited Core declarations, repeated-array config handling, initial static-mesh rendering/collision, EU MeshAnimation decoding/sampling and initial skeletal geometry/CPU skinning/rendering and per-actor animation channels with local-space blending and script notify routing. Original-data tests cover all 797 static meshes, 140 animation resources and 167 skeletal meshes. Core and animation checkpoints passed ARM64 compilation; the skeletal checkpoint has its own build gate.

These changes are not in the published 0.4.14 APK. HP3 startup and gameplay remain unverified on Android. Original animation parity, remaining channel APIs/global blending/root motion/native-only notifies, terrain, richer materials/lighting, movement/travel/save acceptance and release signing remain open. See [the current roadmap](docs/HP3-COMPATIBILITY-ROADMAP.md) and [skeletal implementation scope](docs/HP3-SKELETAL-IMPLEMENTATION.md).

## 0.4.14 — KWGame range defaults

- Fix the reported `Name index out of bounds!: KWGame` while loading `CompanionCombatManager` defaults.
- Read EU PoA `RangeVector` and nested `Range` values as tagged properties; retain flat binary handling for UE1 and native vectors.
- Use the corresponding tagged save path and retain property-offset and serialized-size checks.
- Reproduce the former `SerializeFrom` misread and invalid name index with the exact EU export; verify the corrected reader and save/reload round trip.
- Retain all seven Core bindings, the EU `kwGame.u`, optional IpDrv guard and font fix from 0.4.13.
- HP3 startup and gameplay still need phone validation.

## 0.4.13 — HP3 startup compatibility

HP:Trilogy Mobile is the Android source port of the PC trilogy, using EU game versions.

- Recover the missing `System/kwGame.u` directly from the uploaded EU disc. It is a standalone disc file outside `compressed.zip`, and supplies `KWGame.KWConsole`.
- Require the script package during installation and pre-launch validation; a Windows DLL cannot replace it.
- Avoid loading absent optional `IpDrv` classes during property initialization. Actual required package imports still fail normally.
- Add seven verified HP3 core native bindings and track the previous UnrealScript state.
- Retain the 0.4.12 font serialization fix, all three bundled games, application identity and permanent signing certificate.
- HP3 startup beyond language selection, saves and gameplay still require phone validation. This is not a claim of complete UE2 compatibility.

## 0.4.10 — PoA engine update

- Include the `Core.Object` loader, UE2 casts, dynamic array operations and delegate fixes from 0.4.10-test.
- Package all three game bundles from the verified 0.4.9 APK. Android version code 52 lets this build update both 0.4.9 and 0.4.10-test.
- PoA startup and gameplay still need phone verification.

### 0.4.10-test — PoA engine update

- Fix version 129 class and struct metadata loading, including the `Core.Object` startup failure reported on 0.4.9.
- Read additional UE2 primitive casts and dynamic array operations used by the retail game.
- Load, assign and call UE2 script delegates, including menu callback bindings.
- The signed ARM64 test APK uses Android version code 51 and the permanent release certificate. Install it over 0.4.9 after that version has extracted the PoA files.
- The test APK uses existing game files. Phone startup and gameplay remain unverified. The bundled three-game release is available above.

## 0.4.9 — PoA folder-scan hotfix and settings correction

- Fix the new phone startup failure caused by the absent optional `TestMaps` folder.
- Skip unavailable configured map folders and tolerate directory-access errors during map discovery.
- Preserve errors raised while parsing files; missing optional folders do not mask required package failures.
- Remove the separate launcher settings button and dated native dialogs introduced in 0.4.8.
- Keep the existing Android **Display / Controls / Game** settings panel for HP3, with supported render-scale, FOV, FPS and touch controls.
- Retain the native PoA configuration, frame-pacing and FOV fixes from 0.4.8.
- The phone report confirms 109 packages are discovered; gameplay remains unverified.

## 0.4.8 — PoA startup hotfix

- Fix the first reported phone startup failure: **Could not find package Core**.
- Read PoA’s repeated package paths and separate user configuration correctly.
- Include the selected game’s installed folder and core/config file checks in diagnostic exports.
- Add pre-launch Display, Controls, Diagnostics and performance preset settings for all three games.
- Connect PoA frame pacing and widescreen/FOV settings; preserve independent game profiles.
- Preserve all three game payloads, game saves and application identity.
- PoA remains experimental; the next phone launch must confirm progress beyond package discovery.

## 0.4.7 — Trilogy testing update

- Rename the app to **HP:Trilogy Mobile**; retain application ID `com.tranquil.hp1native`.
- Use version **0.4.7**, Android version code **48**, and the installed package version in launcher diagnostics.
- Give all three games matching cover tiles, including a dedicated blue Prisoner of Azkaban tile.
- Keep each game's folder selection and launch requests separate.
- Bundle the original HP1 and HP2 data alongside the newly supplied **EU PoA** data in the three-game APK. Release packaging rejects incomplete or corrupted payloads.
- Keep HD texture packs optional and separate from the APK.

### Philosopher’s Stone

- Retain the original game payload and missing-default configuration repair.
- Retain mirror-plane rendering corrections and touchscreen input improvements.
- Check menus, existing saves, casting, movement and reflections after updating.

### Chamber of Secrets

- Retain the original English game payload, including HGame.u and dialogue.
- Retain the menu clipping correction and separate game settings.
- Check the main menu, six save slots, settings, duels and touch input on a phone.

### Prisoner of Azkaban — experimental

- Install the EU common and English game files, including animations, static meshes and movies.
- Verify bundled files by size and SHA-256 during installation; preserve existing nonempty configuration and saves.
- Accept original PoA folder/ZIP imports and validate UE2 version 129 packages.
- The earlier extraction omitted the EU disc’s standalone `System/kwGame.u`. Version 0.4.13 restores the script and requires it before launch; `KWGame.dll` cannot replace it.
- Retain bounded package loading, object-call checks and supported UE2 primitive-cast decoding.
- **PoA has not yet been verified to reach a rendered, interactive game on Android.** Character swapping, native game functions, menus, saves, rendering and performance require runtime acceptance. A successful APK build does not establish playability.

## Installation and testing

- Download the signed three-game APK and checksum file from the [0.4.14 release](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.14). The package version is 0.4.14 / 56; signing continuity and all uploaded asset hashes passed verification.
- Back up saves before updating. The release builder requires the permanent signing certificate used by 0.4.2; a differently signed 0.4.5 test APK may reject an in-place update.
- Install on Android 10 or later with an ARM64 processor. Allow enough free storage for the APK and extracted game files.
- Open the launcher, select the game tile, and let installation finish before starting.
- For PoA, test startup first. If it fails, export the launcher report and retain the exact error and game version.
- Test HP1 and HP2 launch, menus and an existing save as regressions.

## Source and roadmap

- [Release details](docs/RELEASE-0.4.7.md)
- [Complete PoA compatibility roadmap](docs/HP3-COMPATIBILITY-ROADMAP.md)
- Android launcher and build tooling: `native-runtime/android/`.
- Game data, SDK/NDK and private signing material are external build inputs, not source dependencies.
- Engine and third-party licenses remain in the source tree. Credit to Flipendo, SurrealEngine and their contributors; Harry Potter names and original game content belong to their respective owners.
