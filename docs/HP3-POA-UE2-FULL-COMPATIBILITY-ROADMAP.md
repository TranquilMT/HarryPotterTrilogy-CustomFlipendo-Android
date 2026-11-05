# PoA PC → Android UE2 compatibility: full known scope

Version 14 · 5 November 2026 · Project: HP:Trilogy Mobile

## What this document means

This is the master backlog for making the **PC version of Prisoner of Azkaban run in the Android source port**. It covers the complete known engineering and release scope, not merely the next interpreter feature. Existing UE1/Android code may satisfy some work after adaptation and testing; an unchecked item does not automatically mean rewriting that subsystem from scratch.

It cannot promise that reverse engineering will reveal no further bugs or behaviors. Those discoveries belong in the explicit discovery ledger below. They must be reported as discoveries within this scope, not as a surprise after claiming the port is almost complete.

**Current result: PoA is not verified playable. GitHub v0.4.5 enables experimental HP3 selection; live startup, a rendered map and interactive phone play remain unverified. The 0.4.7 release supplies a signed three-game test APK; phone acceptance remains open.**

## Audited baseline

| Area | Current evidence | What is not established |
| --- | --- | --- |
| Disc/scripts | All 841 disc/archive file records inventoried; 24 package payloads now checked offline: 12 scripts, two maps, five texture and five static-mesh packages | Complete production loading of all required game assets |
| Script metadata | 1,685 classes, 546 states and 8,090 functions checked across all 12 scripts | Correct execution of those classes/states/functions in the game |
| Isolated runtime | Primitive frames/calls/properties/default reads, bounded struct paths, object contexts/constraints and host receiver retirement | Actual native engine object/world behavior |
| Latest checkpoint tests | 97 PoA Python regressions; 100 table/stream, 88 function-header/token and 312 scalar-cast bridge sanitizer cases pass. Earlier full isolated C++ suite and Android packaging results remain historical | A percentage of game completion or gameplay acceptance |
| Native execution | 11 explicitly checked Core primitive intrinsics in the isolated typed runtime | The complete Core/Engine/KWGame native interface |
| Production package loader | v0.4.5 admits versions 60–177; this batch fixes production FString/index decoding and package reference/path defects; checked main PoA scripts use version 129 | A live UE2 loader; raising the version gate alone is not a solution |
| Android integration | Existing HP1/HP2 application/build infrastructure is available | The isolated hp3 runtime connected to the live Android game path |
| Startup target / gameplay | Original configuration and two map tables select preamble → front end → New Game → HP3_Adv1Express; v0.4.5 permits experimental selection; PoA remains unverified/unplayable | Boot, menus, rendering, controls, saves or a full playthrough |

Source audit: `native-runtime/src/compat/ue2/*`, `native-runtime/tools/poa/*`, `native-runtime/android/CMakeLists.txt`, `native-runtime/src/engine/SurrealEngine/Package/Package.cpp`, and the latest `docs/POA-*-VERIFICATION.json` reports. Existing renderer/audio/input code is a reuse candidate, not proof of PoA compatibility.

## Rules for ticking boxes

- `[x]` means the stated item is complete **at the stated level**. An isolated proof is not production support.
- Tasks below describe compatibility/acceptance work **at their stated level**. Inventory, route selection and design can be verified offline; production runtime tasks require production evidence. Partial prototypes must not falsely tick whole systems.
- Each stable ID remains attached to its task. Split tasks with suffixes if needed; do not silently delete/redefine unfinished scope.
- Mark complete only with a source revision and suitable evidence: test/report, original-behavior comparison, live trace, map capture or phone acceptance.
- “Parse/decode”, “execute an isolated fragment”, “load in the live engine” and “play correctly on Android” are different milestones.
- No honest completion percentage or release date can be derived by counting these boxes.

## Current work and update policy

**Active source baseline: GitHub `v0.4.5`, commit `94c75369030e867e24d5aab8c050e6eac968f24b`. Active implementation: PKG-03/PKG-04/PKG-08 loader prerequisites, while BASE acceptance remains open.**

**Baseline workstream: 1 of 21 — BASE.** Source reconciliation and architecture design are complete at the source/design level; complete original-data identity and signing/build acceptance remain open; the startup test route is selected. Workstreams 2–21 remain open. The current DATA-04/05/08 batch supports the open BASE-04 identity work; baseline build acceptance remains incomplete.

Update this document and the project roadmap at each completed work batch, fix, new finding or scope change:

- Retain stable task IDs and the original acceptance requirement. Add subtasks when needed; do not silently replace unfinished work.
- Record each affected task as open, in progress, blocked, fixed pending verification, or complete. Only verified complete items receive `[x]`.
- For a fix, record the defect, changed source/artifact, verification result and any remaining acceptance gap. Reopen completed tasks if regression evidence invalidates them.
- Add discoveries to the ledger immediately, with evidence, affected task IDs and release gate. Separate additional scope from work already covered by an existing task.
- Keep both roadmaps consistent about the current baseline, active workstream, blockers and next batch. Preserve dated checkpoint history.
- A completed batch does not imply that it is the only work remaining. Release gates remain open until their own evidence is obtained.

| Task | Status | Evidence / remaining requirement |
| --- | --- | --- |
| BASE-01 | Complete — source consolidation | Canonical 0.4.2 changes merged with the cumulative PoA overlay. See `POA-BASELINE-CHECKPOINT.md`, source lock and verified recovery package. No APK acceptance implied. |
| BASE-02 | Complete — pinned source checkpoint | Baseline `poa-base-20261101`, cumulative `poa-startup-20261101`; canonical main `d367c77c745a776c7178ba7218a27b115c75f025`; engine `c41d349e7e23584dac863367ba661bf90ef1c4f3`, verified upstream hashes and all 11 patches round-tripped. Dependencies and complete source hashes recorded. |
| BASE-03 | In progress / build inputs blocked | Original key recovered privately; certificate matches the permanent fingerprint. Read-only preflight identifies SDK/NDK/compiler/build tools, previous signed APK and full payload folders still unprepared. Signed rebuild and device save/update acceptance remain open. |
| BASE-04 | In progress | Original executable/configuration and 24 package payloads have checked identities (12 scripts, two maps, ten additional assets). Remaining 86 package payloads and independent region identification remain open; see `POA-PACKAGE-INTEGRITY-VERIFICATION.json`. |
| BASE-05 | Complete — architecture decision | `POA-PRODUCTION-ARCHITECTURE.md`: extend the production engine through an explicit PoA profile. Implementation remains open. |
| BASE-06 | Complete — bridge definition | Same decision documents package/object/property/VM/native/GC ownership contracts; no parallel synthetic actor registry. Live integration remains open. |
| BASE-07 | Complete — target selection | `POA-STARTUP-ROUTE.md`: original LocalMap and FirstLevel select preamble/front end and HP3_Adv1Express as the first interactive acceptance target. Classes/maps verified from original data; no live execution implied. |
| BASE-08 | In progress / ongoing | Master scope, acceptance rules and synchronized updates established; milestone evidence reviews continue throughout implementation. |

