# Harry Potter Trilogy — Custom Flipendo Android

Private development repository for William Utting's Android port work.

## Current source: 0.3.2 recovery

The recovered native runtime, Android launcher, dependency sources, build scripts, tests and POA research tools are uploaded under `native-runtime/`. See [recovery status](docs/RECOVERY-0.3.2.md) for validation and remaining release gates.

- Package: `com.tranquil.hp1native`; planned versionCode 7, versionName `0.3.2-tq`.
- Chamber of Secrets launcher support is present; complete HP2 data recovery is still required to build the bundled APK.
- Missing or empty `System/Default.ini` and `System/DefUser.ini` repair is retained. Existing nonempty settings and saves are preserved.
- HD texture packs are external and optional through the launcher; they are excluded from the replacement APK.
- Prisoner of Azkaban remains greyed out and is not playable. UE2 tools are research components, not a completed game runtime.

## Release upload status

The older 0.3.1 draft contains its source archive, installation notes, verification and checksums. It has **no APK attached**. Its HD-bundled release description does not describe the recovered 0.3.2 source.

No complete signed 0.3.2 APK has been produced or uploaded. The remaining gates are complete HP2 data recovery, APK assembly, signing-continuity and content verification, and APK/source/checksum release uploads. Android device gameplay remains untested.

Original signing keys and passwords must stay private and must never be committed or attached to a release.
