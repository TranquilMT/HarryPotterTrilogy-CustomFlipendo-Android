# HP:TPS Android — Development Roadmap & Tick-off Checklist

**Project:** [HarryPotterTrilogy-CustomFlipendo-Android](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android)  
**Upstream:** [Flipendo ROADMAP.md](https://github.com/kroplabeskidu/flipendo/blob/master/ROADMAP.md)  
**Local source version:** **0.4.3**, version code **44**; complete standalone source ZIP requested and assembled locally. No 0.4.3 APK or device acceptance is claimed.

**Canonical source baseline:** Android **0.4.2**, version code **43**, main `d367c77c745a776c7178ba7218a27b115c75f025`; its release report records a signed Both Games build. This PoA checkpoint reconciles that source; no new APK or physical-device acceptance is claimed.

**Baseline:** Android v0.3.3; upstream roadmap status dated 2026-10-27; checklist prepared 2026-10-31.  
**Rule:** `[x]` only after implementation **and** evidence-based verification. `[ ]` means open, including partially completed or unverified work. Do not confuse upstream progress with our Android port's progress.

**PoA integration checkpoint (2026-11-01):** canonical 0.4.2 source reconciled with the cumulative isolated PoA overlay; host checks pass. Checkpoint `poa-base-20261101` records source hashes, dependency pins and patch recovery. Follow the [master checklist](docs/HP3-POA-UE2-FULL-COMPATIBILITY-ROADMAP.md), workstream 1/21. No live PoA or phone acceptance is established.

## How to resume in Work mode

1. Read this file, the latest upstream `ROADMAP.md`, both repositories' current branches, and recent release/build history.
2. Compare upstream engine source with `native-runtime/`; record file-level changes and relevant commits. Do not blindly replace Android-specific code.
3. Work in small, testable batches; commit each coherent fix and update this checklist with commit SHA, build/test results, and remaining blockers.
4. Preserve HP1 working behaviour, game saves, Android package ID, and signing certificate. Never commit signing keys, credentials, or copyrighted game assets.
5. Run available tests; build an ARM64 APK at release milestones, verify checksum and install/launch on device. Do not claim game compatibility without playtesting.
6. Keep HD packs optional, downloadable separately from APK, and game-specific. Keep POA disabled until an actual UE2 runtime test passes.

## Current verified/reported baseline

- [x] GitHub repository exists, with Android `main` and upstream `master` accessible (checked 2026-10-31).
- [x] v0.3.3 release README identifies HP1 + HP2 in the APK, optional HD textures, and HP3 disabled (repository documentation; not an independent APK test).
- [x] HP1 user reports gameplay working on Android.
- [x] HP2 user reports gameplay playable with issues; complete playthrough not established.
- [x] HP1 combined optional pack metadata fix documented: 6 modules / 3,505 textures.
- [x] HP1 pack installer reported success after a freeze/restart (user report).
- [ ] Confirm HD textures actually override original textures during rendering.
- [ ] Investigate/fix full UI freeze at end of HD pack verification; user observed frozen controls.

## P0 — Source audit, reproducible builds, release integrity

- [ ] Fetch and compare current upstream source and roadmap with Android `native-runtime/` at file and commit level.
- [x] Record current Android build configuration, dependencies and native libraries in `native-runtime/android/verification/0.4.1-toolchain.json` and `0.4.1-build.json`. Upstream file-level comparison remains a separate open item.
- [ ] Create a compatibility matrix for HP1, HP2, HP3 (import, boot, menu, gameplay, audio, save/load, renderer, controls).
- [x] Establish and run the local Android ARM64 build command; document prerequisites and host regression commands in `docs/RELEASE-0.4.1.md`. Device smoke tests remain open.
- [x] Preserve application ID `com.tranquil.hp1native`; verified in the built 0.4.1 APK manifest.
- [ ] Verify existing signing certificate continuity and upgrade install (no private keys in repo). Certificate matches prior APK in 0.4.1; upgrade installation remains for the phone test.
- [ ] Add regression tests for HP1 launch, saves, Default.ini/DefUser.ini repair, and HD pack toggle.
- [ ] Track each fix in this checklist with source reference, commit, tests and phone result.

## P1 — HP2: Chamber of Secrets (upstream roadmap priority)

### HP2 main menu and front-end parity — Android release blocker
- [ ] **Implement/restore the Chamber of Secrets main menu** in the Android runtime (0.4.1 implementation built; phone navigation/rendering pending); do not mistake the launcher game selector for HP2's own front end.
- [ ] Compare the original HP2 PC `Game.exe` menu with upstream Flipendo and our Android fork: identify missing menu assets, scripts, input handlers, camera/background rendering and initialization.
- [ ] Show the correct HP2 title screen, background, menu graphics, fonts, cursor/highlight effects, transitions, music and button sounds using the user's imported game assets.
- [ ] Wire **New Game** to the correct HP2 starting map and initial save/profile flow; verify it works after a fresh install and with existing saves.
- [ ] Wire **Load Game** to list existing HP2 saves, handle empty/corrupt slots, and load without mixing HP1 and HP2 save directories.
- [ ] Implement **Options / Settings on the HP2 main menu**, with working navigation and persistence, matching HP1/Philosopher's Stone usability where applicable.
- [ ] Include video settings: resolution/render scale, aspect ratio, brightness, fullscreen/immersive mode, frame-rate cap and relevant graphics toggles; expose only options supported by the Android renderer.
- [ ] Include audio settings: master/music/effects/voice volumes, mute and subtitle toggles where supported; apply changes immediately or indicate restart requirements.
- [ ] Include input settings: touch sensitivity, invert look, button layout/size/opacity and gamepad mapping where supported; ensure touchscreen can operate every menu.
- [ ] Implement **Apply / Save / Cancel / Defaults** behaviour correctly, including validation, INI/config writes, and reload on next launch.
- [ ] Confirm **Back**, **Resume**, **Return to Main Menu** and **Quit to Launcher** work from gameplay and pause/book menus without freezing or corrupting saves.
- [ ] Ensure the HP2 main menu remains accessible after cutscenes, save/load, background/resume and Android activity recreation.
- [ ] Compare HP1 and HP2 settings screens side by side; share stable UI/settings infrastructure without overwriting game-specific options or configuration.
- [ ] Add automated smoke tests for menu entry points and config persistence, plus device test evidence (screenshots/logcat) for all menu buttons.
- [ ] Regression-check HP1's existing main menu, settings, save/load and controls after HP2 front-end changes.

### HP2 mirror/window texture bug — Android release blocker
- [ ] Reproduce the reported **missing/incorrect mirror texture** in HP2 on Android; record exact map, camera angle, expected original-PC appearance and screenshot.
- [ ] Inspect `PrivetDr` outside window `extwindow_PD` (surface **991**, `PolyFlags 0x08000000`) and verify whether it exhibits the same bug on Android.
- [ ] Compare upstream `PF_Mirrored` handling and HP2 `Render.dll` routines (`URender::OccludeBsp` / `URender::DrawFrame`) against Android native rendering.
- [ ] Determine whether the bug is a missing base texture, incorrect mirrored surface flags, reflection compositing, UVs, alpha/depth state, or a texture-override conflict.
- [ ] Ensure mirror/window base texture remains visible when appropriate and reflection rendering does not incorrectly hide the surface.
- [ ] Test with **HD textures off and on** to isolate the engine bug from optional texture pack overrides.
- [ ] Check other reflective surfaces/windows across HP2 maps for regressions; compare original-PC and Android screenshots.
- [ ] Add a focused rendering regression test or reproducible capture and confirm the fix on the target ARM64 phone.

### Full playthrough and gameplay verification
- [ ] Play all 43 maps; log each map's launch, exit, collision, cutscenes, objectives and save/load results.
- [ ] Verify all 10 duels and Quidditch sequences.
- [ ] Compare Android behaviour with original PC game for every significant deviation.
- [ ] Verify in-game book pages, folio, map and rollover sounds.
- [ ] Verify hub persistence and save/load through progression, not only at the opening.

### Sound, dialogue, animation and movers
- [ ] Confirm dialogue from `Sounds/AllDialog.USA_uax` plays correctly.
- [ ] Confirm moving actors' sounds loop while moving and stop correctly.
- [ ] Confirm Harry's sounds and dialogue are centred and `StopSound` fades out.
- [ ] Verify Dobby mouth movement in PrivetDr intro and that mouth closes afterward.
- [ ] Find and test an `MV_SpringByTime` mover.
- [ ] Test music, voices, lip-sync, walking, stair smoothing and tick order on Android.

### Known bugs from upstream roadmap
- [ ] Complete the detailed **HP2 mirror/window texture bug** checklist above; upstream reference: PrivetDr `extwindow_PD`, surface 991, `PolyFlags 0x08000000`; compare `Render.dll` `PF_Mirrored` handling in `URender::OccludeBsp` and `URender::DrawFrame`.
- [ ] Reproduce and fix Lumos wand beam persisting after cast; distinguish `Lumos_fly`, `Lumos_hit`, cursor and charging effects; confirm normal 30-second Lumos light expiry.
- [ ] Fix Hermione getting stuck on other NPCs during escort (upstream PR #12 context).
- [ ] Compare PrivetDr intro crane flyover camera rotation, brightness and sky/backdrop with original.
- [ ] Fix overlapping cutscene commands (`... hasnt finished yet`); compare Dobby `Introduce`, Harry `Look_Frantic`, and Adv1Willow skipping.
- [ ] Reproduce/guard `SKIP ERROR ... failed to advance after 16 loops` when fast-forwarding PrivetDr intro; add deterministic regression test.

### Engine analysis still open upstream
- [ ] Audit remaining HP2-specific changed engine ports for actual behaviour differences.
- [ ] Verify mover bounding-box collisions and mover flickering fix.
- [ ] Reverse/implement HP2 state-label lookup from script state/label tables where needed.
- [ ] Investigate and test HP2-only oval collision cylinders.
- [ ] Read upstream `docs/re/hp2/gameplay.md` and `docs/re/hp2/engine.md` for implementation evidence.

## P2 — HD texture installer and rendering (Android-specific)

- [ ] Reproduce 3,505-texture installer freeze and capture Android logcat / ANR trace.
- [ ] Move long extraction, checksum and copy operations off the Android UI thread if currently blocking.
- [ ] Report separate extraction, verification, installation and completion progress; update controls on success/failure.
- [ ] Make cancel/retry and interrupted-install recovery safe and idempotent.
- [ ] Confirm actual engine texture override paths and count loaded replacements, not just ZIP entries.
- [ ] Validate HP1 masked/translucent DXT1 alpha overrides against original masking (upstream roadmap).
- [ ] Test optional HP1 and HP2 packs separately; no cross-game installation or APK embedding.
- [ ] Check low-storage, corrupt ZIP, duplicate install and memory-pressure behaviour.

## P3 — HP1 optional extras (upstream roadmap; lower priority than HP2)

- [ ] Add per-mod on/off and settings under `[Flipendo.Mods]` in INI, preserving command-line compatibility.
- [ ] Add in-game FEBook Extras settings page where practical on Android.
- [ ] Investigate fixed 60 Hz logic independent of drawing: separate tick/render, interpolation for actors, camera, animation and particles; keep 60 FPS cap default until level-by-level validation.
- [ ] Add/verify FOV slider and controller support, adapted for Android touch/gamepad UI.
- [ ] Evaluate restored-content opt-in mod only after validating cut actors (`Hub2.h2barron`, `mirrortarget`, `InvisibilityCloak`, `FloatingSpellBook`); then optional unused dialogue/audio; exclude cut spells without level content.
- [ ] Defer optional Twitch chaos mod unless explicitly requested; if implemented, ensure effects are reversed before saving and can be disabled.

## P4 — Android usability and performance

- [ ] Verify touchscreen mapping, controller support, pause/menu access and rotation behaviour in both games.
- [ ] Profile CPU/GPU/memory on ARM64 Android device and address crashes or frame-time spikes.
- [ ] Check audio latency, focus loss, background/resume, low-memory recovery and storage permissions.
- [ ] Keep production builds free of debug overlays (Eruda/vConsole or equivalent).
- [ ] Add optional diagnostics: active HD overrides, memory usage, engine version, game profile and exportable crash report.

## P5 — HP3: Prisoner of Azkaban (separate UE2 compatibility track)

> **Full remaining scope:** [PoA full compatibility master backlog](docs/HP3-POA-UE2-FULL-COMPATIBILITY-ROADMAP.md). The abbreviated checkpoint history below records isolated proofs, not a complete list of remaining production work. Do not present its next few unchecked items as everything left.


**2026-11-01 progress checkpoint:** workstream **1/21 (BASE)** remains active. BASE-01/BASE-02 source consolidation and pinned recovery checkpoint are complete; BASE-05/BASE-06 architecture choice and bridge design are complete. See [checkpoint evidence](docs/POA-BASELINE-CHECKPOINT.md) and [production architecture](docs/POA-PRODUCTION-ARCHITECTURE.md). Canonical 0.4.2 HP2 menu behavior is preserved, including a corrected saved engine patch (D-002). Verification passes: 54 PoA Python tests, 13 packaging Python tests, Canvas regression, 12 isolated PoA sanitizer suites and all 11 engine patches round-tripped against the pinned upstream. BASE-03, BASE-04, BASE-07 and ongoing BASE-08 acceptance remain open. G0–G6 remain open; there is no new APK or live PoA Android evidence.

**Startup checkpoint (2026-11-01):** BASE-07 is complete as original startup/first-interactive-target selection: preamble → front end → New Game → HP3_Adv1Express. DATA-02's supporting offline audit is complete across all 12 scripts (1,685 classes, 546 states, 8,090 function decodes/schemas). See [route selection](docs/POA-STARTUP-ROUTE.md) and [identity evidence](docs/POA-STARTUP-IDENTITY-VERIFICATION.json). Fixed hard-coded startup settings and stale audit acceptance (D-003); 66 Python tests pass. BASE-04 remains in progress for complete asset identities and independent region identification. BASE-03 and ongoing BASE-08 remain open; G0–G6 remain open and PoA remains disabled/unplayable.

**Ongoing update rule:** update this roadmap and the master checklist after each work batch, fix or discovery. Record task IDs, source/artifact, verification and unresolved gaps. Keep partial work and fixes pending verification unchecked; reopen items when regressions invalidate their evidence. Add newly discovered work explicitly to the master's discovery ledger and retain its history. The next batch is not the total remaining scope.

Verified source checkpoint (31 October 2026; based on supplied 0.4.1 source):
- [x] Add isolated explicit virtual/global named-call bindings with nested argument evaluation.
- [x] Test scope separation, package-local name bindings, missing targets, argument rejection, recursion and shared execution limits under ASan/UBSan.
- [x] Add isolated metadata linker and test class/state inheritance, global state bypass and caller-local name resolution.
- [x] Extract Core/Engine class/state/function relationships from retail tables; validate ownership/imports and compare Python/C++ lookup across 305 scopes.
- [x] Validate HGame, GUI, GamePlay and UWindow retail tables and decode 5,807 functions across six packages without errors.
- [x] Audit lexical versus explicit-receiver named calls; report incomplete native ancestry without fabricating lookup success.
- [x] Locate standalone System/kwGame.u on the disc; resolve the 429 super links and 2 ownership checks after including its dependencies.
- [x] Add hybrid disc/archive package inventory for ISO and raw Mode 1 BIN; verify 12 script packages with one standalone package.
- [x] Add isolated receiver identities, class/state ownership checks and per-instance named dispatch across nested scalar calls.
- [x] Add typed primitive per-receiver property storage and script reads/writes with explicit host schemas/defaults; verify ten retail getters.
- [x] Implement Boolean wrappers for local/instance reads and assignment targets; verify 14 wrapped retail getters with true/false test values.
- [x] Add primitive default-variable reads (including Boolean wrappers) and inherited host-default overrides; test 95 retail-derived read fragments.
- [ ] Load original serialized class-default objects and validate default-write/class-context semantics.
- [x] Add bounded primitive instance struct paths, nested members and Boolean wrappers; validate 95 retail-derived read fragments.
- [x] Add read-only default-rooted primitive struct paths and inherited leaf defaults; verify eight retail-derived read fragments.
- [x] Resolve imported struct/property metadata across checked packages with kind-qualified identities.
- [ ] Add local/returned struct values, arrays, default writes and object-context struct access after validating layouts.
- [ ] Deserialize class defaults and packed/native property layouts; support object references/explicit contexts and real actor allocation.
- [x] Add isolated ObjectRef, Self/None and non-null object-context reads/calls with caller Self/argument scope and context restoration.
- [x] Add optional ObjectProperty class constraints with subclass/null checks; validate 1,290 retail-metadata schemas.
- [x] Add optional object argument/return class constraints; verify 2,891 retail-metadata call slots and return flag handling.
- [x] Add host-only receiver retirement, stale-handle rejection, no identity reuse and permanent schema freeze.
- [ ] Implement actual actor Destroy/pending-kill/events/GC/world removal and save/load lifetime semantics.
- [ ] Add null-context recovery, class contexts, context assignment targets and constrained object locals.
- [ ] Implement state lifecycle/bytecode execution and validate against retail behavior.
- [ ] Verify original native behavior against the DLLs before enabling PoA.
- [ ] Deserialize Class/State payloads and verify original runtime dispatch behavior; check remaining script packages.
- [ ] Connect the compatibility runtime to Android and pass a real PoA startup test.

These checks do not establish PoA gameplay support.


- [ ] Keep HP3 launcher entry disabled/experimental until verified game boot and rendering.
- [ ] Inventory actual HP3 installation data and package formats; document UE2 differences from HP1/HP2 UE1.
- [ ] Extend isolated parser/runtime tests for UE2 package imports, exports, name tables, properties and script bytecode.
- [ ] Map missing UE2 native functions, class/state semantics, physics, collision and actor lifecycle.
- [ ] Implement and test UE2 rendering, materials, skeletal animation, particles, audio/music and cutscenes as required by actual game data.
- [ ] Establish staged proof: parse assets → load first map → render first frame → input/camera → first interactive gameplay → saves → wider compatibility.
- [ ] Record unsupported features and reproducible traces; do not mark playable based on successful imports alone.

## P6 — Release and documentation

- [ ] Verify Android game-folder picker imports both HP1 and HP2 reliably (upstream release-roadmap equivalent).
- [ ] Update README with game import, HD pack optional setup, troubleshooting and known limitations.
- [x] Produce the corrected 0.4.1 Both Games signed ARM64 APK, full source archive, SHA-256 checksum and changelog locally. Physical-device acceptance remains open.
- [ ] Verify APK installs over previous release without losing saves, game imports or HD packs.
- [x] Publish v0.4.1 release assets and verify download links, asset sizes and checksums.
- [ ] Record physical-device test results before calling a release stable.

## Release acceptance gates (do not tick without on-device proof)
- [ ] HP2 opens its own main menu with visible background, readable labels and working touch navigation.
- [ ] HP2 New Game / Load Game / Options all work; settings survive relaunch.
- [ ] HP2 pause-to-menu and return-to-launcher routes work without freezes.
- [ ] HP2 mirror/window surface renders correctly with HD textures both disabled and enabled.
- [ ] HP1 menus, saves, controls and HD textures still work after the HP2 fixes.
- [ ] Build logs, screenshots, device model, APK version and commit SHA are recorded for each accepted fix.

## Local investigation — 2026-10-31

User direction: keep implementation, builds and commits local; no GitHub pushes.

- [ ] Opening PrivetDr mirror/window reflection glitches: base texture remains hidden by the invisible portal seal. Mirror grouping fix below is a separate defect and does not prove this scene fixed.
- [ ] Lumos light is offset from the wand tip; distinguish this from the existing persistent-beam issue.
- [ ] Some translucent walls render red; record map/surface/material and compare HD off/on.
- [ ] Spawned jelly beans sometimes fail to follow Harry between levels; inspect travel serialization and actor persistence.
- [ ] The game's own Options controls page contains empty rows; inspect binding enumeration/localization and Android input support.
- [ ] Verify the mirror plane grouping change on Android with mirrors on separate planes and HP1 Mirror of Erised regression checks.

Source finding: `Engine.cpp` explicitly excludes `HP2::RunFrontEnd()` on Android under the assumption that SetupActivity replaces it. The launcher is not HP2's six-slot New/Load/Options front end. Removing the guard alone has not been validated against SDL Android modal-window/input constraints.

Local change: mirrored BSP faces now share a reflection only when their oriented planes match. Previously every BSP mirror joined the first mirror portal, regardless of position/orientation. Pure C++ regression test passes; device acceptance remains open. See `native-runtime/docs/re/hp2/android-local-findings.md`.

## Local fixes — audio settings and HD progress

- [x] Remove the older HP2 console interceptor that swallowed `MUTESOUNDS` / `UNMUTESOUNDS`; use the existing HP2 audio handler. Source flow reviewed; affected C++ files pass host syntax checks. Device playback remains unverified.
- [x] Share volume saving across the HP2 front end, in-game `set` and `SETVOLUMES`; update both the HP2 and engine INI sections and flush changed values. Source/syntax verified; device relaunch test remains open.
- [x] Reject nonfinite or malformed volume values; pass the C++ volume parser/round-trip tests.
- [x] Use the existing engine fullscreen query rather than HP2's hardcoded `True` reply. Source/syntax verified.
- [x] Coalesce HD progress into one pending UI update; host burst test passes with 3,505 updates, including completion and destroyed-screen protection.
- [x] Report HD extraction/verification, copying, manifest save and cleanup; real installer host tests pass with synthetic DDS/ZIP fixtures.
- [x] Check interruption between committed HD files and verify rollback restores the previous texture and manifest, with temporary directories cleaned.
- [ ] Confirm audio behavior, saved volume reload and installer responsiveness on the ARM64 phone. None of the above closes an on-device release gate.

## 0.4.1 local APK — implemented and built; device gates remain open

- [x] Enable HP2's existing front-end code before engine startup; pass the complete ARM64 native build.
- [x] Add proportional layout and matching touch coordinates for landscape phone surfaces; pass small/wide-screen host regression tests.
- [x] Preserve the existing flat Save0..Save5 files while moving New/Load selection out of the launcher. Opening the menu never deletes an empty slot's shared save folder.
- [x] Add an in-game main menu with Resume, Save Game, Load Game, Game Book, Options, Return to Main Menu and Quit to Launcher; pass full Android Java compilation. Gameplay/input validation is still pending.
- [x] Route front-end Display/Controls buttons to the working shared Android settings; keep HP2's Sound page with the earlier volume persistence fix.
- [x] Keep original Help-folder menu bitmaps/sounds during ZIP and ISO imports; verify synthetic ZIP import and ISO path classification. Provide text/button fallbacks if earlier imports lack Help.
- [x] Exclude thumbnails and empty files when identifying saves; pass mixed-case and six-slot-boundary host tests.
- [x] Honor the slot passed to HP2 LoadGame, validate slot bounds and remember the chosen slot for internal restarts.
- [x] Save future persistent-world snapshots separately per Android save slot; retain legacy snapshot loading as a compatibility fallback. Full hub/bean testing remains open.
- [x] Add loaded-class lookup fallback for persistent spawned actors when EditPackages omits their package; pass the native build. Bean respawn behavior still needs a map test.
- [x] Change HP2 wand effect alignment from a bounding-box corner to the far end-cap centre and compensate for rendered length; pass geometry tests. Lumos attachment remains a candidate fix until phone comparison.
- [x] Build and sign 0.4.1 / versionCode 42 locally with the permanent certificate; verify ARM64 ELF, 16 KiB alignment, ZIP integrity, package identity and matching certificate.
- [ ] Install the new APK over the existing release and confirm saves/imports survive.
- [ ] Confirm menu graphics, all six save/load slots, overwrite confirmation, gamepad navigation, pause/resume and settings on the OnePlus.
- [ ] Confirm Lumos position while standing/moving/casting; red translucent walls, opening-scene mirror texture and the original book's empty controls rows remain unresolved.

Source commit: `ae991c1`. Build/test evidence is in `native-runtime/android/verification/0.4.1-*`. No compatibility success or physical-device playthrough is claimed.

## Work log (append after each batch)

| Date | Phase / task | Commit or PR | Automated tests | Android playtest | Outcome / blocker |
|---|---|---|---|---|---|
| 2026-11-02 | Batch E: UE2 header extended (depends offset + generation count) + 3 tests; native function mapping doc (Batch F) | 5091709 | 10/10 tests pass | Not run | No ISO used yet; parser ready for real data |
| 2026-11-02 | Batch C: src/hp3 skeleton + HP3_ENABLED guard (default off) + host tests | 5091709 | 5/5 tests pass | Not run | HP3 still disabled; no C++ compile yet |
| 2026-11-02 | Batch D: HP3 native registration placeholder (empty, guarded) | 5091709 | docs/tests only | Not run | Native mapping tracked in PoA roadmap; no bodies implemented |
| 2026-11-02 | Batch B: isolated UE2 package header validator + 7 tests (no engine changes) | a8eecd9 | 7/7 tests pass | Not run | HP3 still disabled |
| 2026-11-02 | Batch A: HP3 UE2 compatibility matrix + hp3 notes (design only; HP3 stays disabled) | 7350d2e | docs only | Not run | No runtime changes; UE2 profile not implemented |
| 2026-11-02 | Adopt 0.4.3 source checkpoint; port Italian HP1 detection (issue #15); add Windows ARM64 build pipeline; reconcile git history on top of 0.4.2 main | ad59709 | Python release/preflight/both-games tests pass; host tests not rerun | Not run | No new APK; test-signed 0.4.3 build pending user request; HP3 remains disabled |
| 2026-10-31 | Roadmap baseline captured | Documentation only | Not run | User reports HP1 playable; HP2 playable with issues; HD pack install success but freeze occurred | No engine fixes claimed |

## Next Work-mode instruction (copy/paste)

> Continue the HP:TPS Android project using `HP-TPS-ANDROID-ROADMAP-TODO.md` as the authoritative progress checklist, cross-checking against the latest upstream Flipendo ROADMAP.md. Start with P0 source audit, then P1 HP2 **main menu, functional settings, mirror texture** and other highest-priority fixes, followed by P2 installer freeze. Make real source changes in testable batches, run tests, commit and push changes, and update checkboxes only when verified. Preserve HP1 gameplay, Android signing, saves and optional texture packs. Keep HP3 experimental and separate. Do not claim APK or compatibility success without evidence.


| 2026-10-31 | Local source recovery, HP2 startup/render audit, mirror plane grouping | Local commit; see Git log | Mirror regression 1/1; Android release checks 8/8; check_repo passed | Not run | Opening-scene texture compositing, menu/settings, other reported issues remain open; no APK produced |

| 2026-10-31 | Local HP2 sound/settings fixes and bounded HD progress | Local commit; see Git log | 8 C++ tests; 8 release tests; 74 existing Java checks with synthetic fixtures; new queue/phase/rollback tests; syntax and repo checks | Not run | No APK build; main menu, opening-scene mirror base texture, red walls, bean travel and empty controls rows remain open |

| 2026-10-31 | Local 0.4.1 candidate: HP2 menu/save flow, imports, persistence and wand alignment | `ae991c1` | ARM64 build, Android Java compile, 10 C++ tests, 74 existing Java checks plus new menu/import/queue/rollback suites, 8 Python checks | Pending user test | APK signed locally; main menu and candidate gameplay changes require phone verification; no GitHub upload |

## Corrected 0.4.1 Both Games packaging

- [x] Confirm the GitHub v0.3.3 reference APK is 713,739,242 bytes and includes both games, with HD separate.
- [x] Recover both original game discs from the existing private uploads; verify all 330 previous HP1 manifest hashes.
- [x] Include both complete validated game payloads and installation stamps in `Harry-Potter-TQ-0.4.1-Both-Games.apk` (717,216,944 bytes).
- [x] Verify 339 HP1 files and 653 HP2 files by SHA-256 after embedding; verify all four 0.4.1 native libraries are unchanged.
- [x] Require both payload inputs in the release build and reject engine-only/missing-game/missing-config packages; 13 Python release tests pass.
- [x] Sign the corrected bundle as 0.4.1 / code 42 using the permanent certificate; verify package, signature, ARM64 and 16 KiB alignment.
- [ ] User installation/gameplay test, followed by GitHub upload only after user approval.

The earlier 9.9 MB engine-only 0.4.1 is superseded. Build and recovery evidence is in `native-runtime/android/verification/0.4.1-*`. Retail game data and signing keys remain outside the source repository.

| 2026-10-31 | Correct 0.4.1 packaging to include HP1 + HP2 | `3466e4a` packaging gate | 13 Python release tests; all bundled hashes, native library identity, signature and archive checks passed | Pending | Corrected Both Games APK 717.2 MB, code 42; no GitHub upload |

## Rebuild and publication — 2026-10-31

- [x] Recover the supplied full source, permanent signing backup and original HP1/HP2 ISOs; verify disc hashes and signing continuity against the earlier APK.
- [x] Rebuild ARM64 native libraries and Android Java locally; package both games (339 HP1 files / 682 HP2 files), verify all embedded hashes, signature, ZIP and 16 KiB alignment. APK: 718,670,864 bytes.
- [x] Rerun 13 Python release tests, 74 Java host checks with synthetic DDS data and save/import/progress/rollback suites.
- [x] Update README by game and record rebuild evidence. User has authorized GitHub upload and latest-release designation.
- [x] Publish v0.4.1 as Latest and verify all four uploaded assets match local sizes and SHA-256 digests (APK 718,670,864 bytes).
- [ ] Physical-device installation and gameplay acceptance; existing scene/rendering issues remain open.

## HP2 menu cropping — 0.4.2 candidate

- [x] Inspect phone report and trace inherited Canvas clipping boundaries not changing with menu drawing scale.
- [x] Preserve physical clip bounds on scale changes; regression passes for 1536×687 at four density scales and restoration.
- [ ] Confirm full menu, all six slots and touch navigation on the phone; local 0.4.2/code 43 build and signing checks pass; user approved publication as Latest.

- [x] Produce 0.4.2/code 43 locally with both games; verify embedded hashes, permanent certificate, ZIP integrity and alignment. APK 718,674,960 bytes. User approved publishing the new version.

## Local 0.4.3 source consolidation — 2026-11-01

- [x] Version the current local source as 0.4.3 / code 44, preserving package identity and canonical 0.4.2 fixes.
- [x] Include the complete native engine/dependencies, cumulative PoA work, tests and both roadmaps in a standalone source ZIP; no 0.4.1 ZIP is needed for extraction.
- [x] Refresh the complete source hash lock and verify every packaged file and ZIP integrity.
- [ ] Build and sign a 0.4.3 APK with the original certificate and verified HP1/HP2 payloads; not performed by source packaging.
- [ ] Complete remaining PoA compatibility/acceptance work in the master checklist; PoA remains disabled/unplayable.

## Inventory and build-input checkpoint — 2026-11-01

- [x] DATA-01: inventory all 75 standalone and 766 archived file records, preserving standalone kwGame.u; payload verification is separate.
- [x] Recover the original signing backup privately and verify the permanent certificate fingerprint; no key replaced or exposed in source.
- [x] Fix archive build-info generation to accept a complete verified source-lock identity without Git history; reject changed files.
- [x] Add safe complete inventory and read-only build preflight; 73 PoA Python and 18 packaging/preflight tests pass.
- [ ] DATA-04: inspect embedded assets in the complete package list; filenames/categories alone do not finish asset-level inventory.
- [ ] BASE-03: prepare SDK/NDK/compiler/build tools, previous signed APK and complete HP1/HP2 payloads, then verify signed build and device continuity.
- [ ] BASE-04: complete remaining package payload identities and region evidence. G0–G6 remain open; no APK or live PoA result.

See [inventory/build evidence](docs/POA-INVENTORY-AND-BUILD-INPUTS.md). Local source stays 0.4.3 / code 44; both master and project roadmaps reflect this checkpoint.

## Payload integrity and dependency checkpoint — 2026-11-01

- [x] Add extracted-package size/CRC/SHA/table checks, missing-versus-corrupt reporting and static transitive dependency tracing with cycle handling.
- [x] Extract and CRC-check ten additional original texture/mesh packages; record SHA-256 identities and table kinds. Current checked set: 24 valid of 110 packages, 86 not yet extracted.
- [x] Pass 83 PoA Python tests; ten new tests cover integrity and dependency rejection paths. Original assets and private signing inputs remain outside source ZIPs.
- [ ] Complete DATA-04/05/08: remaining assets, complete dependency closure and production/Android verification remain open. Current static frontiers (16 preamble / 21 story) are not final totals.
- [ ] Complete BASE-03/04 and all live PoA gates. No APK or live UE2 execution is claimed.

See [payload/dependency evidence](docs/POA-PAYLOAD-INTEGRITY-AND-DEPENDENCIES.md). Local source remains 0.4.3 / code 44.
