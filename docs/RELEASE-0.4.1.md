# Harry Potter TQ 0.4.1 — local Android test build

Version code: **42**. Package: `com.tranquil.hp1native`. Native source commit:
`ae991c1`. This APK was built locally, signed with the permanent certificate,
for the GitHub 0.4.1 test release. It has not been playtested on a physical device.

## Changes

- Enable HP2's existing New Game / Load Game / Options front end on Android.
  Scale its layout and touch coordinates together for landscape surfaces.
  Confirm replacing an occupied game inside the same native window.
- Keep the existing six flat save files. Put runtime Save Game, Load Game,
  Resume, Options, Game Book, Return to Main Menu and Quit to Launcher under
  HP2's in-game menu; remove the HP2 quick-save button and launcher slot row.
- Route the front end's Display and Controls buttons to the Android settings.
  Retain HP2's Sound page and the earlier fixes for volume persistence,
  malformed volume values and cutscene mute command routing.
- Preserve the original `Help` folder in ZIP/ISO imports. It contains the
  menu's artwork and sounds. Earlier imports may lack it, so the menu has
  readable fallback buttons and a game title. Original artwork can be restored
  by copying the original HP2 `Help` folder alongside the imported `System`
  and `Maps` folders; replacing game data is not required for the text fallback.
- Fix save detection that could mistake a thumbnail for a saved game. Honor
  the requested load slot, validate slot bounds, retain the selected slot on
  internal restarts and notify the UI when an HP2 save finishes.
- Keep future persistent-world snapshots separately for each Android save
  slot. Legacy saves retain the previous shared-snapshot fallback until saved
  again. This cannot reconstruct different historical hub states that earlier
  releases already overwrote.
- When restoring a spawned persistent actor, fall back to already loaded
  classes if the imported INI omits its package from `EditPackages`.
- Change HP2's wand-effect frame from an off-axis bounding-box corner to the
  centre of its far end cap, and compensate for rendered wand length. This is
  a candidate Lumos alignment correction, pending an in-game comparison.
- Include the earlier mirror-plane grouping and bounded HD installer progress
  fixes. Installation reports copying, manifest saving and cleanup, with
  regression coverage for interruption and rollback.

## Install and test

Install over the existing APK without uninstalling it. **Both HP1 and HP2 game
payloads are bundled in this corrected APK.** HD packs remain separate optional
downloads. The earlier 9.9 MB engine-only 0.4.1 is superseded by this build.

The corrected APK is `Harry-Potter-TQ-0.4.1-Both-Games.apk`, **718,670,864 bytes**
(718.7 MB). It includes 339 HP1 payload files (531,064,131 bytes) and 682 HP2
payload files (512,852,516 bytes), plus both installation stamps. Every bundled
file hash was verified before signing. All four native libraries exactly match
the newly compiled 0.4.1 runtime. Version code 42 permits updating the accidental
engine-only build (code 41) as well as the preceding releases.

GitHub v0.3.3 confirms the reference bundle was 713,739,242 bytes. Its private
binary download was unavailable to this build environment, so the payloads were
recovered from the user's original private uploads instead. All 330 files in
the prior HP1 extraction manifest match; added Help/config files account for
additional entries. HP2 was extracted from its original installer cabinets and
the separate HGame.u disc file. Recovery evidence is recorded with the build. The package ID and signing certificate match the
previous release, but update installation itself still needs your phone test.

1. Check HP1 launch, menu, a saved game and controls before testing HP2.
2. HP2 Play should open its game menu. Check New Game, all six Load Game
   entries, back navigation and Options. Test both touch and gamepad cursor.
3. In HP2 gameplay, open MENU and save to an empty slot. Change something,
   then load that slot. Check replacement confirmation in an occupied slot;
   confirm a different slot was not changed.
4. Check Display, Controls and Sound settings; quit and relaunch to check
   persistence. Try a background/resume cycle with a menu open.
5. Test Lumos while standing, moving and casting. Record whether the light
   now sits on the visible tip, and whether the lingering-beam issue remains.
6. Spawn or collect beans, leave the hub and return, then repeat after saving
   and loading. Check two slots with different world states.
7. Check the opening PrivetDr mirror/window and the reported red translucent
   walls with HD off and on. These rendering reports remain unresolved.

The original book's empty Controls rows, complete 43-map progression, duels,
Quidditch and broader save compatibility are still open. Android's shared
Controls page is available through the new menu; that does not repair the
original book's rows. No full-playthrough or stable-release claim is made.

## Build and verification

Prerequisites: Linux x86_64, OpenJDK with `jdk.compiler` (17.0.20 used), Android
NDK r27c, SDK platform 36, build-tools 36.0.0, CMake 4.4.4, Ninja 1.13.2 and
SDL source release-3.2.20. Pinned revision/hashes and build results are in
`native-runtime/android/verification/0.4.1-*`. The full source archive includes
SDL source, but no SDK, signing key, password, retail game assets or HD packs.

Run from `native-runtime`, substituting local absolute paths:

```sh
python android/build_linux.py \
  --sdk /path/android-sdk --ndk /path/android-ndk-r27c \
  --jdk /path/jdk --cmake /path/cmake --ninja /path/ninja \
  --keystore /private/release.jks --password-file /private/password.txt \
  --previous-apk /path/previous.apk \
  --game-data /private/hp1 --hp2-data /private/hp2 \
  --version-name 0.4.1 --version-code 42 --jobs 6
```

The build refuses to generate a replacement signing key and compares the new
APK certificate with the previous APK. ARM64 ELF architecture, 16 KiB load
alignment, APK signature, ZIP integrity and alignment checks passed. Manifest
inspection confirms version 0.4.1 / code 42, minimum SDK 29 and target SDK 35.
Full native and Android Java compilation passed. Host checks passed: 10 C++
tests, 74 existing Java checks, new menu/import/slot and progress/rollback
suites, and 13 Python release tests. Synthetic fixtures were used for the host
Java checks; they do not establish retail game compatibility.


## Bundling regression guard

Both game-data arguments are mandatory in the release packager. It inventories
both games, validates the core packages and configuration files, writes both
stamps and reads back every embedded file to compare its SHA-256 hash. The
release fails if either payload is absent. Five additional regression tests
cover a valid dual bundle, either missing game, an engine-only APK, a missing
configuration and a missing HP1 core package. HD packs and private signing
material remain forbidden inside the APK. The release contains the corrected Both Games APK, source and checksums.

## Rebuild — 2026-10-31

Rebuilt locally from the recovered full source archive, with both original discs verified against the recorded SHA-256 hashes. The permanent certificate matches the retained 0.3.2 APK. This rebuild retains additional original HP2 Help/texture files; its updated inventory supersedes the earlier local artifact. Native and Java compilation, 74 Java checks with synthetic DDS fixtures, save/import/progress/rollback suites, 13 Python packaging tests, every embedded payload hash, signature and alignment checks passed. Physical-device testing remains pending. See `native-runtime/android/verification/0.4.1-rebuild.json` for this APK’s exact size and checksum.
