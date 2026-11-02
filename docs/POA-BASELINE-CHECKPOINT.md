# PoA baseline checkpoint — poa-base-20261101

This source checkpoint reconciles the retained complete 0.4.1 source archive with canonical main `d367c77c745a776c7178ba7218a27b115c75f025` (0.4.2) and the later cumulative isolated PoA compatibility overlay. The source is consolidated; a new Android build has not been produced.

## Preserved changes and verification

- Restore canonical release/build metadata and the HP2 Canvas clipping correction, including its host regression.
- Retain the later cumulative `src/compat/ue2` and `tools/poa` work, corresponding verification reports and master compatibility scope.
- Merge canonical release history into the project roadmap while retaining the more recent PoA checklist.
- Refresh the Canvas portion of `0003-cursor-focus.patch`. Canonical 0.4.2 changed the vendored header without updating this saved patch; a reset/reapply could otherwise lose the correction.
- Verify the documented engine revision against the upstream Git tree: `c41d349e7e23584dac863367ba661bf90ef1c4f3`. All 3,254 untouched upstream files match. Reversing all 11 patches restores every patched upstream file hash; reapplying all 11 reproduces all 127 affected files exactly.
- Preserve package `com.tranquil.hp1native` and the recorded certificate fingerprint `25aa34c1db19a5007e934df03f5927e4b71a990856fe439a57e3e73ead8eae40`. No game payload, save file, signing key or certificate was replaced by this source reconciliation.

Host evidence is in `POA-BASELINE-VERIFICATION.json`: 54 PoA Python tests, 13 Android packaging Python tests, the Canvas clipping regression and 12 isolated PoA ASan/UBSan suites pass. Leak detection is disabled because LeakSanitizer fails under the execution environment's ptrace; memory-leak coverage is not claimed. A pre-existing trailing-whitespace warning occurs in 0100-knowwonder-core.patch during reversal; it does not prevent the verified patch round trip.

## Dependency and recovery record

- Engine: exact revision above; complete vendored source plus all routed patches are retained. `POA-SOURCE-LOCK.json` records the final source file hashes.
- Android SDL: vendored source, documented release-3.2.20, commit `96292a5b464258a2b926e0a3d72f8b98c2a81aa6`; exact included files are covered by the source lock.
- NDK 27.2.12479018, SDK platform 36, build-tools 36.0.0, CMake 4.4.4 and Ninja 1.13.2 are recorded by `native-runtime/android/verification/0.4.1-toolchain.json`, including archive hashes. That report records JDK 17.0.20; older historical reports differ. This checkpoint does not establish a newly provisioned working toolchain.
- Reconstruct this checkpoint using the existing complete `Harry-Potter-TQ-0.4.1-Both-Games-Full-Source.zip` and this checkpoint's recovery package. The recovery tool checks the exact base archive hash, overlays changed/new source and verifies the complete locked file set. It requires a new destination and does not overwrite an existing working tree.
- No original game data, extracted retail scripts, private signing material or compiled checkpoint test executables are included in the recovery package.

## Open acceptance

BASE-01 and BASE-02 cover source consolidation and a pinned recoverable checkpoint; they do not prove a reproducible signed APK or preserve-on-device update. BASE-03 remains open for artifact/device verification, BASE-04 for original-data identity, BASE-07 for original startup/first-scene selection, and BASE-08 is maintained throughout the work. BASE-05/BASE-06 are documented design decisions in `POA-PRODUCTION-ARCHITECTURE.md`; they are not implemented runtime integration.

G0 remains open. All live PoA loading, first-frame, interaction, persistence and release gates remain open. PoA remains disabled and unplayable.