## Release gates and order

| Gate | Required proof | Current state |
| --- | --- | --- |
| G0 — Recoverable integrated baseline | Canonical source, architecture and reproducible build with existing games preserved | Open |
| G1 — Live UE2 loading | Production loader constructs verified startup dependencies/defaults and real objects | Open |
| G2 — Boot and first frame | Live PoA startup reaches a rendered real scene/menu with usable diagnostics on Android | Open |
| G3 — First interactive section | Input, camera, collision, animation, scripts/natives/events and progression work together | Open |
| G4 — Persistent multi-map play | Menus/settings, save/load, travel, death/retry and app resume pass on the phone | Open |
| G5 — Declared compatibility target | Required story/optional routes, performance and cross-game regressions pass | Open |
| G6 — Release | Signed payload-verified APK, recoverable source, checksums and accurate published status | Open |

Workstreams can overlap, but the implementation plan should aim at a live vertical slice early. Do not keep extending the isolated interpreter indefinitely without addressing G0–G2 integration. A debug first-frame build can be useful before full gameplay; that does not complete G3–G6.

## BASE — Source baseline and integration design

- [x] **BASE-01** Consolidate the latest cumulative compatibility overlay with the complete 0.4.1 source; preserve every earlier fix. **Baseline clarification D-001:** also reconcile the newer canonical 0.4.2 HP1/HP2 changes before declaring this complete.
- [x] **BASE-02** Create one canonical source checkpoint/branch and record the exact engine revision, patches and build dependencies.
- [ ] **BASE-03** Preserve Android package identity, permanent signing, HP1/HP2 saves and their existing game payloads.
- [ ] **BASE-04** Record the PC PoA edition/region, executable and package hashes; identify other editions separately.
- [x] **BASE-05** Decide and document the production UE2 architecture: which existing engine systems are reused, extended or replaced.
- [x] **BASE-06** Define the bridge between the isolated hp3 interpreter and production UObject/package/world/rendering systems; avoid two conflicting object models.
- [x] **BASE-07** Choose a minimal real startup route and first interactive scene from verified original configuration/data.
- [ ] **BASE-08** Agree on milestone acceptance evidence and maintain this master checklist as the authoritative remaining scope.

## DATA — Complete game-data inventory and importer

- [x] **DATA-01** Inventory the whole disc/install tree, both standalone files and compressed.zip contents; retain the standalone System/kwGame.u path. **Verified file-list level:** 75 standalone + 766 archived records in `POA-FULL-DISC-INVENTORY.json`; payload extraction/hashes and installer behavior remain separate tasks.
- [x] **DATA-02** Finish checking the four script packages outside the current eight-package corpus: Decorations, AmbientCreatures, Editor and Fire. **Verified offline:** all 12 scripts now pass table/dispatch checks, 8,090 function decodes and function-schema checks; this does not complete production package loading.
- [ ] **DATA-03** Identify which packages are required at runtime; prove any editor-only exclusions rather than assuming all packages must execute.
- [ ] **DATA-04** Inventory all maps, textures/materials, static/skeletal meshes, animation, sounds/music, movies, GUI assets and localization files.
- [ ] **DATA-05** Resolve complete startup, map and dynamic-load dependency graphs, including assets not referenced by static imports.
- [ ] **DATA-06** Preserve case, path spelling, object names and package identities across Windows-origin data and Android storage.
- [ ] **DATA-07** Implement a PoA-aware installer/import route for supported disc/install/archive formats; reject unsupported inputs clearly.
- [ ] **DATA-08** Check file sizes/hashes/CRC where available; detect missing, truncated or mismatched assets before launch.
- [ ] **DATA-09** Preserve required original configuration, help/menu assets and language resources; validate repair/default generation for PoA.
- [ ] **DATA-10** Handle free space, extraction progress, cancellation, rollback, interruptions and repeated imports without losing valid data.
- [ ] **DATA-11** Test Android scoped-storage/document-provider access and copying into the runtime game directory.

## PKG — Production UE2 package and object serialization

- [ ] **PKG-01** Implement version-aware production loading for the PoA package versions actually used; do not simply raise the UE1 version gate.
- [ ] **PKG-02** Validate package summaries, flags, generations and any compression/bulk-data variants present in the game.
- [ ] **PKG-03** Load name/import/export tables in the production loader, preserving package-local references and cross-package identities.
- [ ] **PKG-04** Resolve imports, exports, outer chains, super references and circular dependencies with bounded failure reporting.
- [ ] **PKG-05** Deserialize the full Class, Struct, State, Function, Enum and Property layouts used by the game.
- [ ] **PKG-06** Load function/state serialized bytecode and tails correctly, including native indices, flags, labels and replication-related fields where serialized.
- [ ] **PKG-07** Implement original class-default-object loading and inheritance of serialized default values.
- [ ] **PKG-08** Implement tagged-property decoding against the declared schema, including arrays, structs, enums, strings, names and object/class references.
- [ ] **PKG-09** Validate Boolean storage/packing and native property layouts; do not infer offsets from the isolated scalar stores.
- [ ] **PKG-10** Handle object state frames and any version-dependent native object payloads required by maps/assets.
- [ ] **PKG-11** Preserve deferred references/fixups and perform object post-load/initialization in the correct order.
- [ ] **PKG-12** Provide malformed-data bounds, missing-dependency diagnostics and useful package/object identities in errors.
- [ ] **PKG-13** Validate representative real packages end to end through the live engine, not only offline inspectors.

## OBJ — Live object/class model and lifetime

- [ ] **OBJ-01** Allocate real runtime objects and register the native classes required by PoA.
- [ ] **OBJ-02** Apply serialized class defaults to actual instances before initialization.
- [ ] **OBJ-03** Connect real property storage to script access while preserving inheritance and object identity.
- [ ] **OBJ-04** Implement class objects, class references, class constraints, class/default contexts and necessary casts.
- [ ] **OBJ-05** Complete object-reference resolution and any object constants used by scripts.
- [ ] **OBJ-06** Implement actor/component creation, ownership, outer relationships and attachment references.
- [ ] **OBJ-07** Implement actual actor spawning and initialization events in verified order.
- [ ] **OBJ-08** Implement Destroy/pending-kill semantics, destruction events and removal from world/tick/collision/render lists.
- [ ] **OBJ-09** Implement safe reference cleanup/garbage collection required by the game; establish correct behavior for references to destroyed actors.
- [ ] **OBJ-10** Integrate save/load and level-travel lifetime rules, including persistent and dynamically spawned objects.
- [ ] **OBJ-11** Connect the host-only retirement tests to verified engine behavior without treating registry retirement as a replacement for Unreal destruction.
- [ ] **OBJ-12** Validate repeated spawn/destroy/travel cycles without leaks, stale pointers or unintended identity reuse.

