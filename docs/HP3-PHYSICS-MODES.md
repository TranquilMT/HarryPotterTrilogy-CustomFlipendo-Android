# EU physics dispatch and root-motion evidence

The original EU Engine.u EPhysics enumeration has sixteen values. Its values 7–11 differ from the inherited UE1 runtime. The production PhysicsMode query interprets them for shared native handler selection while Physics(), SetPhysics, script values and package serialization retain the original byte.

| EU value | EU meaning | Shared handler |
| --- | --- | --- |
| 0–6 | None through Projectile | Same values |
| 7 | Interpolating | UE1 Interpolating (8) |
| 8 | MovingBrush | UE1 MovingBrush (9) |
| 9 | Spider | UE1 Spider (10; inherited handler is empty) |
| 10 | Trailer | UE1 Trailer (11) |
| 11 | Ladder | Explicitly unsupported |
| 12 | RootMotion | Explicitly unsupported |
| 13–14 | Karma / KarmaRagDoll | Explicitly unsupported |
| 15 | PushPulled | Explicitly unsupported |

Pawn spider checks and reachability dispatch use the same interpretation. Handler selection is re-evaluated each physics substep so callback-driven changes retain their meaning. Bone-attached actors continue through the attachment path. HP1/HP2 and other UE1 games retain their existing mode interpretation and KnowWonder physics path.

Unsupported PoA modes now report their raw number instead of silently running rotation alone. Correct routing does not establish original movement behavior: interpolation, movers and trailers still use inherited handlers, Spider is empty, and UE2-specific properties/collision/ladder behavior need implementation and device validation.

## Root-motion trace

Verified original Engine.dll SHA256: `73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a`.

- Actor.execGetRootLocation at `0x104d40a0` checks for a skeletal mesh, obtains its instance and delegates to the mesh-instance query.
- MeshInstance.GetRootLocation at `0x104e1570` returns cached position at instance offset `0x1b0`, refreshing through GetFrame mode 3 when dirty and owned.
- GetFrame writes root coordinates to `0x278`/`0x2a8` and root position/rotation to `0x1b0`/`0x1bc` at `0x104f7892–0x104f7915`. Its imported FCoords operator combines the sampled root frame with instance coordinates at `0x1e0`.
- LockRootMotion starts at `0x104e17e0`, sets mode/dirty state, and stores an owner/mesh-derived coordinate frame at `0x1e0` around `0x104e1a8b`. It snapshots root coordinates at `0x218`/`0x248`, the consumed translation at `0x1c8`, and rotation at `0x1d4`.
- GetRootLocationDelta at `0x104e7010` subtracts and advances the consumed-translation cache; the existing traced rotation-delta query returns zero.
- Actor.physRootMotion at `0x10485670–0x104857cb` does not directly integrate velocity or consume the translation delta. A velocity-only implementation would miss the mesh/frame movement path.

The cached anchored frame and visual root lock must be integrated with channel sampling, movement/collision, actor transforms and save/load. No root-location or translational root-motion implementation is claimed by this dispatch correction.

## Verification

All 140 compatibility tests pass locally, including the original EU asset corpus regressions. Changed C++ files pass syntax checks and the canonical engine patch passes reverse-application verification. This checkpoint passed ARM64 Actions 42640609435.

The original enum regression reads Engine.u directly. Production TickPhysics/PhysicsMode/SetPhysics body tests check dispatch, raw-value preservation, all 256 bytes in both game modes, unsupported-mode rejection, substep mode changes, attached actors and the legacy KnowWonder path. Separate source checks guard pawn spider/reach routing. Android movement and game playability remain unverified. This source update does not change the published APK.
