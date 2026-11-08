# 8 November — EU animation reader and sampler

- [x] Register a PoA-only MeshAnimation native resource and add it to the engine build.
- [x] Decode all 140 EU MeshAnimation resources and 3,464 sequences to the exact payload end.
- [x] Sample 312,585 bone/sequence combinations with quaternion interpolation, explicit key times, looping and clamping.
- [x] Handle 298 zero quaternion keys as identity; retain two all-NaN position keys as unavailable translation channels.
- [x] Reject unsupported compressed flex tracks, malformed counts, invalid timings and truncated resources without replacing prior data.
- [x] Pass 126 compatibility tests and 29 Android packaging tests locally.
- [ ] Pass the new animation ARM64 build.
- [ ] Connect skeletal geometry, skinning, playback state, blending and notify dispatch.

Evidence: HP3-ANIMATION-EVIDENCE.json. Corpus coverage uses production reader/sampler and native Load body with bounded stream/object adapters. No device gameplay claim. All-NaN translation fallback is a defensive policy, not verified retail playback equivalence. Published v0.4.14 remains unchanged.

# 8 November — remaining Core declarations

- [x] Implement both quaternion/rotator handlers using original EU reference behavior and a documented unit-quaternion correction.
- [x] Implement string-array load/save and exact save-slot file deletion.
- [x] Implement reflected script-object Copy for the shipped scripted-action call.
- [x] Replace PoA Commandlet.Main's success-zero placeholder with script dispatch and explicit unsupported-base rejection.
- [x] Validate 1,331 isolated original math inputs and production-body/adapter/identity/file regressions.
- [x] Run 124 compatibility tests with original inputs and 29 Android packaging tests.
- [x] Correct the production header include path; the corrective ARM64 checkpoint passed.
- [x] Pass ARM64 compilation for the complete Core follow-up (Actions 42485673319).
- [ ] Complete remaining Engine/game APIs, native-resource copy variants, skeletal/terrain/rendering support and phone acceptance.

See HP3-CORE-DECLARATIONS-COMPLETION.json. All eleven entries from the initial list now have concrete scoped behavior; this is not a claim that the full UE2 port is complete.

# 8 November — original EU native tracing follow-up

- [x] Resolve the original EU Core.dll export entries and inspect four native implementations.
- [x] Implement exact ClockwiseFrom_IntInt half-turn/equality behavior and NeedSAPFont language selection.
- [x] Implement SetUTracing/IsUTracing with a working call-log hook and native-call filtering.
- [x] Test handler bodies and production trace callback preservation; 119 compatibility tests now pass with original EU inputs.
- [x] Complete the six remaining Core declarations (see newer completion section); other Engine/game APIs remain open.
- [ ] Complete skeletal animation, terrain, richer rendering and device acceptance.

See HP3-EU-NATIVE-TRACE.json and HP3-FOUR-AREA-IMPLEMENTATION.md. No claim that all four broad areas are complete.

# 8 November — four-area implementation pass

- [x] Add a functional PoA-only StaticMesh reader; test all 797 EU static mesh exports.
- [x] Connect static mesh bounds/transforms, initial diffuse rendering and triangle sweep/overlap collision.
- [x] Implement repeated-key dynamic-array config/globalconfig loading and saving.
- [x] Fix INI list resizing without deleting comments or unrelated lines.
- [x] Audit unresolved Core declarations and trace their static script call sites.
- [x] Run 118 compatibility tests with original EU inputs and 29 Android packaging tests.
- [x] Gate ARM64 CI on host/original-data regression tests.
- [ ] Implement UE2 skeletal animation, terrain and remaining native/game behavior.
- [ ] Complete material/lighting/collision-hull support and validate performance.
- [ ] Pass the ARM64 build for this source.
- [ ] Execute phone startup, menus, controls, switching, travel and save/load acceptance.

See HP3-FOUR-AREA-IMPLEMENTATION.md for evidence and limits. All four broad workstreams remain open; published v0.4.14 has not changed.

# 8 November — legacy String and name text round trips