## VM — Complete the script execution features PoA uses

- [ ] **VM-01** Generate a complete opcode/operand/native-call usage inventory from all required script packages and state bodies.
- [ ] **VM-02** Map every used opcode to supported, unsupported or not yet verified runtime behavior; track executed coverage separately from decoding.
- [ ] **VM-03** Bridge isolated scalar frames/calls into the production VM and real object/property model.
- [ ] **VM-04** Implement names and strings, including comparison/conversion/formatting operations used by the game.
- [ ] **VM-05** Implement vector, rotator, color and other required struct values and their operations.
- [ ] **VM-06** Implement local, temporary, passed and returned struct values; retain correct copy/reference semantics.
- [ ] **VM-07** Implement fixed arrays, dynamic arrays and required container operations with correct bounds behavior.
- [ ] **VM-08** Implement indexing and assignments through arrays/structs/object contexts, including nested lvalues.
- [ ] **VM-09** Implement out/reference parameters, relevant parameter flags and any lazy/short-circuit argument behavior used by the game.
- [ ] **VM-10** Complete casts/conversions, class-valued expressions, object constants and delegates if present in required scripts.
- [ ] **VM-11** Complete switch/case, labels/goto, iterators/foreach and other control-flow instructions used by PoA.
- [ ] **VM-12** Complete supported arithmetic/comparison/logical/string/object natives, including correct short-circuit evaluation and argument order.
- [ ] **VM-13** Implement null-context/accessed-none behavior and result handling from verified semantics rather than fabricating success.
- [ ] **VM-14** Implement class contexts, default writes and remaining Boolean/default/struct wrapper forms.
- [ ] **VM-15** Implement class-constrained object locals and any remaining signature/local-layout constraints.
- [ ] **VM-16** Confirm virtual/global/final/super dispatch, context restoration and caller/callee Self/argument scope against original behavior.
- [ ] **VM-17** Preserve execution/depth limits for diagnostics while allowing legitimate game scripts and resumable/latent execution.
- [ ] **VM-18** Validate whole original script functions and real event chains; extracted instruction fragments alone do not complete this workstream.

## STATE — States, events, latent actions and scheduling

- [ ] **STATE-01** Deserialize and bind complete State payloads, super-state links, masks/flags, labels and state-local functions.
- [ ] **STATE-02** Implement state selection and transition semantics, including initial/automatic states and required GotoState behavior.
- [ ] **STATE-03** Invoke BeginState/EndState and related notifications in verified order, including reentrant transitions.
- [ ] **STATE-04** Implement state labels, Goto/continuation entry and persistent state execution position.
- [ ] **STATE-05** Implement latent actions used by PoA, including required wait/sleep/animation/movement completion behavior.
- [ ] **STATE-06** Resume latent state execution on the correct tick/time base and cancel it correctly on state change/destruction.
- [ ] **STATE-07** Implement timers, event enable/disable/probe behavior and event dispatch through class/state inheritance.
- [ ] **STATE-08** Integrate per-frame Tick, paused ticking, time scaling and frame-rate-independent scheduling.
- [ ] **STATE-09** Serialize and restore the state/timer/latent information actually required by saves and level travel.
- [ ] **STATE-10** Test original state-driven actors and gameplay event chains, not only host-selected dispatch scopes.

## NATIVE — Original native functions and KnowWonder behavior

- [ ] **NATIVE-01** Inventory native functions/classes and registration indices/signatures from required packages and original DLLs.
- [ ] **NATIVE-02** Compare UE1/HP1/HP2 implementations against PoA requirements before reusing any handler.
- [ ] **NATIVE-03** Analyze Core, Engine and KWGame native behavior needed by the startup route; include other DLLs when dependencies demand it.
- [ ] **NATIVE-04** Build a native dependency/coverage report: implemented and verified, reused but unverified, unsupported or not required.
- [ ] **NATIVE-05** Implement the Core object/math/name/string/config/file operations actually reached by PoA.
- [ ] **NATIVE-06** Implement actor/world/spawn/destroy/timer/event native operations required by PoA.
- [ ] **NATIVE-07** Implement collision, trace, movement, controller/AI and navigation native operations required by PoA.
- [ ] **NATIVE-08** Implement required KnowWonder pawn, interpolation, scripted-sequence, persistence, beam/effect and other custom behavior.
- [ ] **NATIVE-09** Implement mesh/animation/material/canvas/GUI/audio natives required by original game scripts.
- [ ] **NATIVE-10** Confirm native parameter ABI, return/out handling, registration profile and class layout compatibility.
- [ ] **NATIVE-11** Keep missing native behavior explicit; do not substitute empty handlers that falsely advance the game.
- [ ] **NATIVE-12** Validate native implementations against original evidence and live reproducible game behavior.

## WORLD — Map loading, travel and world management

- [ ] **WORLD-01** Load the verified startup/entry maps and construct the world and level objects.
- [ ] **WORLD-02** Deserialize BSP/model/brush/zone/level data and actors in a valid production world.
- [ ] **WORLD-03** Load static-mesh actors, terrain and other level geometry actually used by the game.
- [ ] **WORLD-04** Build actor lists, controllers, player start selection and game/console/controller objects.
- [ ] **WORLD-05** Raise pre-begin/begin/post-begin and required initialization events in the correct order.
- [ ] **WORLD-06** Implement map switching/travel and preserve the game state that should cross maps.
- [ ] **WORLD-07** Restore dynamically spawned items/actors according to PoA persistence rules.
- [ ] **WORLD-08** Handle unload/reload, reference fixups, memory release and failed travel recovery.
- [ ] **WORLD-09** Validate streaming/culling/visibility or other world features wherever the actual maps depend on them.
- [ ] **WORLD-10** Load every required map and dependency; record per-map failures and missing systems.

## PHYS — Collision, movement, physics and camera

