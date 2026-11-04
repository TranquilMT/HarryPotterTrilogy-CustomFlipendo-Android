# Three-game launcher, controls and performance checkpoint

Base: v0.4.5, with the earlier package-loader fixes and post-tag launcher changes retained. Source checkpoint only; app version/code/signing and bundled payloads are unchanged. This patch has not been built as an APK or exercised on a phone.

## Launcher defects addressed

- HP3's picker used request 13, already owned by the HD ZIP picker, so the HD handler consumed the folder result. HP3 now uses request 14.
- HP3 wrote the HP1 folder preference and followed the Boolean HP1/HP2 setup path. Selection, highlighting, preference keys, folder settings, import guidance and launch intent now recognize all three IDs.
- PoA preparation validates original PoA files separately. It never installs HP1 data, repairs PoA with HP1 defaults, or applies HP1 translations, movement mods or HD packs to PoA. A legacy saved PoA path can be copied to its own preference without overwriting HP1's preference.
- PoA preflight previously asked for HP2-only HProps/HPModels/HPSounds scripts. It now checks the PoA script names recorded in the retained original-data profile, configurations and map/texture presence. Required core script headers must be version 129. This is a minimum preflight, not complete dependency validation.
- Native Android directory detection omitted hppoa.exe and silently fell back to the default game when the explicit selected folder was rejected. It now recognizes all three executables with Windows-style case differences, rejects empty/non-file executables, and reports an invalid explicit folder instead of starting another game.
- The in-game game chooser lists all three titles. Missing-data messages retain the launcher so another title can still be selected.

## PoA PC controls and character selection

The PC version selects Harry, Ron or Hermione according to the current sequence; manual on-demand character switching belongs to the console versions. Keep the original PC script flow. Creature possession is a separate action, with Enter used to return from the controlled creature.

References:

- [GameSpot PC review](https://www.gamespot.com/reviews/harry-potter-and-the-prisoner-of-azkaban-review/1900-6100652/)
- [GameSpot preview distinguishing the PC and console controls](https://www.gamespot.com/articles/harry-potter-and-the-prisoner-of-azkaban-impressions/1100-6091212/)
- Retained original PC bindings: `native-runtime/tools/poa/reference-profile/hp3-controls.json`.

| Android control | Original PoA PC input | Intended action |
| --- | --- | --- |
| Left stick / directional pad | W / S / A / D | Move and strafe; PoA's Left/Right arrows turn instead |
| Wand button, held / gamepad X | Left mouse | Fire/held spellcasting; drag the wand to aim |
| Jump / gamepad A | Space | Jump, creature wing flap and the original popup completion binding |
| USE / gamepad Y | Enter | Use, return from creature possession; context-dependent original action |
| MAP / right-stick click | Tab | Show map |
| ZOOM | Middle mouse | Zoom, held |
| WALK / left-stick click | Shift | Walk, held |
| MENU / B | Escape | Original game menu |
| Cutscene SKIP / hold Y | Enter | Original PoA bypass action |

No manual swap button or invented swap native is added. These are Android-side bindings for the future live PoA path, not evidence that its UE2 VM/world can already process them. HP3 quick-save remains hidden because the existing quick-save bridge is HP1/HP2-specific. The extra PoA controls use safe-area positions; the existing layout editor still edits its four established groups.

## Shared input and performance changes

- Release all tracked keys, mouse buttons, touch roles and gamepad state on Android touch cancellation, focus loss, pause, detach, settings entry and game/menu/cutscene transitions. Previously cancellation handled one pointer only.
- Invalidate delayed input callbacks on these transitions, preventing a queued menu press from firing after cancellation. Reset pending gamepad menu-click state and remove aim/navigation callbacks.
- Prevent a second finger from taking an occupied button. Suppress repeated button/key-down injection and preserve the mouse-button mask when cast and jump are held together.
- Frame-scheduled aim/gamepad motion uses elapsed time, so display refresh rate does not multiply camera speed. Clamp a stall's contribution and apply the selected sensitivity/inversion to camera swipes and stick look.
- Stop native-state polling and repeated control callbacks while unfocused. Reuse toolbar and arrow arrays when drawing. Remove per-spell-collision diagnostic formatting/logging from the shared KnowWonder touch path without altering touch events.
- Stop scanning/decoding/rebuilding HP2 save cards which were already hidden because the game owns its save menu.
- Persist render scale, FPS limit, FOV, UI size and touch options separately for HP1, HP2 and HP3. On first use each profile inherits existing preferences. Renderer-backend selection and control-layout offsets retain their existing storage.

These changes reduce identified launcher/overlay work and improve input behaviour. No FPS, temperature, GPU-time or memory improvement is asserted without measurement. PoA still requires production UE2 loading/native/resource work.

## Validation

Run `python3 native-runtime/android/tests/check_launcher_controls.py`.

- 22 checks compile and exercise actual GameFiles/ControlTiming: independent roots/preferences, mixed-case and nested/System-folder selection, PoA-only requirements, wrong/truncated script headers, unchanged configuration files and 30/60/120 Hz timing totals.
- Compile and exercise actual Overlay release/key/button/callback method bodies with a host input sink: held mouse chords, duplicate press suppression, WASD/Space/Enter mapping, multi-input cancellation, focus changes and stale callbacks.
- Compile and run actual native HasGame under AddressSanitizer/UndefinedBehaviorSanitizer for all three titles, mixed casing and invalid executable directories.
- Parse both Android activities with javac for syntax; this does not perform SDK/SDL type checking.
- The existing 94 PoA Python tests pass. Previous C++ package/runtime test evidence remains recorded in the earlier checkpoint; those loader files were not changed here.

## Required device/build acceptance

- [ ] Build with the complete SDL/Android SDK/NDK dependencies, preserving the release key and both existing game payloads; verify APK signature/checksum.
- [ ] Cold start and switch each title; return from cancelled picker, settings, permission page and interrupted game; ensure no title uses another title's folder/configuration.
- [ ] Select PoA root, parent wrapper and System folder; verify that the engine receives the selected PoA root and reports the next real UE2 compatibility failure without starting HP1.
- [ ] HP1 and HP2: move/look/cast/jump together, lift either finger, cancel touches, pause/resume and open settings while holding inputs; verify no stuck controls and no input after cancellation.
- [ ] Exercise actual menus, spell drawing, broom/creature sequences, gamepad navigation, PoA Use/Map/Zoom/Walk, possession return and original story-driven character selection once the required runtime works.
- [ ] Measure CPU/GPU time, startup, memory and sustained thermal behaviour separately for each game on the target OnePlus device before setting quality defaults or claiming an optimization gain.

All live compatibility gates remain open.
