# EU world-geometry collision filtering

PoA now treats bWorldGeometry hits through the world-collision rule before applying inherited actor/player blocking flags. Original EU AActor.IsBlockedBy at 0x103e2910–0x103e292f returns the moving actor's bCollideWorld value immediately for world geometry. The override is applied to the existing movement-blocking, touch and encroachment checks. Non-world actor/brush/encroacher policy otherwise remains inherited and is not claimed complete.

The production TraceFirstHit filter also classifies world-geometry actors as world hits. This prevents a static mesh returned by the geometry trace from being discarded simply because actor tracing is disabled.

| Static-mesh classification | Trace selection |
| --- | --- |
| bWorldGeometry true | traceWorld |
| bWorldGeometry false | traceActors and bCollideActors |

PoA mesh triangle routing follows that selection, and the fast world-ray candidate check no longer classifies every static mesh as world geometry. These routing corrections preserve inherited owner and actor-category filtering where applicable; they do not implement every original UE2 trace flag or collision shape.

## Reference and verification

Engine.dll SHA256: 73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a. Actions 42653655827 passed 512 original world-geometry blocking cases with synthetic actor flag storage. The cases vary all low-byte mover collision flags and two obstacle flag patterns, confirming that the world branch depends on the mover's world-collision bit. The job excludes ordinary-actor/brush/encroacher branches and full collision queries.

Captured outputs are retained in tools/poa/reference-profile/eu-world-collision.json. Artifact ZIP SHA256: 6fad304945f915c187f9d988fa7a56cd11d7fdd97c160c9bb7b1cb2706718494.

Production-helper tests compare all 512 reference cases. Additional tests execute the production TraceFirstHit body, exercise static-mesh flag combinations, optional false overrides, owner filtering and the legacy-game gate, and inspect EU flag declarations and integration sites. Existing mesh geometry/collision corpus regressions remain part of the compatibility suite. Full Android movement, original scene comparisons and query performance still require validation.

All 147 compatibility tests pass locally. The changed production C++ files pass syntax checks, and canonical patch 0217 passes reverse-application verification. This source checkpoint still needs its ARM64 build and device acceptance.

## Still open

Floor discovery/replacement, collision-width and other shape handling, the remaining UE2 trace flags and actor/brush/encroacher rules, root-motion locking/queries/translation/persistence and device acceptance remain open. The original FindBase trace is eight units downward; it has not been connected by this filtering update. The published v0.4.14 APK is unchanged.