- [ ] **PHYS-01** Implement real traces, collision queries and overlap/touch/untouch notifications.
- [ ] **PHYS-02** Implement collision for the BSP, static meshes, terrain, blocking volumes and dynamic actors used by PoA.
- [ ] **PHYS-03** Implement pawn walking/falling/jumping, gravity, steps/slopes, floor detection and movement modes used by the game.
- [ ] **PHYS-04** Implement controller/AI movement, navigation/path following and required movement completion events.
- [ ] **PHYS-05** Implement doors/platforms/movers and attachment/base-relative motion.
- [ ] **PHYS-06** Implement projectiles, spell traces, damage, impact and trigger interactions.
- [ ] **PHYS-07** Audit Karma-dependent behavior and implement or faithfully substitute the subset PoA actually needs; document the choice.
- [ ] **PHYS-08** Implement relevant swimming/flying/riding/special traversal modes where the game uses them.
- [ ] **PHYS-09** Implement player, companion and cutscene camera behavior, obstruction handling and transitions.
- [ ] **PHYS-10** Validate collision and physics under variable frame rates and on Android without tunneling/stuck actors.

## RENDER — Rendering, materials and lighting

- [ ] **RENDER-01** Connect loaded UE2 world/assets to the production render path; establish correct Android first-frame output.
- [ ] **RENDER-02** Load and render required BSP surfaces, static meshes and terrain.
- [ ] **RENDER-03** Decode required texture formats, mipmaps, palettes/compression and texture coordinate data.
- [ ] **RENDER-04** Implement the material/shader graph nodes and blend modes actually present in PoA assets.
- [ ] **RENDER-05** Implement material animation/modifiers such as panning, scaling, rotation and combiners where used.
- [ ] **RENDER-06** Implement required lightmaps, lighting, vertex colors and dynamic light behavior.
- [ ] **RENDER-07** Implement transparency, alpha testing, depth/culling and render-state behavior without missing/solid-colored surfaces.
- [ ] **RENDER-08** Implement required sky, fog, water, reflective or other special surfaces; verify each feature against actual asset use.
- [ ] **RENDER-09** Implement mesh/terrain visibility, culling and LOD behavior required for correctness and mobile performance.
- [ ] **RENDER-10** Implement GUI/canvas rendering, text/font drawing and viewport/scaling behavior.
- [ ] **RENDER-11** Compare original and ported scenes for missing geometry, bad UVs/materials, lighting and depth artifacts.
- [ ] **RENDER-12** Validate graphics context loss/recreation and required GPU capabilities on the target Android device.

## ANIM — Characters, meshes and animation

- [ ] **ANIM-01** Deserialize skeletal mesh, bone hierarchy, skin weights, materials and animation packages.
- [ ] **ANIM-02** Implement skinning and animation playback with correct coordinates, timing and interpolation.
- [ ] **ANIM-03** Implement channels/blending, transitions, loops and animation completion notifications used by gameplay.
- [ ] **ANIM-04** Implement animation notifies for sounds, effects, footsteps, interactions and other required events.
- [ ] **ANIM-05** Implement sockets/bone attachments for wands, items and effects.
- [ ] **ANIM-06** Handle the static/vertex mesh formats additionally used by PoA.
- [ ] **ANIM-07** Implement required facial/lip/secondary animation behavior where data and gameplay depend on it.
- [ ] **ANIM-08** Validate player, companion, creature/enemy and cutscene animation in motion rather than only static previews.

## FX — Particles and effects

- [ ] **FX-01** Inventory and deserialize emitters and required custom particle/effect classes.
- [ ] **FX-02** Implement particle spawning, updates, bounds, lifetimes, blending and texture animation.
- [ ] **FX-03** Implement spell, trail, beam, impact, environmental and weather effects actually used by PoA.
- [ ] **FX-04** Attach effects to correct bones/actors and propagate animation/camera transforms correctly.
- [ ] **FX-05** Integrate effect lights, sounds and event timing.
- [ ] **FX-06** Validate particle cleanup on destruction/travel and bound mobile overdraw/memory without breaking effects.

## AUDIO — Sound, music, dialogue and movies

- [ ] **AUDIO-01** Inventory required sound/music/movie formats and reuse or implement compatible decoders/playback paths.
- [ ] **AUDIO-02** Load and play sound assets through the production Android audio path.
- [ ] **AUDIO-03** Implement spatial audio, attenuation, looping and actor-attached sound behavior.
- [ ] **AUDIO-04** Implement music start/stop/transition, volume, pause/resume and script-controlled playback.
- [ ] **AUDIO-05** Implement dialogue playback, subtitle synchronization and required localization selection.
- [ ] **AUDIO-06** Implement the original movie/story/cutscene playback route and skip controls where the game requires it.
- [ ] **AUDIO-07** Handle audio focus, calls/app suspension, device route changes and activity resume.
- [ ] **AUDIO-08** Validate sound/movie resources release correctly on repeated scenes and map travel.

## GUI — Original menu, GUI, settings and localization

- [ ] **GUI-01** Boot the original PoA front end using its actual game/console/GUI controller classes.
- [ ] **GUI-02** Implement the required GUI/UWindow/native controls, layout, focus, input and callbacks.
- [ ] **GUI-03** Make New Game, Continue/Load, Options and Quit work through original game flow.
- [ ] **GUI-04** Implement display, sound, input and gameplay settings with persistent values and correct engine effects.
- [ ] **GUI-05** Connect pause/resume, game book/inventory/objective/save screens and other in-game interfaces used by PoA.
- [ ] **GUI-06** Render menu assets, fonts, Unicode text, tooltips and subtitles correctly.
- [ ] **GUI-07** Adapt viewport/UI scaling and focus to landscape phones and varying aspect ratios without clipping or black regions.
- [ ] **GUI-08** Prevent launcher settings and game settings from overwriting unrelated game configurations.
- [ ] **GUI-09** Test menus with touch, gamepad and Android back/app lifecycle behavior.

## GAME — PoA gameplay completeness

- [ ] **GAME-01** Inventory every required story route, hub, challenge, minigame and ending from actual maps/scripts.
- [ ] **GAME-02** Complete player spawning, input, camera, HUD and the original new-game sequence.
- [ ] **GAME-03** Implement Harry/Ron/Hermione control and companion/switching behavior required by the PC game.
- [ ] **GAME-04** Implement each spell, transformation, targeting/casting rule and associated puzzle interaction used in the game.
- [ ] **GAME-05** Implement collectibles, inventory, rewards, counters and persistent pickup behavior.
- [ ] **GAME-06** Implement enemy/creature AI, combat, damage, health, failure/death and retry/checkpoint flow.
- [ ] **GAME-07** Implement required interactions, switches, doors, moving objects, puzzle state and scripted sequences.
- [ ] **GAME-08** Implement progression/objectives/unlocks, challenge completion and story transitions.
- [ ] **GAME-09** Implement every required flying/riding/special-control segment and minigame identified in the inventory.
- [ ] **GAME-10** Implement required bosses/endgame sequences and completion flow.
- [ ] **GAME-11** Validate cutscenes, dialogue, animations and gameplay event timing together.
- [ ] **GAME-12** Complete a start-to-ending playthrough without developer skips; separately validate optional content in the agreed compatibility target.