- [x] Make legacy String properties use the same escaping/unquoting as Str properties.
- [x] Quote names containing list delimiters/whitespace; preserve ordinary identifier output.
- [x] Test empty strings/names, quoted punctuation, backslashes, spaces, ordinary identifiers and raw legacy input with production method bodies under ASan/UBSan.
- [x] Run host suite (112 pass, one skip) and native syntax checks.
- [ ] Verify rebuilt ARM64 and device behavior.

Remaining work is broader than config: repeated-key array INIs; map/object/delegate text; missing or conditional native calls; UE2 static/skeletal meshes, animation and terrain; rendering/collision; phone startup, controls, character switching and save/load acceptance. These are separate open areas, not an assurance that one further fix makes HP3 playable. Source changes are not in published v0.4.14.

# 8 November — reflected dynamic-array follow-up

- [x] Import/export supported dynamic-array text, including nested arrays and quoted strings.
- [x] Permit supported array members in PoA struct config export.
- [x] Preserve existing arrays on failed text or binary replacement.
- [x] Validate serialized counts, fixed indexes and exact tagged payload sizes.
- [x] Execute compiled sanitizer regression harness and host suite (112 pass, one skip).
- [ ] Validate this batch in ARM64 and on the phone.
- [ ] Implement repeated-key top-level dynamic-array INI handling and map/object/delegate text support.
- [ ] Complete UE2 native assets/rendering and demonstrate HP3 startup/gameplay.

See HP3-ARRAY-TEXT-COMPATIBILITY.md. Source additions are not in published v0.4.14.

# 8 November — structured property text and config follow-up

- [x] Parse nested struct values and quoted comma/parenthesis literals without truncation.
- [x] Round-trip escaped strings and fixed-array struct members.
- [x] Preserve omitted struct defaults during partial PoA config overrides.
- [x] Save supported PoA struct config values; preserve INI entries for unsupported member types.
- [x] Parse 44 original EU INI struct-text records and execute production round-trip/rejection harnesses.
- [ ] Validate this source in a rebuilt ARM64 runtime and on the phone.
- [ ] Implement dynamic-array/map config text and complete UE2 native asset/rendering support.

See HP3-STRUCT-TEXT-COMPATIBILITY.md for scope and validation limits. This source follow-up is not in published 0.4.14 yet.

# 8 November — wider UE2 Core API extension (source follow-up)

- [x] Verify EU signatures for QuatInvert, QuatFindBetween, GetIniEntry and two interpolation-curve range functions.
- [x] Implement those five handlers and register them only for PoA.
- [x] Verify InterpCurve/InterpCurvePoint layouts against the bundled EU Core.u.
- [x] Read curve float fields through runtime reflection, including offset/type/bounds checks.
- [x] Test quaternion alignment edge cases, curve ranges and invalid metadata, and INI lookup with compiled production bodies.
- [ ] Implement and validate UE2 static/skeletal mesh and animation asset readers; existing StaticMesh loading is unfinished.
- [ ] Implement terrain primitives/sectors and related rendering/collision paths.
- [ ] Trace the remaining Core functions without registrations and verify conditional native bindings.
- [ ] Confirm ARM64 execution, phone startup, save/load and gameplay with the new source.

See HP3-WIDER-UE2-COMPATIBILITY.md and HP3-WIDER-UE2-CORE-EVIDENCE.json. Published 0.4.14 does not contain this source follow-up yet.

# 8 November — Core binding and auxiliary config audit (source follow-up)

- [x] Verify all seven EU Core declaration shapes and native indices against bundled scripts.
- [x] Trace static call sites across twelve script packages / 8,090 function records.
- [x] Fix const-quaternion conversion required by QuatProduct and QuatRotateVector.
- [x] Guard PoA axis-angle construction against a zero-length axis.
- [x] Handle missing auxiliary class INIs without replacing serialized defaults; retain strict primary config reads.
- [x] Execute all seven native bodies through production adapter/conversion code in isolated host harnesses.
- [x] Test production previous-state transitions and preserve UE1 behavior.
- [ ] Verify rebuilt ARM64 runtime on the phone past Hardware2Options.ini and subsequent startup stages.
- [ ] Complete mesh/terrain, rendering, save/load and gameplay acceptance separately.

See HP3-SEVEN-CORE-AUDIT.md and HP3-SEVEN-CORE-TRACE.json for evidence and limits. Published 0.4.14 does not contain this follow-up yet.

