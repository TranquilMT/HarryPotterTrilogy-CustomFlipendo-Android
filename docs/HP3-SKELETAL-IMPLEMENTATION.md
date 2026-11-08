# EU skeletal mesh source checkpoint

## Implemented

- A PoA-only native SkeletalMesh class derives from the existing C++ mesh hierarchy but reads the EU UE2 layout after UPrimitive, bypassing incompatible UE1 mesh serialization. HP1/HP2 keep their existing class registration.
- Version 129 / internal version 4 is verified against all 167 original EU meshes. The reader retains material slots and references, mesh transforms, bones, full-resolution points/wedges/triangles and float bone weights. Absolute lazy-array endpoints are validated. Counts, reference indices, finite coordinates, negative weights and unsupported versions fail explicitly.
- Optimized LOD streams and collision metadata are bounded and consumed but not interpreted for rendering or collision. No package rewriting is implemented.
- CPU skinning composes hierarchical transforms, inverts the reference pose and normalizes accumulated weights. Unweighted points keep their reference positions. Bone names map animation tracks to mesh bones. Missing animation channels retain reference values; unavailable low-memory Hermione translations use the earlier documented bind-pose policy.
- The PoA renderer samples actor base AnimSequence/AnimFrame fields and routes skinned triangles through the initial diffuse-material path. It applies actor and mesh transforms, UVs, mirrored winding, masked/translucent passes and zero-scale rejection. A missing sequence renders the bind pose.
- Default animation sequences populate the existing actor sequence list with frame/rate/group data and function-only notifies. This is metadata integration with existing actor code, not verified UE2 animation-state parity.

## Evidence

`HP3-SKELETAL-EVIDENCE.json` records original package hashes and corpus totals: 167 meshes, 8,790 bones, 105,997 points, 163,721 triangles, 173,285 influences and 668 serialized LOD records. Every original mesh passes the compiled production reader and bind-pose skinning identity check. These five packages contain no serialized socket entries; socket parsing has no positive original-data acceptance claim.

Three ASan/UBSan host tests cover the corpus, weighted translation/rotation, parent composition, animation-name mapping, cycle rejection, invalid influences, truncation, lazy endpoints and captured production render-body output. Captured-device tests verify animated vertex positions, transforms, UVs, missing-sequence fallback and material passes. Adapters replace the object/stream/device interfaces; this is not a live world/VM/GPU test. Leak detection is disabled in the constrained host environment.

The full compatibility suite passes 129 tests; Android packaging regressions are checked separately. Production C++ integration receives a syntax check and an independent ARM64 Actions build gate.

Layout and coordinate references inspected: [UEViewer UnMesh2.cpp](https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMesh2.cpp), [UnMesh2.h](https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMesh2.h), [SkelMeshInstance.cpp](https://github.com/gildor2/UEViewer/blob/master/MeshInstance/SkelMeshInstance.cpp) and [Math3D.cpp](https://github.com/gildor2/UEViewer/blob/master/Core/Math3D.cpp). This port uses active local-to-world quaternion transforms; the reference uses world-to-local axes and parent UnTransformCoords. Their root conjugation corresponds to conjugating child quaternions in this active-transform representation. Tests include a nontrivial root/child rotation chain. Original animated playback still needs an independent visual comparison.

## Remaining animation integration

- Correct UE2 actor-native signatures and semantics, including optional integer animation channels. Original Engine.u declares PlayAnim with four parameters; the inherited UE1 registration has three. LoopAnim's fourth parameter is an integer channel in PoA but a float minimum rate in the inherited UE1 handler. FinishAnim/IsAnimating also have optional channel parameters. This batch does not silently declare those ABIs complete.
- Implement per-actor LinkSkelAnim overrides, optional replacement meshes and lifecycle/GC ownership without changing other actors sharing a mesh. The current render path uses the mesh's default animation resource.
- Implement channel blending, tweened poses, blend masks, root motion and original AnimEnd/FinishAnim behavior.
- Dispatch object-based animation notifies for sound/effects/footsteps; function-only notify metadata alone does not cover them.
- Implement actor/bone attachments and verify wand/item positioning.
- Add optimized LOD selection, character lighting, measured cache/performance work and Android GPU acceptance. The initial path skins full-resolution geometry on the CPU and recomputes it per render pass.
- Validate real startup, scenes, character switching, movement, cutscenes, save/load and HP1/HP2 regressions. No claim of playable HP3, original playback parity or a newly signed APK.