## SAVE — Saves, persistence and recovery

- [ ] **SAVE-01** Identify the save formats and exact game state PoA persists.
- [ ] **SAVE-02** Serialize/restore required objects, properties, player/companion state, inventory and progression.
- [ ] **SAVE-03** Restore maps, dynamic actors, persistent pickups, timers/state continuations and references where required.
- [ ] **SAVE-04** Implement save-slot selection, overwrite, load, autosave/checkpoint behavior and original menu integration.
- [ ] **SAVE-05** Validate cold app restart, repeated save/load, map travel, death/retry and interrupted writes.
- [ ] **SAVE-06** Handle corrupt/incompatible saves with clear recovery; preserve valid existing HP1/HP2 data.
- [ ] **SAVE-07** Define version/migration behavior for PoA experimental saves and future compatibility builds.

## ANDROID — Production Android integration and controls

- [ ] **ANDROID-01** Add a real PoA runtime/profile selection path to the native Android build and launcher.
- [ ] **ANDROID-02** Wire package loading, the VM, native bindings, world creation and renderer into that live path.
- [ ] **ANDROID-03** Configure verified PoA startup map/classes, paths and subsystem replacements; validate configuration repair.
- [ ] **ANDROID-04** Build and link the required code for Android ARM64 with the existing toolchain and application identity.
- [ ] **ANDROID-05** Map touch controls to all required PoA actions, including context-sensitive actions and special-control segments.
- [ ] **ANDROID-06** Implement gamepad input, rebinding and navigation where included in the compatibility target.
- [ ] **ANDROID-07** Test multi-touch, camera movement, deadzones/sensitivity, orientation and UI/gameplay coordinate mapping.
- [ ] **ANDROID-08** Handle app background/resume, pause, graphics/audio recreation, process termination and Android back navigation.
- [ ] **ANDROID-09** Ensure input, settings and saves stay game-specific so HP1/HP2 behavior is preserved.
- [ ] **ANDROID-10** Keep PoA disabled or clearly experimental until the relevant live launch/playability gates pass.

## PERF — Mobile performance and robustness

- [ ] **PERF-01** Measure CPU/GPU frame time, memory, asset load times and storage footprint on the target phone.
- [ ] **PERF-02** Profile script/native execution, world ticks, collision, animation, rendering and audio to find actual bottlenecks.
- [ ] **PERF-03** Bound asset caches, temporary allocations, actor/effect growth and level-travel memory.
- [ ] **PERF-04** Optimize draw calls, batching, LOD/culling, skinning and effects based on measurements.
- [ ] **PERF-05** Provide tested quality/performance settings appropriate to the target hardware.
- [ ] **PERF-06** Validate thermal throttling, long sessions, repeated map changes and low-memory behavior.
- [ ] **PERF-07** Test file corruption, missing dependencies, invalid bytecode/native calls and recovery without silent false success.
- [ ] **PERF-08** Produce exportable diagnostics/crash reports with useful package/map/function identities; keep on-screen debug consoles out of builds.

## QA — Compatibility evidence and acceptance

- [ ] **QA-01** Maintain separate reports for decoded metadata, isolated execution, native coverage, live map loads and completed gameplay.
- [ ] **QA-02** Run the existing Python/sanitizer regressions and add focused tests for new behavior.
- [ ] **QA-03** Add tests of whole original functions and original event/state chains with independently expected results.
- [ ] **QA-04** Validate original native behavior with binary evidence and/or reproducible original-game comparisons.
- [ ] **QA-05** Compare representative original/ported screenshots, audio timing, movement and gameplay outcomes.
- [ ] **QA-06** Run every required map and route; keep an explicit map/feature compatibility matrix.
- [ ] **QA-07** Test on the actual OnePlus CPH2663 target; record Android/build/GPU details and logs.
- [ ] **QA-08** Complete cold boot, menu/new game, interactive gameplay, save/load, travel and resume acceptance on the phone.
- [ ] **QA-09** Run complete-story and agreed optional-content acceptance tests; record known limitations honestly.
- [ ] **QA-10** Regression-test HP1/HP2 launch, menus, settings, controls, saves and optional HD installs after shared engine changes.
- [ ] **QA-11** Do not convert instruction/test counts into a percentage of game completion.

## SHIP — Packaging and release

- [ ] **SHIP-01** Produce a reproducible signed Android ARM64 build with correct version code/name and permanent certificate.
- [ ] **SHIP-02** Verify package contents and asset manifests; distinguish full game-payload builds from engine-only builds.
- [ ] **SHIP-03** Preserve the agreed HP1/HP2 payloads and keep optional HD packs separate unless the user changes that policy.
- [ ] **SHIP-04** Confirm the exact PoA data-delivery/import policy and required dependencies before labeling a PoA-inclusive APK.
- [ ] **SHIP-05** Test installation/update over the previous app without uninstalling or losing saves.
- [ ] **SHIP-06** Provide complete recoverable source, patches/build instructions, APK and SHA-256 checksums for a release.
- [ ] **SHIP-07** Publish accurate per-game README/changelog/known-issues and the tested compatibility matrix.
- [ ] **SHIP-08** Upload source/release artifacts to the canonical project repository when authorized; verify downloads and release metadata.
- [ ] **SHIP-09** Only label PoA playable/latest-ready after its stated acceptance gate passes; experimental builds must name their limitations.

## DISC — Explicit discoveries and scope control

- [ ] **DISC-01** Keep a discovery ledger for newly found formats, opcodes, native functions, map dependencies and behavioral differences.
- [ ] **DISC-02** Link each discovery to an existing workstream ID and identify the affected release gate.
- [ ] **DISC-03** If a genuinely new subsystem is required, add it here explicitly with evidence and explain the scope change.
- [ ] **DISC-04** Never describe the next small batch as the entire remaining project.
- [ ] **DISC-05** Do not mark an item complete from a synthetic test when its acceptance requires production/phone evidence.
- [ ] **DISC-06** Review remaining scope and milestone evidence at each checkpoint; retain unresolved issues until independently verified.

## Discovery ledger

