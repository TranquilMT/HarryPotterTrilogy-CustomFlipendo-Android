# Harry Potter Trilogy — Custom Flipendo Android

Private development repository for William Utting's Android port work.

## Downloads: v0.3.3

[Release and APK downloads](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.3.3). Sign into an account with repository access.

The locally built v0.3.3 APK bundles HP1 and Chamber of Secrets, includes Default.ini/DefUser.ini repair, and retains the app signing certificate. Prisoner of Azkaban remains disabled. User reports HP1 works and HP2 is playable with issues; POA is not playable.

## OPTIONAL HD TEXTURE PACK — compatibility fix

Download **HP1-HD-Refresh-All-Modules-v0.2.1-Compatible.zip** from the v0.3.3 release Assets. This separate optional HP1 download contains all six modules and 3,505 textures. It fixes the missing `game: HP1` compatibility metadata in the older HP1 v0.2 modules. Texture bytes and per-texture checksums are unchanged. Use the corrected v0.2.1 pack instead of the older v0.2 packs.

In Philosopher's Stone, open **HD textures > INSTALL HD PACK ZIPS** and select the combined ZIP directly. Do not extract it. Keep the installation screen open. No APK reinstall is needed. This pack is for HP1 only, not HP2 or POA. HD textures remain outside the APK and optional in the launcher.

## Source recovery

The recovered native runtime, Android launcher, dependency sources, build scripts, tests and POA research tools are under `native-runtime/`. See [recovery status](docs/RECOVERY-0.3.2.md) for the earlier recovery checkpoint; that historical document is not the current release status. The v0.3.3 launcher source archive is maintained separately from the older repository recovery snapshot.

The older 0.3.1 draft preserves its source archive, installation notes, verification and checksums. No standalone HD texture ZIPs were attached to that draft. Its HD-bundled description is historical and does not describe the v0.3.3 APK.

Original signing keys and passwords must stay private and must never be committed or attached to a release.
