# HP:Trilogy Mobile 0.4.13

Android ARM64 source port of the PC trilogy, using EU versions.

This local build includes all three game bundles. Install it over the existing app to preserve application data; do not uninstall first.

## Changes

- Recovered the EU disc's standalone `System/kwGame.u`, omitted by the earlier compressed.zip-only extraction. Its SHA-256 is `de29aa642969ff5586559580fe05f29df6ca7fffa215dd50156d3be7e0f9b11e`.
- Added mandatory package, launcher and installer checks for that script. KWGame.dll is not accepted as its replacement.
- Fixed unconditional initialization of optional IpDrv network classes when the game does not ship IpDrv.u. Required package lookup remains strict.
- Implemented EU Core.u native bindings for Acos, Asin, Greater_StrStr, GetLastStateName, QuatProduct, QuatRotateVector and QuatFromAxisAndAngle; retained the 0.4.12 font fix.
- Removed the previous regional profile and historical disc evidence from this checkpoint. The active HP3 profile and fresh dependency audit use the uploaded EU files.

## Verification and limits

106 host compatibility checks pass. All 110 EU packages inspected without parsing errors. The remaining missing import is UnrealEd from QuidditchTerrain; runtime reachability is not yet established. The source checkpoint includes the exact patched engine and refreshed canonical patches for the four engine files changed here.

The ARM64 engine builds successfully. Full APK packaging, certificate and payload checks are recorded separately. No Android device execution has been performed here: launching past language selection, first scene, rendering, controls and save/reload still require testing. GUI, meshes and other UE2 systems have known unfinished work; this build does not establish playability or release readiness for 0.5.0.

## Publication status

Local APK, full source ZIP, checksums and verification are complete. Uploading the binary release assets and selecting latest remains pending because browser publication was blocked.
