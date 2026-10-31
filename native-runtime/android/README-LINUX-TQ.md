# Linux build for the TQ native test

This full-source archive includes the pinned, already-patched SurrealEngine source and SDL3 source. Do not run apply_patches.sh on the archive; it expects live submodule Git metadata. Build the prepared source directly.

Required external dependencies: Linux x86_64 JDK21, Android NDK27.2.12479018, Android platform36 and build-tools36.0.0, CMake and Ninja. Recorded download hashes/revisions are in toolchain-verification.json. CMake4.4.4 and Ninja1.13.2 were used. The existing scene-viewer setup_sdk.py can install the pinned SDK36 archives.

Run from native-runtime (replace every path with an absolute local path):

```sh
python3 android/build_linux.py --sdk /path/android-sdk --ndk /path/android-ndk-r27c --jdk /path/jdk21 --cmake /path/cmake --ninja /path/ninja --keystore /private/signing/release.jks --password-file /private/signing/password.txt --jobs 6
```

The separate private signing recovery archive supplies the permanent keystore/password for com.tranquil.hp1native, alias hp-native. Do not generate a replacement key for an installed release. The build script only generates a key when the provided keystore path does not exist.

The script configures ARM64 Android29 native code, sets 16 KiB maximum page alignment for shared libraries, builds the native runtime, compiles Java/Dex/resources with SDK36, assembles the APK, aligns it and verifies its signature. Result: android/dist/Harry-Potter-TQ-Native-0.1.0.apk. Build output contains no original game data; the on-phone ISO importer supplies the user's own files.

The inherited Windows packaging script is included as upstream source. Use build_linux.py for this build's package ID, importer, stable key and production manifest. Keep package ID/provider authority/signing continuity for future updates and increment versionCode/versionName before issuing an update.

## Bundled 0.1.1 build

Add `--game-data /absolute/path/to/extracted/game` to build Harry-Potter-TQ-Native-0.1.1-Bundled.apk. Version code is 2. The source APK's entire selected game data is added under assets/gamedata, along with a deterministic .stamp that enables the inherited first-launch installation. It uses the same package ID and signing key as the engine-only 0.1.0, so it updates that installation. Existing saves and .ini configuration files are retained.

This checkpoint used the user's existing Vault ISO, SHA256 6291cdcb3cd9338824d6428abeea9f528afbc0b5867af50bf091a93d0f29b744. All 330 selected files (530,761,842 bytes) were fully extracted with the production ISO reader; seven package files and HP.exe match previous recovery. Per-file hashes are in bundled-game-verification.json. The full ISO and extracted assets are build inputs, kept outside the Git source tree. The delivered bundled APK contains those assets, so no further upload or on-phone ISO import is needed.

The source archive contains all native code/dependency source and verification metadata. Original data can be recovered from the bundled APK's assets/gamedata subtree when reproducing the build; no original data is added to the upstream native Git repository.