| Discovery ID | Evidence/reproduction | Existing task/gate affected | Required action | Status |
| --- | --- | --- | --- | --- |
| D-001 | Canonical GitHub main `d367c77c745a776c7178ba7218a27b115c75f025` documents 0.4.2, version code 43, and an HP2 menu clipping correction; local extracted source began at 0.4.1. Repository documentation is not independent device acceptance. | BASE-01, BASE-02, BASE-03 / G0 | Reconcile newer code, patch and payload changes before consolidating the PoA overlay; retain HP1/HP2 fixes. This is a baseline correction within existing scope. | Source reconciliation verified in poa-base-20261101; device acceptance tracked separately under BASE-03/G0 |
| D-002 | Canonical 0.4.2 Canvas header contained the menu clip correction, but `0003-cursor-focus.patch` still saved the older implementation. | BASE-01, BASE-02, BASE-03 / G0 | Refresh the routed patch and verify upstream reversal plus exact reapplication. Existing source-recovery scope; no new subsystem. | Fixed and host-verified: all 11 patches round-trip; Canvas regression passes. Phone acceptance remains open. |
| D-003 | `build_profile.py` hard-coded the startup/game/console/GUI selections and did not bind saved inspection results to current package hashes. | BASE-04, BASE-07, DATA-09 / G0–G2 | Read original configuration, verify configured classes/maps, reject stale audits and separate unknown executable identities. | Fixed and host-verified: twelve new tests, 66 total Python tests; original-data route matches. No live startup claim. |
| D-004 | The complete source ZIP has no Git history, but build_linux.py unconditionally called git rev-parse HEAD for build metadata. | BASE-02, BASE-03 / G0 | Use a verified complete source-lock identity for archive builds; reject modified source and retain Git revision/dirty-state reporting for checkouts. | Fixed and host-verified with five preflight tests; final source-lock identity check passes. This does not supply missing build tools or produce an APK. |
| D-005 | Ten checked texture/mesh payloads include versions 120, 123, 128 and 129 and material/resource export kinds; their imports expand the known static dependency frontier. | BASE-04, DATA-04/05/08, PKG-02, RENDER / G1–G2 | Verify remaining payloads, inspect version-aware asset layouts and continue transitive dependency discovery. This confirms existing scope, not a new subsystem. | Ten assets CRC-checked, hashed and table-inspected; 24/110 packages currently verified. Static/dynamic closure and native payload deserialization remain open. |

## Checkpoint history

### 2026-11-01 — Payload integrity and transitive dependencies

- Local source remains 0.4.3 / code 44; checkpoint `tq-0.4.3-payload-integrity`. Complete standalone source ZIP/checksum and both roadmaps refreshed.
- Added read-only extracted-package checks for expected paths/sizes/CRCs, pinned hashes where available, table validity, unanchored standalone files, unexpected packages and transitive imports with cyclic graphs.
- Original data advanced from 14 to 24 valid package payloads: ten required texture/mesh assets extracted from the supplied disc and CRC-checked, hashed and table-inspected. 86 inventoried package payloads remain unextracted; no invalid/unanchored or unexpected packages in the checked set.
- Current static frontier: preamble chain reaches 16 unextracted packages, first story chain reaches 21. These are incomplete frontiers that can expand as further imports are inspected; dynamic/config/native/movie dependencies remain separate. The checker correctly returns nonzero for this incomplete data set.
- Evidence: `POA-PAYLOAD-INTEGRITY-AND-DEPENDENCIES.md`, `POA-PACKAGE-INTEGRITY-VERIFICATION.json` and `POA-ADDITIONAL-ASSET-IDENTITIES.json`. Original asset bytes remain outside the source/archive.
- Host checks: 83 PoA Python tests pass, including ten new integrity/dependency regressions. Android packaging/preflight checks remain 18 passing; C++ unchanged and prior sanitizer reports remain historical.
- No additional full checklist item completed. BASE-03/04, ongoing BASE-08, DATA-04/05/08 and G0–G6 remain open. No APK, live loader or gameplay result; PoA remains disabled.
- Next bounded work: extract/inspect required dependency payloads and adapt their verified layouts for production loading while retaining the unresolved build/acceptance inputs. This is not a claim that only those payloads remain.

### 2026-11-01 — Full file-list inventory and signing recovery

- Local source remains 0.4.3 / code 44; checkpoint `tq-0.4.3-inventory`. Current complete standalone source archive and checksum refreshed.
- Completed: DATA-01 full file-list inventory, 75 standalone + 766 archived records, including all 110 Unreal package files. Source metadata is original directory/ZIP central records; most payloads remain unavailable in the sparse view.
- Partial: DATA-04 all package files categorized, but embedded asset exports still pending; BASE-04 still needs extraction/identity checks for the remaining 96 Unreal packages and independent region evidence. No declared CRC is mislabeled as a verified hash.
- BASE-03 advanced: original private signing backup recovered, public certificate matches the permanent fingerprint exactly; private key/password excluded from source. Missing build/release inputs remain explicit blockers.
- Fix: D-004 standalone archive builds now use a validated checkpoint source identity rather than requiring Git history. Read-only preflight reports all missing inputs and verifies supplied signing identity.
- Host evidence: 73 PoA Python tests and 18 packaging/preflight tests pass. C++ unchanged; prior sanitizer evidence remains historical. Inventory provenance and signing/build status are in `POA-INVENTORY-AND-BUILD-INPUTS.md` and related JSON reports.
- G0–G6 remain open. No APK, signed update, phone acceptance or live PoA evidence.
- Next bounded work: recover/provision remaining build inputs and inspect required original package payloads/dependencies before advancing live loader integration. The complete 21-workstream backlog remains authoritative.

### 2026-11-01 — Standalone local 0.4.3 source

- Local source version: 0.4.3 / code 44, package `com.tranquil.hp1native`; canonical upstream/source baseline remains the recorded 0.4.2 commit.
- Complete standalone source archive includes all current source, vendored engine/SDL, patches, PoA tools/tests, evidence and both roadmaps. It does not require an older source ZIP or recovery package for extraction.
- Source lock refreshed for `tq-0.4.3-local-source`; each ZIP member verified against the locked bytes and archive integrity checked.
- No additional compatibility task is completed by versioning/packaging. Existing completed task IDs remain unchanged.
- No new APK, signing/payload acceptance, phone result or live PoA evidence. BASE-03/BASE-04/ongoing BASE-08 and G0–G6 remain open.


### 2026-11-01 — Original identity and startup selection

