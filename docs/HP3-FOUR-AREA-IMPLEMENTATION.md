# HP3 four-area implementation status

8 November 2026. Source follow-up; published v0.4.14 is unchanged. None of the four broad areas is declared complete.

| Area | Implemented and checked in this batch | Still open |
| --- | --- | --- |
| UE2 assets | PoA-only StaticMesh native class with actual section, vertex, normal, color, UV and index decoding. Material references/collision flags are read from original tagged material structs. All 797 original EU mesh exports pass the compiled loader harness. | SkeletalMesh UE2 layout, MeshAnimation tracks/notifications, TerrainPrimitive/Sector, native collision/editor trailer, static-mesh package rewriting. |
| Rendering/collision | Static-mesh actor visibility/bounds, location/rotation/pre-pivot/DrawScale3D transforms, UV submission, mirrored-scale winding, initial diffuse material routing and render passes. Triangle ray/swept-AABB/overlap paths, per-material collision flags and extent-trace flags; mesh bounds enter collision hashing. | Android GPU visual acceptance, lighting/lightmaps, layered materials/UV animation, simple collision-hull modes, skeletal/terrain rendering and full movement acceptance. Linear triangle scans also need profiling before performance claims. |
| Native/config | PoA config/globalconfig dynamic arrays read repeated keys, preserve defaults when a key is absent, and save complete lists. INI updates grow/shrink lists while retaining comments/unknown lines. Static audit now names unresolved Core declarations and call sites. | The 10 missing Core registration candidates in HP3-REMAINING-CORE-EVIDENCE.json; remaining Engine/game native behavior and conditional/ABI checks; object/map/delegate text and localized arrays. |
| Device acceptance | Host tests, original-data checks, native syntax checks and Android packaging tests. The preceding ARM64 build succeeded; this source requires its own ARM64 rebuild. CI now runs the original-data regressions before compiling ARM64. | No attached Android device or emulator is available here. Startup, menus, controls, character switching, map travel, rendering and save/load require actual device execution. No claim of playable HP3 or a newly signed APK. |

## Data and source evidence

- The published private EU APK supplied the original fixtures. Its SHA-256 is `05235ab2631b0e25f402f3f576566441190e9bae5af851380627d0f31d8784e9`.
- `HP3-STATIC-MESH-EVIDENCE.json` records package hashes and totals: 797 meshes, 116,099 vertices, 91,339 section triangles.
- Stream-layout reference: [UEViewer UnMesh2.cpp](https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMesh2.cpp) and [UnMesh2.h](https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMesh2.h). The bounded reader is independently implemented and verified against the supplied EU package versions 128/129; other versions fail explicitly.
- The EU Engine.u EDrawType declares StaticMesh as value 8. That routing is PoA-only and does not replace HP1/HP2's value-8 particle path.
- Material fields/blend values were read from EU Engine.u, not assumed from UE1. Texture upload now avoids the absent UE1 Texture.DrawScale field for PoA.
- Native mesh trailer bytes are bounded but not decoded. Geometry queries use section triangles; no claim of kDOP/simple-hull compatibility.

## Validation and limits

118 compatibility tests pass with original EU inputs enabled; 29 Android packaging tests pass. New compiled ASan/UBSan harnesses execute production mesh loader/material-preview bodies with bounded stream/object/UPrimitive doubles; the primitive test double skips the already established 41-byte bounds/sphere prefix. The captured-device rendering harness executes production transform, material and draw bodies with real engine matrix math. Collision tests execute production SAT. These are not full VM/world/GPU/device tests. Leak detection is disabled in the constrained host environment; no leak-check claim.

Config-array text supports primitive/name/string and supported nested structs/arrays. An explicit singleton `Key=()` marks an empty array for this port; a literal string `()` is saved quoted. This marker is a port convention, not a claim about every stock UE2 INI implementation. Unsupported member types preserve existing INI data. Unknown and commented lines survive the INI update, including repeated sections and fixed-index key spellings.

Do not mark the full asset, renderer, native or gameplay workstreams complete from these tests. The native candidate audit reports registration-name absence and static call sites; it does not assert that every present registration is usable in PoA.
