# Original PoA menu layout checkpoint

William requested the menu be added before APK assembly. `POA-MENU-ACCEPTANCE.json` keeps sourceIntegrationReady false, and the APK assembly step is gated by that value. Native builds and host checks may continue. Nothing here establishes an interactive original menu.

Four native GUIComponent bindings are implemented: ActualWidth, ActualHeight, ActualLeft and ActualTop. Original EU GUI.dll helpers are at 0x1001a730, 0x1001a8a0, 0x1001aa10 and 0x1001abc0. Values <=1 are normalized; values >1 are pixels. Width/height optionally scale against MenuOwner when bScaleToParent is set. Left/top optionally use and add the owner's rectangle when bBoundToParent is set. A missing Controller yields zero. Original float signatures and native index zero are checked against GUI.u.

The port resolves properties reflectively and uses the current full Canvas Size for the single Android viewport, never a clipped control region. Viewport/Controller initialization is still required. Cyclic owner chains are guarded; exact corrupt-input/exception parity is not claimed. No style drawing, initialization, focus/input, or page flow is supplied by these four bindings.

Host tests cover viewport and nested parent geometry, independent scaling and binding, pixel coordinates, the inclusive normalized boundary, negative values, NaN, missing context, cyclic chains and several viewport sizes. Both changed C++ translation units pass syntax checks. This is source/helper coverage, not original binary execution or a rendered menu.

Original GUI.dll SHA256: f23540ac83e1172e920fa47be25e40509a2b7c8cad13b52202dda6a668af44b5

## Controller and owned-control initialization

The render-ready PoA path now constructs the configured HGame.HPGUIController and dispatches its original InitializeController script event. A retained root protects it across allocation/callbacks, repeated calls do not initialize twice, failures clear the partial controller, and engine shutdown releases it. This is an initialization hook; it does not open or render the main page yet.

GUIController.GetStyle and GetMenuFont search the registered StyleStack/FontStack in order using case-insensitive keys and return the first match or None. Original EU helpers are 0x10018920 and 0x100187c0; they compare FString keys via Core's _wcsicmp. Current port comparison covers the shipped ASCII-style keys; general Unicode case parity remains open.

GUIMultiComponent.InitializeControls follows the original 0x10023110 helper: retain nonempty control lists, discover inherited GUIComponent object fields carrying flag 0x80000000, and insert them by ascending RenderWeight, placing a new equal weight before an earlier one. InitComponent's original scripts then assign owners/controllers and initialize each collected control.

The port's generic NewObject copies object references shallowly. To keep page state independent, GUI-owned inline controls are copied through the existing bounded script-object copier. Nested inline controls are copied, aliases preserved, and object/delegate references in fields, arrays and structs remapped. Resources outside the owned graph remain shared. Root assignments are staged so a failed copy leaves the page's original inline references intact. This is scoped to script-backed GUI components, not native resource/actor copying.

GUIController.GetMainMenuClass reads the original GameEngine.MainMenuClass property, following original helper 0x100186a0. GetCurrentRes uses the current viewport's width and height with the original `%ix%i` format (0x10018540). Canvas.DrawTileScaled now dispatches texture-backed scaled images without advancing the Canvas cursor or line state, following 0x103f79b0; non-texture UE2 materials and broader Canvas helpers remain open.

Production-body tests cover resource lookup, GC-root lifetime, re-entrant/error initialization, control ordering and inherited fields, owned graph isolation/aliases/delegates/nested arrays and structs, failed-copy atomicity, accessors, and drawing cursor restoration. These adapters establish source behavior, not a real PoA menu. Style/text drawing, fonts, remaining Canvas functions, controller focus/input, page opening and New Game/Load/Options execution remain unaccepted.