- Cumulative source checkpoint: `poa-startup-20261101`; baseline checkpoint 001 remains retained. New evidence: `POA-STARTUP-ROUTE.md` and `POA-STARTUP-IDENTITY-VERIFICATION.json`.
- Completed: BASE-07 route/first-interactive-target selection; DATA-02 supporting offline inspection of the four remaining scripts. Workstream 1 remains active; this supporting DATA inspection does not complete workstream 2.
- Partial: BASE-04 executable/configuration, 12 scripts and two maps hashed; complete asset hash inventory and independent region identification remain open. The supplied image name identifies North American, but volume label HPPoA alone does not prove it.
- Fix: D-003 profile generation now follows actual config and verifies class/map existence and audit hashes. Unknown executable/script sets get distinct disabled profile IDs.
- Verified: original executable matches retained SHA-256; archive CRCs match extracted executable/config/maps and 11 archived scripts; standalone kwGame matches its retained disc hash. Tables cover 1,685 classes/546 states; 8,090 functions decode and have schemas, with zero reported errors; 499 native argument orders checked. Native ABI/behavior is not established.
- Host checks: 66 Python tests pass, including twelve startup/profile regressions. C++ unchanged; previous checkpoint sanitizer results are retained rather than claimed as a new run.
- Remaining: BASE-03 signed build/payload/save/update/device continuity, BASE-04 complete original-data identity, BASE-08 ongoing evidence reviews. G0–G6 remain open; no new APK or live PoA result.
- Next bounded batch: complete original-data/package inventory and build/signing input recovery; then continue production loader integration. This is not the entire remaining scope.

### 2026-11-01 — Consolidated source and architecture checkpoint

- Source checkpoint: `poa-base-20261101`, reconstructed from the retained complete base archive and checkpoint recovery package; exact file hashes are in `POA-SOURCE-LOCK.json`.
- Tasks completed at their stated level: BASE-01 source consolidation; BASE-02 pinned recoverable source; BASE-05 architecture choice; BASE-06 bridge design. Evidence: `POA-BASELINE-CHECKPOINT.md`, `POA-BASELINE-VERIFICATION.json`, `POA-PRODUCTION-ARCHITECTURE.md` and source-lock recovery verification.
- Fix: D-002 saved Canvas patch now preserves canonical 0.4.2 behavior after patch reset/reapply. D-001 source reconciliation resolved.
- Verification: 54 PoA Python tests, 13 Android packaging Python tests, 12 isolated PoA ASan/UBSan suites and the Canvas clipping regression pass. Leak detection is unavailable under ptrace and was disabled; no leak-check claim. All 11 engine patches reversed to upstream hashes and reapplied byte-for-byte.
- Remaining BASE acceptance: BASE-03 signing/build/payload/device continuity; BASE-04 original-data identity; BASE-07 verified startup and first scene; BASE-08 ongoing evidence reviews.
- Live PoA Android evidence: none. No new APK. v0.4.5 permits experimental selection; PoA remains unverified/unplayable; G0–G6 remain open.
- Next bounded batch: original PoA configuration/identity audit for BASE-04/BASE-07, alongside the remaining build/signing input audit. This does not complete the 21-workstream backlog.

### 2026-11-01 — Tracking policy and baseline correction

- Source reference: canonical main `d367c77c745a776c7178ba7218a27b115c75f025`; local tree is an extracted 0.4.1 archive plus cumulative isolated PoA compatibility work, not yet a reconciled canonical checkpoint.
- Completed production tasks: none in this update. No completion checkbox changed.
- Partial work: BASE-01/BASE-02 baseline audit; BASE-08 acceptance and update policy.
- Discovery: D-001, newer canonical 0.4.2 baseline. Exact code/payload reconciliation remains open.
- Fix status: the 0.4.2 HP2 clipping correction is reported in canonical release documentation; preservation in this local baseline and phone acceptance remain unverified.
- Live PoA Android evidence: none. v0.4.5 permits experimental selection; PoA remains unverified/unplayable.
- Release gate: G0 open; G1–G6 open.
- Next bounded batch: reconcile 0.4.2 source/patch changes, record dependency provenance and preserve the cumulative compatibility overlay. Remaining work is the full 21-workstream backlog.

## Checkpoint update template

- Source revision / artifact:
- Tasks changed from open to complete, with evidence:
- Partial work advanced but not complete:
- New discoveries and linked task IDs:
- Live Android evidence obtained:
- Current release gate:
- Next bounded work batch (not a claim that only this batch remains):

## Explicitly outside the required compatibility scope

HD remaster packs, new game modes, added content, modernized art, achievements, cosmetic redesigns and implementing every unused UE2/editor feature are not required merely to make PoA compatible. Any such expansion needs its own explicit scope. Conversely, a feature cannot be excluded as “unused” until the game's required data/routes confirm that.



## 4 November 2026 — v0.4.5 production package-loading checkpoint

- Base: exact GitHub tag v0.4.5 / 94c75369030e867e24d5aab8c050e6eac968f24b. Source version remains 0.4.5 until a verified APK release; no version-code or signing changes.
- PKG-03/04/08 in progress: production PackageStream and ObjectStream share bounded signed compact-index and ANSI/UTF-16LE string readers. Surrogate pairs decode to UTF-8; oversized, malformed and truncated data fail explicitly.
- PKG-04 fixes: reject export references at the end boundary; use one-based class/export lookup correctly; avoid signed overflow for INT_MIN imports; resolve nested groups in root-to-leaf order; detect cyclic/excessive outer chains; export-name lookup no longer overwrites its own table entry.
- Regression evidence: package helper suite covers valid UE1-style ANSI, Unicode, malformed inputs and reference chains. A separate test compiles the actual Package.cpp reference accessor/name methods with in-memory tables and sanitizers. This is a narrow production-code test, not a complete engine build.
- G0–G6 remain open. No original PoA assets, Android SDK/NDK or signing key were available in this workspace; no new APK, original-data launch or device test was performed. Existing HP1/HP2 saves, payloads and certificate handling are not altered by this source patch; game regressions still require device checks.
- Next bounded work: verify production summary/table layouts with retained PoA packages; adapt Class/Struct/State/Function/default layouts and native bindings, then exercise the real startup route. All 21 workstreams remain authoritative.


## 4 November 2026 — three-game launcher/input checkpoint

Source fixes and host regressions are detailed in `docs/LAUNCHER-CONTROLS-2026-11-04.md`.

- ANDROID-01/03/09, GUI-08 in progress: unique HP3 picker ID, separate folder preference/preflight/setup, three-state selection and explicit native folder recognition. HP3 no longer enters HP1 preparation or silently falls back to another title.
- ANDROID-05/06/07/08 in progress: PoA's original PC WASD, held cast, jump/flap, Use/possession-return, Map, Zoom, Walk and menu bindings; shared cancellation/lifecycle release, mouse chords and delayed-input invalidation. PC characters change with the story; no manual console-style swapping is required. Live native input acceptance remains open.
- PERF-03/05 in progress: stop background polling, repeated overlay allocations, hidden launcher save scans and hot collision logging; persist graphics/input choices per game and time-scale stick motion. No measured FPS/thermal improvement or tested quality default is claimed.
- Host checks: 22 Java path/profile/timing checks; actual Overlay method tests; native executable detection with ASan/UBSan; Android Java syntax parse; existing 94 PoA Python regressions pass. Full SDK type checking, native Android linking, original-data launch and phone testing remain unperformed.
- G0–G6 stay open. This is a source checkpoint on the 0.4.5 base, not a new APK or a completed PoA runtime. Follow the full 21-workstream scope and the device/build acceptance list in the checkpoint report.


