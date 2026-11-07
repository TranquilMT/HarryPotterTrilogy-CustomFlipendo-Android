# HP:Trilogy Mobile 0.4.13

Android ARM64 source port of the EU PC trilogy. All three games are bundled; HD packs remain optional and separate.

## HP3 startup compatibility

- Restore the EU disc's standalone `System/kwGame.u`, which supplies `KWGame.KWConsole`. Its SHA-256 is `de29aa642969ff5586559580fe05f29df6ca7fffa215dd50156d3be7e0f9b11e`.
- Require the script during installation and before launch; a Windows DLL cannot replace it.
- Guard absent optional IpDrv classes during property initialization while retaining strict required-package lookup.
- Include all seven EU Core native bindings: Acos, Asin, Greater_StrStr, GetLastStateName, QuatProduct, QuatRotateVector and QuatFromAxisAndAngle.
- Preserve previous-state tracking and the 0.4.12 font serialization fix.
- Retain HP1/HP2 fixes, separate settings and existing saves.

## Rebuild verification

- Recompile the ARM64 engine in GitHub Actions from commit `f008ea1d0cf441892dc15055951d9f867b99b689`; rebuild the Android launcher from the retained 0.4.13 source checkpoint.
- Version 0.4.13 / code 55; package `com.tranquil.hp1native`; permanent signing certificate retained.
- APK: 1,196,910,771 bytes. SHA-256: `ba0dc9aa83170b0907aaa523d6c79b0e0ca25728d3497d39561b271c9bbd47f9`.
- Verify every embedded payload file: HP1 339, HP2 901, EU HP3 767. Verify all four ARM64 native libraries and 16 KiB alignment.
- 106 host compatibility tests and 29 Android packaging tests passed. All 110 EU packages parsed without errors; host sanitizer checks passed.
- Retain the exact full-source archive including SDL. Signing keys and retail game files are excluded from that source archive.
- Preserve a 9.8 MB reconstruction payload in the private repository. Reconstruct the signed APK byte-for-byte from the pinned 0.4.11 base without a signing key; verify all GitHub release asset digests before publication.

## Install and test

Install the Three-Games APK over the existing app without uninstalling to preserve application data.

HP3 startup beyond language selection, first scene, rendering, controls and save/reload still require phone validation. The remaining UnrealEd import from QuidditchTerrain has unestablished runtime reachability; other UE2 systems remain unfinished. This build does not establish complete compatibility or gameplay readiness. Version 0.5.0 remains reserved for the confirmed HP3 release.
