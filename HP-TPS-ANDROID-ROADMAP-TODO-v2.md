# HP:TPS Android — Development Roadmap & Tick-off Checklist

**Project:** [HarryPotterTrilogy-CustomFlipendo-Android](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android)  
**Upstream:** [Flipendo ROADMAP.md](https://github.com/kroplabeskidu/flipendo/blob/master/ROADMAP.md)  
**Baseline:** Android v0.3.3; upstream roadmap status dated 2026-10-27; checklist prepared 2026-10-31.  
**Rule:** `[x]` only after implementation **and** evidence-based verification. `[ ]` means open, including partially completed or unverified work. Do not confuse upstream progress with our Android port's progress.

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
- [ ] Record current Android build configuration, dependencies, native libraries, and changes relative to upstream.
- [ ] Create a compatibility matrix for HP1, HP2, HP3 (import, boot, menu, gameplay, audio, save/load, renderer, controls).
- [ ] Establish repeatable Android ARM64 build and smoke-test commands; document prerequisites.
- [ ] Preserve application ID `com.tranquil.hp1native` unless existing current source proves a deliberate change.
- [ ] Verify existing signing certificate continuity and upgrade install (no private keys in repo).
- [ ] Add regression tests for HP1 launch, saves, Default.ini/DefUser.ini repair, and HD pack toggle.
- [ ] Track each fix in this checklist with source reference, commit, tests and phone result.

## P1 — HP2: Chamber of Secrets (upstream roadmap priority)

### HP2 main menu and front-end parity — Android release blocker
- [ ] **Implement/restore the Chamber of Secrets main menu** in the Android runtime (currently reported missing); do not mistake the launcher game selector for HP2's own front end.
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
- [ ] Produce versioned signed ARM64 APK, full source archive, SHA-256 checksum and concise changelog.
- [ ] Verify APK installs over previous release without losing saves, game imports or HD packs.
- [ ] Publish release assets and verify download links, asset sizes and checksums.
- [ ] Record physical-device test results before calling a release stable.

## Release acceptance gates (do not tick without on-device proof)
- [ ] HP2 opens its own main menu with visible background, readable labels and working touch navigation.
- [ ] HP2 New Game / Load Game / Options all work; settings survive relaunch.
- [ ] HP2 pause-to-menu and return-to-launcher routes work without freezes.
- [ ] HP2 mirror/window surface renders correctly with HD textures both disabled and enabled.
- [ ] HP1 menus, saves, controls and HD textures still work after the HP2 fixes.
- [ ] Build logs, screenshots, device model, APK version and commit SHA are recorded for each accepted fix.

## Work log (append after each batch)

| Date | Phase / task | Commit or PR | Automated tests | Android playtest | Outcome / blocker |
|---|---|---|---|---|---|
| 2026-10-31 | Roadmap baseline captured | Documentation only | Not run | User reports HP1 playable; HP2 playable with issues; HD pack install success but freeze occurred | No engine fixes claimed |

## Next Work-mode instruction (copy/paste)

> Continue the HP:TPS Android project using `HP-TPS-ANDROID-ROADMAP-TODO.md` as the authoritative progress checklist, cross-checking against the latest upstream Flipendo ROADMAP.md. Start with P0 source audit, then P1 HP2 **main menu, functional settings, mirror texture** and other highest-priority fixes, followed by P2 installer freeze. Make real source changes in testable batches, run tests, commit and push changes, and update checkboxes only when verified. Preserve HP1 gameplay, Android signing, saves and optional texture packs. Keep HP3 experimental and separate. Do not claim APK or compatibility success without evidence.
