# EU SetPhysics stopping transitions

The PoA production setter now returns immediately when the requested mode equals the current mode. On an actual transition into PHYS_None (0) or PHYS_Rotating (5), it clears Velocity and Acceleration. Script/save bytes retain their original values. HP1/HP2 and other UE1 games retain their existing setter behavior.

These corrections follow original EU Engine.dll SHA256 73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a: unchanged-mode return at 0x1047f701–0x1047f713 and vector clearing at 0x1047f8b6–0x1047f919. A repeated mode also preserves the port's ladder marker; changed modes reset that marker as before.

## Verification

The original reference job passed 144 transitions among modes 0–11 (Actions 42643326159, commit 5604d1cb3fc848f3018bad26b367bf3d675ec50b). It executes only the original setter with synthetic zeroed actor storage, null base/mesh, profiler counters and a stubbed floor lookup. It excludes root locking, collision callbacks, Karma and gameplay. Captured JSON is retained in tools/poa/reference-profile/eu-physics-setter.json. Artifact ZIP SHA256: 535deeeb1e604c8a0f39768d1c30c5582ee6b31c682542d333f1f88c76b23e27.

Production-body regressions compare all 144 captured velocity/acceleration outputs and exercise all 65,536 old/new byte pairs for scoped stopping side effects. Engine.u confirms one SetPhysics argument and native index 3970. The 142-test compatibility suite passed with the setter corrections; the five focused physics tests also passed after adding the captured reference comparison. The changed C++ file and reverse application of patch 0215 passed local checks. This setter checkpoint requires its own ARM64 build. The preceding mode-mapping checkpoint passed Actions 42640609435.

## Root motion and base transitions remain open

Original setPhysics unlocks an existing skeletal root when leaving mode 12 (0x1047f782) and locks it when entering mode 12 (0x1047f8ae). LockRootMotion snapshots an anchored coordinate frame, root coordinates and consumed translation/rotation state. AActor.physRootMotion does not directly integrate velocity or consume that translation delta. Root queries, mesh/frame-driven movement, visual locking and persistence need implementation together.

Original base/floor changes, root-motion transitions, Karma hooks, broader UE2 movement/collision parity and device acceptance remain open. These stopping fixes do not complete root motion. No APK is published by this source checkpoint.
