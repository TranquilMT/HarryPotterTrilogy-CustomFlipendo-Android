# PoA production architecture decision

Decision date: 2026-11-01. Tasks: BASE-05 and BASE-06. Status: design selected; implementation and live acceptance remain open.

## Decision

Extend the existing production engine through an explicit PoA format/runtime profile. Keep its PackageManager, UObject ownership, garbage collector, world and frame loop as the single production object model. Port verified UE2 decoding and execution semantics into that path incrementally. Do not turn ReceiverProgram's synthetic receiver registry into a second actor system.

The isolated hp3 tools remain format inspectors and regression oracles. Their numeric identities, explicit host defaults and scalar property maps are test scaffolding, not serialized native layouts. TypedProgram can validate individual semantics while the production VM is extended; it will not own production actor state or execute side by side with Frame for one object.

This chooses extension over replacing the engine or installing the isolated VM as a parallel game runtime. Reuse reduces duplicated ownership and enables HP1/HP2 regression checks. It does not establish that the existing UE1 renderer, physics, state machine or asset loaders already support PoA.

## Existing source and responsibility

Paths below are relative to native-runtime/. Existing APIs are source observations; the PoA adapters described here are proposed work.

| Area | Existing anchor | Production decision / required adaptation |
| --- | --- | --- |
| Package identities | src/engine/SurrealEngine/Package/PackageManager.h; Package.h | One package cache and import/export identity graph. Add a bounded PoA reader selected by format profile before UE1 serialization assumptions. |
| Object construction | Package::NewObject; Package::GetUObject | Allocate through the existing factory and export cache. Register verified PoA native classes; no parallel allocation by ReceiverProgram. |
| Class/default/property layout | Packages/Core/UClass.h; UObject.h; PropertyDataBlock | Decode UE2 class/property metadata into verified runtime schemas, then initialize the existing property block. Validate Boolean packing and native sizes; never copy synthetic offsets/defaults. |
| Script calls and state | VM/Frame.h; ExpressionEvaluator.h | Extend the production bytecode decoder/evaluator with profile-specific semantics and verified argument marshaling. The existing Frame remains the call/state owner. Validate latent/state behavior separately. |
| Lifetime | GC/GC.h; GCRoot | Keep active calls and deferred fixups rooted through the existing collector. Actual actor Destroy/pending-kill/world removal must use the production lifecycle, not host registry retirement. |
| Native functions | Native registration and src/knowwonder/ | Reuse a body only after signature and behavior verification. Profile-specific bindings prevent PoA indices from replacing HP1/HP2 registrations. Unsupported natives fail with useful identities, never successful dummy results. |
| World and rendering | Existing Engine loop and actor/resource/render systems | Reuse the scheduling and rendering infrastructure. Add verified PoA map, mesh, material, animation and effect adapters as their tasks require. No separate PoA world clock or renderer by default. |
| Android | android/CMakeLists.txt; jni/android_main.cpp | Keep the current SDL/JNI entry point and application identity. Add PoA integration only behind an explicit experimental profile; launcher stays disabled until live acceptance. |

## Bridge contract

1. **Profile selection:** identify supported PoA data explicitly, preserving the current HP1/HP2 path. Do not equate IsHarryPotter2 or IsKnowWonder's existing UE1 behavior with PoA. Record package version/licensee combinations from the user's files; a version 129 header alone is insufficient game identification.
2. **Package-local references:** resolve each signed serialized import/export reference through its owning production package. Package names, outer chains, classes and object names form identities; global integer IDs from fixtures do not. None remains distinct from unresolved or malformed references.
3. **Metadata handoff:** use the bounded readers as independently tested parsing logic. Translate their verified records into production UClass/UProperty/UFunction/UState structures only after each layout is supported. Reject unsupported records with package/export/offset context.
4. **Object/default handoff:** reserve export identities before resolving cyclic dependencies; construct once through the factory; resolve fixups and apply verified inherited defaults before initialization. Do not report a loaded object while its required schema or native payload remains unsupported.
5. **VM handoff:** map decoded bytecode references to production metadata and validated instruction boundaries. Dispatch through Frame's object, function, state and property mechanisms. Preserve caller Self versus explicit context, out/optional argument behavior and exception cleanup; synthetic CallEnvironment callbacks are specifications to verify, not live bindings already present.
6. **Native/world handoff:** native handlers receive production objects and validated property values. They call the existing world/resource services through profile-specific adapters where needed. Save/load and travel serialize this same object graph.
7. **Ownership:** no long-lived unrooted UObject pointers in a new compatibility cache. Package/fixup/call owners must retain GC roots until references are safely owned by the engine. Teardown removes world membership and roots in the established lifecycle; an isolated retired ID proves none of those operations.
8. **Failure policy:** fail closed on unsupported layouts/opcodes/native signatures, with structured file diagnostics rather than an on-screen debug console. Never skip a required initialization or advance a release gate after a substitute success.

## Implementation order and acceptance

| Stage | Related tasks | Evidence required |
| --- | --- | --- |
| Explicit profile and package reader | DATA, PKG-01–PKG-06 | Supported original summaries/tables/metadata loaded through the production path; malformed and missing inputs rejected; HP1/HP2 unchanged. |
| Real object/default graph | PKG-07–PKG-13, OBJ | Real production objects, cyclic imports, inherited defaults, native layouts and lifecycle verified. This is G1 work. |
| VM/native/state slice | VM, STATE, NATIVE | Original startup functions execute through production Frame with real objects and verified handlers; no fixture-only receiver graph. |
| First real scene | WORLD, RENDER, ANIM, AUDIO, GUI, ANDROID | Startup dependencies load and Android presents a real scene/menu. G2 requires device evidence, not host parser output. |
| Interactive and persistent play | Remaining workstreams | G3–G6 evidence as specified by the master roadmap. |

No new runtime integration, APK, first frame or gameplay support is claimed by this decision. BASE-05/BASE-06 cover choosing and defining this architecture; implementation remains under the linked production tasks. Revisit this decision explicitly if original data proves an extension untenable, retaining the evidence and migration impact in the discovery ledger.
