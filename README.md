# Harry Potter Trilogy — Custom Flipendo Android

Private development repository for William Utting's Android port work.

Current build: **0.3.1-HD-bundled**, package `com.tranquil.hp1native`, versionCode 6. HP1 and HP2 HD modules are included in the APK and remain optional through the existing launcher HD textures option. Missing or empty `System/Default.ini` and `System/DefUser.ini` are repaired; nonempty settings and saves are preserved. Chamber of Secrets data is included in the APK; HP1 uses an existing installation/import. Prisoner of Azkaban remains disabled and is not playable yet.

The complete source archive contains native runtime and dependency sources, Android launcher, build scripts, host tests and HD-generation tools. Private signing material is excluded. Native libraries in the APK are unchanged from 0.1.0. Build/signature checks and host tests passed; real-device gameplay testing is pending.

Release assets are being uploaded. Original signing key and password must remain in private offline storage and must never be committed or attached to a release.
