Harry Potter TQ 0.3.1 — HD packs included in the APK

Install this APK over the existing app; do not uninstall first. The package ID and signing key are unchanged. Android 10+ ARM64; allow All files access when requested by the existing runtime.

Both HP1 and HP2 HD packs are embedded in this APK. No separate pack download or file picker is needed. Use the existing HD textures checkbox in the launcher, with the game selected. It is optional and off by default; each game's selection is saved independently. Enabling it installs that game's six modules from the APK. Disabling it removes the recorded DDS overrides and the engine uses the original package textures. Original game packages, saves and nonempty ini files are preserved. Enabling again restores the embedded pack.

Wait for texture installation to finish before pressing Play. All modules are bundled, including characters, NPCs/creatures, environment, props/details, effects and interface/storybooks: HP1 3505 textures, HP2 3697. The APK is about 1.8 GB; installing both packs expands around 4.6 GB of DDS data, and staging needs additional free space. HP1 original game data uses your existing installation/import; HP2 original game data remains included.

Default.ini and DefUser.ini are repaired if missing or empty. Prisoner of Azkaban remains greyed out and unavailable. Native libraries are unchanged from 0.1.0, with the same compatibility limits. Build, signature, embedded-module checksum and host behavior checks passed. Real-phone launch/play testing remains pending.

Build source: android/build_bundled_hd.py --help. Supply the verified 0.3.0 APK, twelve v0.3 module ZIPs, Android SDK 36 tools/platform, Java 17 and the existing private signing key. The source archive excludes signing secrets and original game assets. Tests cover optional enable/disable/restore, repeat launches, per-game state, incomplete bundles, config repair and HD validation/rollback.
