# Active area: collision and floors

Complete this area before starting root motion or another roadmap area, unless William changes the priority. Historical roadmap entries are checkpoints, not evidence that the whole area is complete.

- [x] Original EU world-geometry blocking reference and static-mesh trace classification.
- [x] Cylindrical floor discovery with changed-base callbacks and retained references.
- [x] Baseline ARM64 build: Actions 42666512883 passed.
- [x] Propagate the selected floor normal before base-change callbacks; clear it on a changed base with no hit and retain it when the base does not change.
- [x] Validated EU box/cylinder dimensions and full-XYZ static-mesh sweep entry point.
- [x] Box width through top-level queries, primitive actor collision, overlaps, placement/spawn probes and spatial indexing (source integration; acceptance below remains open).
- [ ] Original trace flags 223 and full actor/brush/encroacher blocking rules.
- [ ] Arbitrarily rotated/scaled brush query parity.
- [ ] Slopes, steps, moving platforms, base-relative motion and contact events.
- [ ] Terrain and blocking-volume collision coverage needed by EU levels.
- [ ] Android scene acceptance, variable frame rates and HP1/HP2 regression checks.

The previous unpushed box-collision batch could not be recovered from the current workspace. Its reported 155-test result does not establish coverage for the retained source. Box support remains incomplete and must be reconstructed and revalidated.

The floor-normal checkpoint follows the original EU APawn::SetBase at 0x103e5a40: Floor is stored before AActor::SetBase. Production-body tests cover sloped normals, callback ordering, unchanged bases and no-hit clearing. Host checks do not establish full collision or gameplay acceptance. Published APK v0.4.14 is unchanged.


## Box extent foundation

GetCollisionExtent dimensions are cylinder (radius,radius,height) or box (radius,width,height), with zero box width falling back to radius. Unknown shapes and invalid active dimensions are rejected; cylinder width is ignored.

TracePoaStaticMeshExtent passes all three dimensions into the existing world-space triangle SAT. Trace flags include the Y dimension, material exclusion is retained, and the cylinder wrapper delegates with (radius,radius,height). Gameplay box routing remains deferred until top-level queries and actor geometry are wired together.

Production-body checks cover width-sensitive wall contact at 6 versus 9 units, normals, width-only nonzero-extent flags, material exclusion, invalid dimensions and cylinder-wrapper behavior. Canonical patch 0219 records this foundation. This does not establish device acceptance or publish a new APK.

## Integrated box query checkpoint

PoA box movement and placement use validated (radius,width,height) dimensions. Spawn probes use the spawned class defaults. Script trace extents retain Y independently. Spatial indexing records full box width and cached removal dimensions. Floor queries support both captured shapes; unknown/invalid dimensions leave the base unchanged.

Top-level sweeps and overlap queries carry all XYZ extents into BSP and static-mesh tests. Primitive box/box sweeps use slabs; upright box/cylinder sweeps use the rounded-rectangle Minkowski sum and a vertical slab. Direct overlaps and sphere/cylinder queries account for box targets. World-only static meshes are indexed for world queries and excluded from actor-only iterators. Mover encroachment uses box width and the candidate check counter on the PoA path.

Rotated/scaled mover queries project the world box to a conservative local AABB. This may produce early contacts; exact oriented-brush parity remains incomplete. Initial penetration response, original flags 223, full brush/encroacher rules, steps/slopes/platform motion, and Android scene acceptance remain open. This checkpoint does not complete the collision area.

Checks include 4,000 deterministic swept primitive comparisons against an independent distance/bisection oracle, production-body trace/overlap/placement queries, all 36 original floor controller cases, and existing world-classification regressions. The primitive oracle tests geometry, not original-game penetration response. Canonical patch 0220 records engine integration; hp3 helper and test files are tracked separately.
