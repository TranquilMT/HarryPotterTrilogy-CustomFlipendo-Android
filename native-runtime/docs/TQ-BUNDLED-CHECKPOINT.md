# Bundled private APK

0.1.1 bundles the user's uploaded original PC ISO's selected game files. The full 603,924,480-byte ISO was downloaded through the existing private vault's authenticated download route. The vault website was read, not changed. No new upload was requested.

The production IsoExtractor extracted 330 files totaling 530,761,842 bytes. Independent previous-recovery comparisons pass for all seven known script packages and HP.exe. The packaging tool adds a deterministic assets/gamedata/.stamp, required by SetupActivity.readStamp(), and all selected files. First-launch installation uses the existing progress UI. Existing .ini files and saves are retained. Signature/package continuity with engine-only 0.1.0 is required; versionCode is increased to 2.

On phone: install the new APK as an update, grant file access if requested, let first-launch installation finish, then press Play. Do not use Import PC ISO. Android execution and gameplay remain untested until the user reports the result.
