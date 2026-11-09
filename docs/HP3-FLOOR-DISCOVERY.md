# Initial cylindrical floor discovery

PoA SetPhysics now invokes floor discovery for the retained-base modes None, Walking, Rotating, Spider and RootMotion, following the existing raw-mode policy. This does not implement the movement or root-lock behavior of those modes. Unchanged physics modes still return before querying, matching the traced setter.

The new production helper queries eight units downward using the actor's cylinder radius/height. It applies the established PoA world-geometry rule and the port's inherited actor/player blocking policy to the sorted collision results. It skips self/deleted hits, selects the first accepted result, and calls SetBase with callbacks only when the selected base differs. No floor hit clears a prior base; the query itself does not move or snap the actor.

Actor and selected-floor references are retained across callbacks. The surrounding setter checks deletion and uses the current mode after callbacks before stopping velocity/acceleration, preserving the preceding fixes.

## Shape/context limits

The EU enumeration is CT_Cylinder=0 and CT_Box=1. The current collision sweep interface does not preserve a box's separate width/shape. CT_Box and other unsupported shape values therefore return without querying or changing the existing base. Missing level context, deleted actors and invalid cylinder dimensions also leave the base intact. There is no silent conversion from a box to a cylinder.

The helper uses existing port collision sweeps, including their skin margin and inherited actor filtering. It does not claim exact implementation of original flags 223, box collision, all brush/encroacher rules or all floor-contact behavior. The floor-discovery helper now stores the selected normal in a reflected Floor property before changed-base callbacks; no-hit clearing uses zero, and unchanged bases leave Floor unchanged.

## Original reference

Original EU Engine.dll SHA256: 73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a. FindBase is at 0x1047b380; GetCollisionExtent is at 0x103218c0. The original query uses start Location, end Location-(0,0,8), dimensions from GetCollisionExtent and trace flags 223. Only a changed hit actor triggers SetBase.

Actions 42665237370 passed 36 cases using the original helpers with synthetic actor/level storage and recording trace/SetBase callbacks. Eighteen cylinder cases cover old/new bases; eighteen box cases retain the separate width requirement for future implementation. These cases validate query inputs and dispatch, not real collision geometry or VM events. Captured outputs are in tools/poa/reference-profile/eu-floor.json.

Artifact ZIP SHA256: dd2498220bb46c6299e436e54959fa4a5ac0bfb954959d5e20c1389b9f385c9e.

Production-body tests compare the eighteen cylinder controller cases, explicitly check deferred box behavior, and exercise filtering, context validation and reentrant/deleting callbacks through SetPhysics. Existing base-list, world-blocking and geometry corpus regressions remain separate coverage.

All 150 compatibility tests pass locally, including the original EU mesh/animation corpus. Changed production C++ passes syntax checks, and canonical patch 0218 passes reverse-application verification. The cylindrical baseline passed ARM64 build 42666512883; the subsequent normal-propagation change requires its own build.

## Acceptance still required

Box/width support, original trace-flag and wider normal handling, actor/brush/encroacher parity, platform/floor behavior on Android and root-motion locking/queries/translation/persistence remain open. No new APK is published by this source checkpoint.

See [the active collision checklist](HP3-ACTIVE-AREA-COLLISION.md) for the authoritative remaining work.
