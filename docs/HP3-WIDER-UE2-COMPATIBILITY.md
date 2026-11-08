# Wider UE2 compatibility: Core API extension

8 November 2026. Source work following the seven-binding/config audit; this is not a new APK release.

## Implemented

- **QuatInvert:** bind the EU quaternion input/return signature to the engine's rotation-quaternion conjugate. Intended for unit rotation quaternions; no general non-unit inverse claim.
- **QuatFindBetween:** bind two vectors to the shortest-arc quaternion helper, with a zero-vector identity guard. Cover parallel, opposite, perpendicular and non-unit input vectors.
- **InterpCurveGetOutputRange:** find the range of OutVal across the curve points. Empty curves return zero endpoints.
- **InterpCurveGetInputDomain:** find the InVal domain across curve points, including unordered values. Empty curves return zero endpoints.
- **GetIniEntry:** route the three-string EU signature to the engine INI store. Accept a case-insensitive trailing .ini suffix, preserve dots elsewhere, use the system alias for an empty package name, and return the engine's empty default for an absent key.

All five are registered by name (EU native index 0) only when launching PoA. PoaCompat.cpp is included alongside PoaCore.cpp without depending on the historical HP3_ENABLED experiment switch. HP1/HP2 registration remains unchanged.

## Evidence and representation

The bundled EU Core.u declarations confirm scalar/struct parameter types, argument order, out parameters and return types. InterpCurve contains exactly one Points array; InterpCurvePoint contains float InVal and OutVal fields. These inspected schemas and package hash are retained in HP3-WIDER-UE2-CORE-EVIDENCE.json.

The curve adapter has a single native ScriptArray member, matching the inspected container shape. Point values are accessed through runtime UStructProperty reflection rather than hard-coded float offsets. The implementation rejects missing/wrong field types, fixed-array fields, out-of-bounds fields, invalid element pitch and missing backing storage for a nonempty curve.

No direct calls to these five declarations were resolved in the twelve supplied script packages. They may be called through runtime lookup or native code, but that has not been established. This is added API coverage and does not prove that a startup blocker was removed.

## Mesh and terrain findings

The earlier phone log lists missing Engine.MeshAnimation, StaticMesh, MeshInstance, StaticMeshInstance, ConvexVolume, TerrainPrimitive and TerrainSector classes. The existing UStaticMesh implementation is gated to Unreal 227 registration and its loader logs unimplemented behavior. Reusing it as a UE2 implementation would not provide PoA mesh decoding, animation, collision or rendering.

No placeholder mesh classes are added here. UE2 asset-format analysis and functional loaders remain required, followed by Android rendering and gameplay checks. Other known gaps include native game APIs, GUI behavior and save/load acceptance.

## Validation

- Compiled production PoaCompat function bodies pass ASan/UBSan tests using reflection and INI service test doubles.
- Tests cover unit inverse/composition, perpendicular/opposite/parallel alignment, zero vectors, empty and multi-point curves, deliberately relocated float fields, missing/out-of-bounds fields, INI suffix handling and engine-store routing.
- Host C++20 syntax checks pass for PoaCompat.cpp and the modified PackageManager.cpp with production engine headers.
- Entire host suite: 111 tests run, 110 passed, one optional original-data regression skipped.
- The prior seven-native adapter/state/config harnesses remain in the suite.

This does not execute the full production VM, Android renderer or game. The current published 0.4.14 APK does not include these source changes. The previous release source lock is historical until the next full source package is assembled.
