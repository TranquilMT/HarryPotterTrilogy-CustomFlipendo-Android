# Harry Potter TQ 0.3.0

Launcher update for HP1 and HP2. Same package ID and signing certificate as 0.1.x; install over the existing app to retain its app-owned files. Do not uninstall first.

- HP2 original PC data recovered from the supplied disc is included in the APK (766 files). Select Chamber of Secrets; allow its first-run extraction to finish.
- HP1 uses the existing imported game, an original HP1 ISO import, an extracted game-data ZIP, or a game folder. Its data and saves are preserved.
- Missing or empty System/Default.ini and System/DefUser.ini are repaired from game-specific defaults before launch/import completion. Nonempty user configurations are retained, including case variants.
- Incomplete installations report missing original files while leaving the other game selectable. Extracted ZIP imports use a new directory and validate required packages before changing the selected path.
- Prisoner of Azkaban is disabled and labelled not working yet. Its UE2 runtime remains unfinished and is not included here.
- Each supported game has Manage HD textures. Select multiple module ZIPs or the collection ZIP. Game labels, DDS mip chains and SHA256 hashes are checked before installing loose overrides. Failed writes roll back the affected previous DDS files. Remove HD overrides restores use of original package textures.
- Native libraries are unchanged from signed 0.1.0. Native behavior, compatibility limits and controls remain those of that runtime; this release does not claim new native engine fixes.

## Build

Use Android SDK build-tools 36.0.0 and android-36 platform with Java 17 (jdk.compiler module). Run android/build_launcher.py --help. Supply the prior runtime APK, recovered HP2 data directory, private original signing key and password file. Private key and original game data are not in this source archive.

The native source and bundled dependency sources are retained for reference and the existing android/build_linux.py supports a full native rebuild with the NDK. The launcher build deliberately reuses the verified prior native binaries.

## Validation

30 host checks cover default repair, package versions, wrapped/nested game imports, path traversal and duplicate rejection, wrong-game HD rejection, matching DDS/hash validation, rollback and removal. Android Java compilation, DEX generation, resource linking, signature, ZIP CRC and native-library identity checks passed. Real-device execution has not been tested in this environment.

Host tests require org.json:json:20240303 on the classpath (Android provides org.json in production). Compile GameFiles, GameZipImporter, HDPackInstaller and android/tests/LauncherTests.java with that test dependency, then run io.github.flipendo.spike.LauncherTests ORIGINAL_HP2_FOLDER HP1_HARRY_MODULE_ZIP.

Texture modules use full mipmapped DDS assets. Upscale targets are up to four times the original texture resolution, capped at 1024 pixels per dimension. Runtime procedural/parametric textures retain original behavior. No claim that every texture is 1080p or that the game has been completely modernized.
