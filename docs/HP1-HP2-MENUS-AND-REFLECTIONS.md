# Menu test build and shared reflection correction

## Implemented

The OpenGL scene shader previously left near-plane clipping commented out. Reflected views supplied a mirror plane, but triangles crossing it could still shade the wrong side of the reflection. The shader now interpolates the signed view-space plane distance and discards negative fragments. This works on GLES 3.0 without requiring the optional clip-distance extension. A zero plane retains menu/2D rendering.

The production shader resources and production Android converter are compiled and linked in a real Mesa GLES context. Five rendered pixel cases verify both clipping orientations, complete rejection, complete retention and disabled clipping. These checks establish plane clipping, not visual acceptance of a specific HP2 window scene. HP1 and HP2 both use this renderer.

The ARM64 workflow now assembles a version 0.4.15-test/code 57 unsigned APK from the verified three-game 0.4.14 payload, existing launcher dex, rebuilt native libraries and matching new Android resources. Old signature entries are removed; the output is aligned and checksummed. It must be signed with the original private key before installation. Saved signature-reconstruction data only applies to the exact old APK and cannot sign these changes. No replacement signing identity is generated and no release is published automatically.

## PoA original menus

The EU static audit covers GUI, UDebugMenu, kwGame, UWindow, HGame and the Engine Canvas/Console/GUIController declarations. It finds 42 native declarations, of which 23 still have no direct class/name registration candidate after adding layout, resource lookup, control initialization, accessors and scaled-image bindings. These are investigation candidates: inherited or numeric dispatch must be checked before calling each one a missing runtime binding.

The candidates include GUI component dimensions and timers, GUI style/text drawing, fonts, control initialization, list operations and Canvas tile/text helpers. See HP3-MENU-NATIVE-AUDIT.json with package hashes. Original menu boot, callbacks, input and rendering remain incomplete; this update does not claim an interactive PoA menu. Existing missing-config handling already covers the historical Hardware2Options.ini crash, but needs phone execution in the changed APK.

## Next acceptance and improvements

1. Sign and install the current three-game test APK with the permanent certificate; retain app data.
2. HP1: main menu, save/load, spell targeting, touch/gamepad, mirror rooms and optional HD packs.
3. HP2: main menu and six slots, Options/control settings, mirror/window scenes at several angles, translucent red-wall issue, Lumos alignment and beans across level travel. The clipping fix does not establish that these other issues are resolved.
4. PoA: record the first current startup/menu failure, trace the original GUI dependencies and implement each required native with its original semantics; then test language selection, New Game, Load and Options.
5. Resume collision scene acceptance, including slopes, steps, platforms and exact rotated-brush behavior. Host geometry checks alone do not close that area.

The original permanent signing backup has now been recovered and its certificate verified. APK assembly is explicitly gated until menu source integration is ready, following William's latest instruction. See HP3-MENU-LAYOUT.md.
