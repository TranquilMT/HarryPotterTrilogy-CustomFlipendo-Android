# 0.3.2 recovery work — APK pending

This source revision restores the launcher update with optional external HD packs. It is not an APK release and does not make Prisoner of Azkaban playable.

- Package remains com.tranquil.hp1native; replacement build uses versionCode 7, versionName 0.3.2-tq.
- Chamber of Secrets data must be supplied to build_launcher.py --hp2-data. Original disc recovery is incomplete; no complete APK has been produced for this revision.
- Missing or empty Default.ini and DefUser.ini repair remains enabled. Existing nonempty files and saves are preserved.
- HD ZIPs are excluded from the APK. The launcher opens the registered HD pack import activity. Imports enable only the selected game; disabling removes the recorded overrides.
- POA remains disabled in the launcher. Restored UE2 compatibility tools and headers are isolated research components, not a live game runtime.
- Original signing material was recovered privately. Never commit signing keys/passwords or include them in releases.

## Validation completed

- Android API 36 Java compilation passed.
- 12 synthetic external-HD checks passed for both profiles: missing embedded packs, imported overrides retained while enabled, removal when disabled.
- 17 POA Python tests passed.
- POA C++ typed-program harness compiled with -Wall -Wextra -Werror; corpus checks passed for typed frames, final/import links, primitive intrinsics and explicit unsupported-operation failures. Its exploratory smoke results are not gameplay coverage.

## Release gates still open

Complete HP2 game-data recovery; build and verify the signed APK; verify signing certificate continuity and absence of embedded HD packs; attach APK/source/checksums to GitHub and verify uploaded asset sizes/digests. Physical Android gameplay remains untested.