# 0.4.14 — EU KWGame range-default repair

- [x] Reproduce phone report in CompanionCombatManager export 222.
- [x] Correct tagged RangeVector and nested Range loading and saving.
- [x] Support bounded nested property writes and verify with the production package writer.
- [x] Verify exact EU default data, save/reload, legacy layouts and offset rejection.
- [x] Audit 1,685 class-default blocks and 90 tagged range values.
- [ ] Verify phone startup past this object; rendering, controls, saves and gameplay remain pending.

# HP:Trilogy Mobile — HP3 compatibility roadmap

Updated 7 November 2026 for 0.4.13. Android source port of the PC trilogy, using EU game versions.

This retains all 21 workstreams and their stable task IDs. Unchecked acceptance items remain open even when part of their implementation exists. Historical regional audit reports have been retired; they are not used as EU validation evidence.

## Current evidence

- EU archive and standalone disc files recovered, including the previously omitted `System/kwGame.u`. Its exact hash is recorded in the change notes and active EU profile.
- 110 EU packages inspect without parser errors. One unresolved root import remains: UnrealEd from QuidditchTerrain. Runtime necessity must be established.
- Production fixes include the 0.4.12 font reader, optional IpDrv initialization and seven EU Core native bindings. The ARM64 production engine compiles.
- 106 host compatibility tests, 29 Android packaging tests, 22 launcher checks and 13 EU configuration repair checks pass.
- Full three-game packaging and signing results are in the build-verification report. Phone execution, menus, first scene, save/reload and cross-game regression acceptance remain open.

## This batch

| IDs | Result | Remaining acceptance |
| --- | --- | --- |
| BASE-02, BASE-03 | Source frozen, original signing key retained, ARM64 engine rebuilt | Phone install/update and retained saves |
| DATA-01, DATA-02, DATA-08 | Standalone EU KWGame recovered; all 110 package tables inspected; missing-script preflight enforced | Device extraction and dynamic asset loading |
| PKG-03, PKG-04, PKG-08 | Earlier loader/font fixes retained; optional IpDrv lookup corrected | Real startup and default-object loading |
| NATIVE workstream | Seven Core bindings registered from EU signatures; KWConsole correctly classified as script | Remaining engine, GUI and game-specific native implementations |
| SHIP workstream | 0.4.13 local artifact preparation | HP3 launch acceptance before 0.5.0 release |

## Release gates

- [ ] Boot beyond language selection with usable menus and a rendered scene.
- [ ] Play the first interactive section with controls, camera, collision, scripts and animation.
- [ ] Save, close the app, relaunch and reload; verify map travel and settings persistence.
- [ ] Check HP1 and HP2 regressions, performance, complete payloads and update continuity.
- [ ] Publish 0.5.0 only after its declared gameplay target is demonstrated.

## BASE — Source baseline and integration design

- [ ] **BASE-01** Consolidate the latest cumulative compatibility overlay with the complete 0.4.1 source; preserve every earlier fix.
- [ ] **BASE-02** Create one canonical source checkpoint/branch and record the exact engine revision, patches and build dependencies.
- [ ] **BASE-03** Preserve Android package identity, permanent signing, HP1/HP2 saves and their existing game payloads.
- [ ] **BASE-04** Record the PC PoA edition/region, executable and package hashes; identify other editions separately.
- [ ] **BASE-05** Decide and document the production UE2 architecture: which existing engine systems are reused, extended or replaced.
- [ ] **BASE-06** Define the bridge between the isolated hp3 interpreter and production UObject/package/world/rendering systems; avoid two conflicting object models.
- [ ] **BASE-07** Choose a minimal real startup route and first interactive scene from verified original configuration/data.
- [ ] **BASE-08** Agree on milestone acceptance evidence and maintain this master checklist as the authoritative remaining scope.

## DATA — Complete game-data inventory and importer

- [ ] **DATA-01** Inventory the whole disc/install tree, both standalone files and compressed.zip contents; retain the standalone System/kwGame.u path.
- [ ] **DATA-02** Finish checking the four script packages outside the current eight-package corpus: Decorations, AmbientCreatures, Editor and Fire.
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

