# TQ native Android test checkpoint

Based on Flipendo Droid 7508365a02fc0f56d1f9303e37de446891534cc7; SurrealEngine c41d349e7e23584dac863367ba661bf90ef1c4f3 with upstream patches. GPLv3 applies to Flipendo and the new importer/build tooling. SurrealEngine retains its zlib license. No game data is committed into this repository.

The complete retail System/HP.exe was read from the user's existing Vault ISO. SHA1 18ba5889e3a3aa221be52dda2eae75e53e52351b is already listed as HARRYPOTTER1_433 in this pinned engine's database. The Windows executable was not executed.

## Added in this checkpoint

- Linux native build and APK assembly tool: android/build_linux.py. Uses Android SDK36 tools, target35 and ARM64/API29 native build. Supply SDK, NDK, JDK, CMake, Ninja, keystore and password paths explicitly. Reuse the key. The existing Windows script is unchanged.
- PC ISO import from the launcher and HP1 folder settings. Imports only System, Maps, Textures, Sounds and Music, into a unique app-specific game folder, and selects that folder for HP1. Existing installations and saves are not overwritten.
- ISO9660/Joliet directory/extent bounds, filename validation, recursion/file-count limits, extraction progress and failure cleanup. Uses a temporary ISO copy so SAF sources do not need random-access support. Needs free space for both the ISO and extracted data during import; temporary copy is deleted afterwards.
- Stable independent package ID com.tranquil.hp1native and report-provider authority. It can coexist with the viewer and upstream Flipendo.
- Android manifest debuggable=false; game debug mode is forced off and its touch/controller settings entries are excluded. This is still an untested native test candidate, not a validated gameplay release.

## Host verification

All Java sources (SDL glue, launcher, overlay and importer) compile against API36. The new ISO parser matches the user's original saved disc metadata: 548 total files, 330 selected game files, 530,761,842 game bytes. That test uses original directory bytes plus a sparse file of the original disc size, so it verifies inventory/extent parsing, not the entire ISO payload. Synthetic extraction checks actual file contents, refuses overwrites, and rejects traversal, out-of-range extents and truncated images.

## Phone test procedure

Install alongside the existing scene viewer. Permit the inherited file access request, press Import PC ISO, choose the HP1 PC disc image, and leave the import screen open. On success return to the launcher and press Play. Check menu, new game, tutorial cutscenes, Harry movement/camera, jump, casting lesson, sound and save/load. A native compile does not establish any of those behaviors. Record exact errors or a screenshot if a test fails.

The runtime is an existing open-source engine implementation. The separate recovered scene/character tools remain useful validation material; the native candidate does not use the offline sampled character binary from checkpoint009.
