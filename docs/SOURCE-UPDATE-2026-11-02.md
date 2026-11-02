# Source update - 2026-11-02

## Adopted checkpoint

- Replaced the working tree with the user-supplied standalone source checkpoint `Harry-Potter-TQ-0.4.3-Full-Source.zip` (0.4.3 / versionCode 44, canonical base `d367c77c745a776c7178ba7218a27b115c75f025`).
- Integrated the HP3 Prisoner of Azkaban compatibility roadmap pack (`HP3-POA-Compatibility-Roadmap-Pack.zip`); its docs are identical to the checkpoint except an extra root copy of `HP3-POA-UE2-FULL-COMPATIBILITY-ROADMAP.md`.

## Carried-forward local changes

- Ported the upstream Italian HP1 detection entries (issue #15) into both `native-runtime/src/engine/SurrealEngine/UE1GameDatabase.h` and `native-runtime/src/surreal-patches/0001-game-detection.patch` (patch blob `6020d12b`).
- Added `native-runtime/android/build_windows.py` and `README-WINDOWS-TQ.md` (Windows ARM64 build/sign pipeline mirroring `build_linux.py`).
- Restored the extracted HP1/HP2 game payloads (339 + 682 files) from the previous working tree; stamps unchanged.
- Preserved verification JSONs from the previous tree.

## Verification

- Python release tests: `test_release_checks.py` (8), `test_both_games.py` (5), `test_build_preflight.py` (5) all pass on Python 3.12.10.
- Host tests: not re-run in this batch (unchanged engine).

## Build

- Test-signed ARM64 build in progress: versionName 0.4.3, versionCode 44, throwaway key (certificate differs from 0.4.2; `--allow-certificate-change`), `--jobs 8`.
- Permanent signing key and GitHub publication still pending user input.

## Constraints preserved

- Package ID `com.tranquil.hp1native`, HP1 behaviour and saves, optional HD packs.
- HP3 remains disabled/experimental.
- No signing keys, passwords or copyrighted game assets committed.