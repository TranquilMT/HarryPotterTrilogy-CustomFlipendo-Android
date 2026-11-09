# Floor discovery: cylinder and box query integration

PoA SetPhysics now invokes floor discovery for the retained-base modes None, Walking, Rotating, Spider and RootMotion, following the existing raw-mode policy. This does not implement the movement or root-lock behavior of those modes. Unchanged physics modes still return before querying, matching the traced setter.

The new production helper queries eight units downward using the actor's cylinder radius/height. It applies the established PoA world-geometry rule and the port's inherited actor/player blocking policy to the sorted collision results. It skips self/deleted hits, selects the first accepted result, and calls SetBase with callbacks only when the selected base differs. No floor hit clears a prior base; the query itself does not move or snap the actor.

Actor and selected-floor references are retained across callbacks. The surrounding setter checks deletion and uses the current mode after callbacks before stopping velocity/acceleration, preserving the preceding fixes.

## Shape/context limits

The EU enumeration is CT_Cylinder=0 and CT_Box=1. Box queries now preserve CollisionWidth; zero width falls back to CollisionRadius. Unknown shape values, invalid dimensions, missing context and deleted actors leave the existing base intact. Cylinder queries retain the existing path.

The helper uses existing port collision sweeps, including their skin margin and inherited actor filtering. It does not claim exact implementation of original flags 223, exact rotated-brush collision, all brush/encroacher rules or collision normals. The port SetBase interface still lacks the original floor-normal argument.

## Original reference

Original EU Engine.dll SHA256: 73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a. FindBase is at 0x1047b380; GetCollisionExtent is at 0x103218c0. The original query uses start Location, end Location-(0,0,8), dimensions from GetCollisionExtent and trace flags 223. Only a changed hit actor triggers SetBase.

Actions 42665237370 passed 36 cases using the original helpers with synthetic actor/level storage and recording trace/SetBase callbacks. Eighteen cylinder and eighteen box cases cover old/new bases and query dimensions, including separate box width. These cases validate query inputs and dispatch, not real collision geometry or VM events. Captured outputs are in tools/poa/reference-profile/eu-floor.json.

Artifact ZIP SHA256: dd2498220bb46c6299e436e54959fa4a5ac0bfb954959d5e20c1389b9f385c9e.

Production-body tests compare all 36 controller cases, explicitly check unsupported shape behavior, and exercise filtering, context validation and reentrant/deleting callbacks through SetPhysics. Existing base-list, world-blocking and geometry corpus regressions remain separate coverage.

All 150 compatibility tests pass locally, including the original EU mesh/animation corpus. Changed production C++ passes syntax checks, and canonical patch 0218 passes reverse-application verification. This checkpoint still requires its own ARM64 build.

## Acceptance still required

Box/width support, original trace-flag and normal handling, actor/brush/encroacher parity, platform/floor behavior on Android and root-motion locking/queries/translation/persistence remain open. No new APK is published by this source checkpoint.

The integrated box path is a source checkpoint. Original trace flags, exact rotated-brush queries and real scene behavior remain acceptance work.
