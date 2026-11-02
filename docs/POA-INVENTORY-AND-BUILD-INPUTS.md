# Complete disc metadata and build-input checkpoint

Local source: **0.4.3**, checkpoint `tq-0.4.3-inventory`. Related tasks: BASE-03, BASE-04, DATA-01 and DATA-04. PoA remains disabled and unplayable.

## Complete file-list inventory

The supplied PoA disc has 75 standalone files and 766 files in compressed.zip: 841 inventory records. The archive container itself is one standalone record; do not add standalone and expanded archive sizes as an installed-game footprint.

| Asset type | Records |
| --- | ---: |
| Script packages | 12 |
| Maps | 34 |
| Texture packages | 42 |
| Static mesh packages | 14 |
| Animation packages | 5 |
| Sound packages | 3 |
| Music files | 152 |
| Movies | 20 |
| Localization files | 427 |
| Configuration files | 7 |

These include all 110 Unreal packages. Remaining inventory records cover platform binaries, installer/support files and other files. The standalone System/kwGame.u is retained alongside the archived scripts. `POA-FULL-DISC-INVENTORY.json` contains every original path, normalized path, source, category, size and archive CRC/offset where available.

This is a **metadata inventory** made from retained original ISO directory records and ZIP central-directory bytes in a sparse disc view. Most payload extents are not populated. Declared CRCs are not verified payload hashes. The earlier startup checkpoint independently hashed 12 scripts and two maps; the remaining 96 Unreal package payloads still need extraction/hash verification. Dynamic runtime dependencies and installer destination rules remain open.

The complete inventory command is:

```sh
python3 native-runtime/tools/poa/disc_packages.py /path/to/original-disc.bin inventory.json --all-files
```

The default command still provides the earlier script-only report. Full inventory preserves original spelling, normalizes Windows separators, rejects unsafe paths and duplicate/case-colliding archive members, bounds member offsets against the archive payload area and reports cross-source conflicts without selecting an overwrite winner. Seven new tests cover both sector formats, asset types, traversal, collisions, invalid offsets and the distinction between corrupt payloads and valid metadata.

## Original signing continuity

The original private signing recovery backup was located and recovered outside the source tree. Exporting its public certificate with the existing hp-native alias produces SHA-256:

`25aa34c1db19a5007e934df03f5927e4b71a990856fe439a57e3e73ead8eae40`

It exactly matches the recorded permanent certificate. No new key was generated. The key and password are not included in the source ZIP or public reports. This verifies recovery of the original signing identity; it does not verify a newly signed APK or an update on the phone.

## Build input check and archive build correction

`native-runtime/android/build_preflight.py` reports missing SDK/NDK/JDK compiler tools, CMake/Ninja, signing inputs, previous APK/certificate and complete HP1/HP2 payload folders together. It reads/checks inputs without generating keys, compiling an APK or changing game data.

```sh
python3 native-runtime/android/build_preflight.py --output build-inputs.json
```

Supply the optional --sdk, --ndk, --jdk, --keystore, --password-file, --previous-apk, --hp1-data and --hp2-data paths to check prepared private inputs. Missing inputs return a nonzero status with a report; passing these checks is not proof of a successful native build or gameplay.

The standalone archive previously lacked Git history while build_linux.py required git rev-parse HEAD to create build-info.txt. The builder now accepts a verified source-lock checkpoint identity when there is no suitable Git checkout. It checks every locked source file and includes the lock's SHA-256 in build metadata. Changed/missing/unsafe files fail rather than inheriting the old checkpoint label. Git builds record their revision and clean/dirty status. Source edits require a reviewed fresh source lock or a Git checkout.

Five new preflight tests cover archive identity, modified source rejection, missing/unsafe locks and reporting multiple missing inputs without building. All 73 PoA Python tests and 18 Android packaging/preflight tests pass.

Current build inputs remain incomplete: SDK/NDK and compiler/build tools are not prepared, the previous signed APK is not recovered locally, and complete HP1/HP2 payload folders are not prepared. The Java runtime is present, but the Java compiler is not on PATH. These are explicit BASE-03/G0 blockers, not additional compatibility subsystems.

DATA-01 is complete as the full disc/archive file-list inventory. DATA-04 has a complete package-file list, but embedded material/mesh/GUI and other asset exports still require inspection. BASE-03 remains open for a full signed/payload-verified build and device continuity; BASE-04 remains open for complete asset identities and region evidence. G0–G6 remain open. No new APK or live PoA startup was produced.
