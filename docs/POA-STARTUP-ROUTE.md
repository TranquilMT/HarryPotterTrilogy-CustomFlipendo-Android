# Original PoA identity and startup route

Checkpoint: `poa-startup-20261101`. Tasks: BASE-04, BASE-07 and the supporting offline inspection in DATA-02.

## Selected acceptance route

The original Default.ini selects `HP_preamble.unr` through `[URL] LocalMap`. Its separate `Map=HP-Test.unr` field is not the selected local startup route. The supplied map filename is `maps/HP_Preamble.unr`; preserve that spelling on Android while resolving the original case-insensitive reference.

Use the original startup map, reach the front end, choose New Game, then load `HP3_Adv1Express.unr`, selected by `[HGame.HPPageFrontend] FirstLevel. The first controllable section in this map is the chosen interactive acceptance target. The configuration and package tables establish this target selection; the complete scripted transition, camera handover and player control have not run in the port.

| Original configuration | Verified selection |
| --- | --- |
| DefaultGame | HGame.HGame |
| Console | KWGame.KWConsole |
| GUIController | HGame.HPGUIController |
| WorldInfo | HGame.HPWorldInfo |
| LevelLoadingPlayerControllerClass | hgame.HarryController |
| LevelLoadingHUDClass | hgame.hphud |
| ServerPackages | AmbientCreatures, Decorations, GamePlay |

All six configured classes are present as Class exports in their actual script packages. Both maps have one Level export and at least one PlayerStart export. The preamble also contains MovieTrigger and ACTION_GotoMenu exports; reaching it alone will not prove that movies, the GUI or gameplay work.

The two maps and original executable/configuration were extracted from the provided disc image's compressed.zip without running the installer or executable. Sizes and archive CRCs match. `POA-STARTUP-IDENTITY-VERIFICATION.json` records SHA-256 identities for the executable, two configuration files, all 12 script packages and both maps, with table inspection and dependency results.

## Identity and edition limits

The original executable is 1,455,177 bytes, SHA-256 `1ac9c1e37b1cd8d9c581558be8c8ebb607ef0853b66c698fa3b881dcb9b54b0d`, matching the retained reference profile. The image was supplied as HPPoA_Na.bin, its volume label is HPPoA and the original configuration selects Language=int. North American is the supplied image's identification; the generic volume label does not independently prove a retail region. No hash of the entire disc was computed in this checkpoint.

This identity record covers all 12 script packages and two maps, not every original game asset. BASE-04 remains in progress until the complete required package hash inventory and edition identification gaps are resolved. Other executable/package hashes must be tracked as distinct unverified data sets, not silently accepted as this reference edition.

## Tooling correction

`tools/poa/startup_profile.py` reads the actual configuration, verifies configured classes and server packages against inspected tables, resolves map paths while preserving their actual spelling, and checks selected map exports. It refuses missing/duplicate singleton settings, case-colliding paths, unsupported map URLs, absent maps/classes and missing configured server packages. Repeated INI list keys remain lists, and commented settings are excluded.

`build_profile.py` now uses those verified settings instead of hard-coded startup/game/console/GUI values. It rejects stale package inspection hashes and assigns a distinct disabled profile ID to an unrecognized executable or script set. Generated profiles remain inspection-only regardless of these checks.

Twelve new regression tests cover actual configuration-driven selection, case handling, rejection paths, unknown executable separation and stale audits. All 66 Python tests pass. Original-data checks also cover 1,685 classes, 546 states, 8,090 function decodes, 8,090 function schemas and 499 native argument orders across all 12 scripts, with no reported errors. These are offline checks; native ABI/behavior, full Class/State payloads, defaults and live objects remain unverified.

## Remaining acceptance

BASE-07 is complete only as the selection of a verified original-data test route. G0 remains open for signed build/payload/device continuity and the remaining baseline evidence. G1 requires live object/dependency loading; G2 requires real Android startup/rendering; G3 requires actual input and progression through the selected interactive section.

The private audit folder currently contains only the scripts and two maps. The first story map references 20 additional root packages not present in that audit folder, including character/prop animation, textures, meshes and sound. Those dependencies remain under DATA-04/DATA-05 and related runtime tasks. Zero missing script roots in the preamble table is not proof of complete startup dependency closure; configuration, dynamic loads, native resources and movies must also be checked.

PoA remains disabled and unplayable. No APK, live first frame or gameplay result was produced by this checkpoint.
