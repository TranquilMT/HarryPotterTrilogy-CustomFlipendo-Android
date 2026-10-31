# HP2 Android local investigation — 2026-10-31

Source snapshot: TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android main,
retrieved locally on 2026-10-31. No remote writes were made.

## Findings and limits

`Engine::Run` in `src/engine/SurrealEngine/Engine.cpp` excludes
`HP2::RunFrontEnd()` under `#ifndef __ANDROID__`, with the comment that
SetupActivity supplies a launcher. `HP2FrontEnd.cpp` already implements New
Game, six save slots, Load Game, Video and Sound pages. Re-enabling the call
requires checking SDL Android modal-window lifetime, fullscreen geometry,
touch routing and the transition to the engine window. No menu restoration
is claimed in this batch.

`VisibleFrame::ProcessSurface` groups every BSP mirror into the first mirror
portal, without comparing planes. `DrawPortals` subsequently uses only the
first node's plane to reflect the camera. Mirrors on different planes thus
receive the wrong view and clipping plane. The local change restricts
grouping to matching oriented planes. It retains existing HP1 rendering
semantics and does not change texture blending or save data.

`DrawPortals` also seals every mirror using `PF_Occlude | PF_Invisible`.
Only `PF_NoOcclude` surfaces are queued for a subsequent textured pass.
The upstream roadmap identifies PrivetDr surface 991, `extwindow_PD`, as
`PF_Mirrored` without that extra flag. Its base texture is therefore absent.
This remains unresolved: original HP2 Render.dll behavior or comparative
captures are needed to select the correct compositing rule. An arbitrary
modulated/translucent blend could hide the symptom while changing the scene.
The plane-grouping test does not establish that the opening scene is fixed.

## Reported issues still awaiting reproduction

| Issue | Next check |
|---|---|
| Opening-scene mirror glitch | PrivetDr surface 991, PC comparison, HD off/on, camera movement |
| Lumos light offset | Wand attachment transform and animation pose; separate from persistent beam |
| Red translucent walls | Map, material flags, modulation order and HD override alpha |
| Spawned beans lost on travel | Spawned-actor ownership, travel flags, hub persistence and save reload |
| Empty controls rows | Options binding enumeration, localized labels and supported Android mappings |

User reports story progression, six saves, shops, duels, Russian text,
subtitles and gamepad support. These are reports, not independent playtests.

## Validation

The actual `KW::SameMirrorPlane` helper is exercised through the existing
Flipendo C++ test runner: same plane, tolerable rounding, separated parallel
planes, perpendicular faces, reversed normals and invalid coordinates.

```sh
c++ -std=c++20 -Wall -Wextra -Werror src/tests/TestMain.cpp src/tests/TestMirrorPlane.cpp -o /tmp/hp-mirror-tests
/tmp/hp-mirror-tests
PYTHONPATH=android python -m unittest discover -s android/tests -p 'test_*.py'
python tools/check_repo.py
```

Results: 1 C++ test passed, 8 release-check tests passed, repository checks
passed. No Android SDK/NDK/JDK or permanent signing key was available in this
workspace, and no APK build or physical-device playtest was performed.
The checked-in patch includes the renderer change; the expanded source is
also updated because this distribution builds its already-patched sources.


## Batch 2: audio settings and HD installer

`Engine::ConsoleCommand` calls `HP2::ConsoleCommand` before the later
`HP2::AudioCommand` fallback. The former had no-op mute/unmute handlers,
so cutscene skipping never reached the already implemented mute state in
`HP2Music.cpp`. Those duplicate handlers are removed. SETVOLUMES now also
uses the common audio handler rather than the earlier duplicate.

The front end saved only `[Engine.SurrealAudioDevice]`, while startup
`LoadAudioVolumes` always rereads `[ALAudio.ALAudioSubsystem]`. This could
restore an older volume on relaunch. `SaveAudioVolumes` now writes both
sections and saves the INIs. The front end and both in-game setting paths
use it. Unchanged values avoid redundant writes; malformed/nonfinite
values preserve the current setting. The hardcoded HP2 fullscreen reply
is removed so the engine reports its actual window state.

The HD worker already runs off the UI thread. However, each verified
texture queued an independent UI callback, and subsequent copy/manifest/
cleanup phases had no progress messages. `LatestProgress` bounds that
queue to one callback at a time, at most one update per 100 ms. Completion
and Activity destruction discard pending progress. Installation now reports
all those later phases. Interruption checks in the commit loop use its
existing rollback to preserve previous overrides.

Validation for this batch:

- Actual HP2 audio decoder/parser and mirror helper tests: 8 passed, with
  added NaN/infinity/malformed-value cases and all 256 volume round trips.
- HP2Console, HP2Music and HP2FrontEnd compile in host syntax-only mode,
  also with `__ANDROID__` defined. This is not an Android ABI build.
- Existing Java launcher checks: 30 passed; external-HD checks: 12 passed;
  bundled-HD checks: 32 passed. Inputs were synthetic package headers and
  valid minimal DDS/ZIP fixtures, not original game assets.
- New Java tests verify 3,505 queued reports collapse into one pending
  callback, the latest phase displays, stale callbacks cannot overwrite
  completion, and closed progress ignores further worker reports.
- New real-installer test verifies phase messages and interruption after a
  replacement: the original DDS and manifest survive and staging/backup
  directories are removed.
- Python Android release checks: 8 passed. Repository checks passed.
- Host Java installer tests used JSON-java sources from the official
  `stleary/JSON-java` repository as a temporary test dependency; those
  sources are not part of the delivered project.

The original physical-device freeze is not independently reproduced or
confirmed fixed. The existing Android front-end exclusion remains in
place until its window/input handoff can be validated. The opening-scene
mirror texture compositing, red translucent walls, spawned-bean travel,
wand-tip light offset and empty controls rows still need runtime evidence.