## 4 November 2026 — production table/stream validation checkpoint

- PKG-02/03/04 remain in progress: validate signed summary/generation counts, minimum table ranges, name/import/export references and disjoint table/payload regions before production object construction. Check mixed import/export outer cycles with bounded, linear traversal.
- Production PackageStream now bounds reads/seeks/skips with a cached file size. OpenObjectStream rejects invalid/negative/out-of-file payloads before allocation and uses 64-bit allocation arithmetic. Zero-payload native exports retain their no-backing-file path.
- Evidence: 95 PoA Python regressions pass; the new test executes the actual production reader/table/open-stream methods on 100 synthetic UE1/PoA-format and malformed cases under ASan/UBSan. Launcher/control host checks still pass.
- Details and limitations: `docs/POA-PRODUCTION-TABLE-VALIDATION.md`. No complete engine compile, original-package runtime acceptance, APK or device play test was performed. UE2 class/default/native/resource layouts and live execution remain open; G0–G6 are not advanced by synthetic fixtures.


## 4 November 2026 — PoA function metadata and token-reader checkpoint

- PKG-05/06 in progress: production UFunction loading now reads the verified PoA v129 zero extension byte before Line/TextPos/script-memory-size; unknown versions/extensions fail. Other Struct/Class/State layouts are not inferred from this Function slice. The matching header write helper does not establish game-save compatibility.
- Production bytecode deserialization treats PoA 0x60 as extended-native syntax rather than UE1's overlapping conversion token. Unsupported PoA 0x39–0x5f opcodes fail explicitly before a UE1 expression can be built. Native registration/call ABI/evaluation remain open.
- Bound decoded bytecode appends and declared size. Serialize Unicode script code units independently of host wchar_t width, fixing multi-character UTF-16 copying on Android/Linux.
- Evidence: 96 Python regressions pass, including 75 new header/actual-token-reader ASan/UBSan cases compared to the isolated decoder. See `docs/POA-FUNCTION-SCRIPT-HEADER-CHECKPOINT.md` for stub boundaries and the remaining acceptance work.
- G0–G6 remain open; no full engine compile, original-data production startup, APK, first frame or phone gameplay test was performed. The complete 21-workstream scope remains authoritative.


## 5 November 2026 — scalar primitive-cast bridge

- PKG-06/VM remain in progress: production UStruct decoding now accepts the UE2 0x39 prefix for the 12 scalar byte/int/bool/float cast codes. The expression builder selects the corresponding existing conversion nodes without shortening script bytes or shifting offsets.
- Other cast codes and bare overlapping UE2 opcodes remain explicit failures. Legacy UE1 conversion decoding retains its original path. This reuses existing evaluator implementations; original PoA numeric parity is not yet established.
- Evidence: all 97 Python regressions pass; production token checks expand to 88 ASan/UBSan cases, and a new 312-case bridge/actual-expression-construction harness covers selections, nesting, offsets, invalid codes and legacy forms. Details and source pin: `docs/POA-SCALAR-CAST-CHECKPOINT.md`.
- Class/State/default/property/native/resource and other UE2 opcode work remains open. No complete engine compile, APK, original-data run or phone play test occurred; G0–G6 remain open.


## 5 November 2026 — full Android startup build audit

- Priority switched to the complete Android build/startup trace rather than another isolated decoder addition.
- Verified blockers: partial local source checkout; no configured SDK/NDK, cmake/ninja/adb; SDL build dependency not present; Google's SDK download endpoint timed out through the available proxy. These findings apply to scratch; a browser Codespace is now available (see continuation below).
- No complete Android compile, APK installation or actual PoA startup occurred. G0 is blocked on build inputs/access; G1–G6 retain their existing open runtime/device requirements. Host regression count remains 97 and is not promoted to build/launch evidence.
- Exact findings and next executable sequence: `docs/ANDROID-STARTUP-BUILD-BLOCKERS-2026-11-05.md`. Continue in the browser Codespace, prepare the Android toolchain, compile/package, then capture the actual phone startup trace.

## Browser build continuation

- A complete compatibility-branch checkout is running in GitHub Codespaces. Desktop Commander and a user-owned computer are not required.
- CMake 3.28/GCC 13.3/Java are available. Native build dependencies installed; full host engine configuration succeeds. Production SurrealCommon compilation started.
- This does not close Android ARM64, APK packaging or phone startup gates; no new APK or original-data launch is verified.

## 0.4.7 packaging acceptance — 5 November 2026

- [x] Recover the full source and merge retained compatibility changes onto the 0.4.5 base.
- [x] Extract original HP1/HP2 payloads and new EU PoA common/English assets; verify archive integrity.
- [x] Add matching PoA launcher tile and separate folder/ZIP import.
- [x] Remove unsupported KWGame.u prerequisite; retain actual v129 script validation.
- [x] Compile Android Java sources against API 36.
- [x] Complete ARM64 native build and signed three-game APK verification.
- [x] Publish APK, complete source and SHA-256 checksums as latest release: [v0.4.7](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.7).
- [ ] Phone: launch PoA to first runtime failure or rendered menu and retain report.
- [ ] Phone: verify movement, casting, character swapping, menu, save/load and level transitions.
- [ ] Measure frame time and memory before claiming optimisation gains.

All other incomplete compatibility gates above remain open. Bundling a game does not establish engine compatibility.

## 0.4.8 — first phone startup report

- [x] Phone confirmed 0.4.7 reaches the native engine and fails before a map loads: `Could not find package Core`, with no scanned paths.
- [x] Fix PoA configuration layout selection: repeated `Paths=` entries and separate user/default-user files regardless of its unset UE1 ancestry field.
- [x] Add production policy regressions covering PoA, HP1/HP2, old Unreal, Rune and KHG.
- [x] Add selected-game managed-folder inventory to diagnostic reports.
- [x] Build and publish the corrected signed three-game APK as [v0.4.8](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.8).
- [ ] Phone: confirm Core resolves and record the next startup result.

This fixes the reported configuration-selection cause; further UE2 compatibility gates remain open.

- [x] Add pre-launch settings for all three games and native PoA frame pacing/FOV controls.
- [x] Build and verify the 0.4.8 signed APK with all three payloads; 100 compatibility regressions and 28 release checks passed.
- [ ] Phone: confirm startup passes Core discovery and verify the new settings in play.
- [ ] Resolve remaining 431 static metadata relationships before claiming complete script corpus support.
