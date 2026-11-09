# Original PoA menu layout checkpoint

William requested the menu be added before APK assembly. `POA-MENU-ACCEPTANCE.json` keeps sourceIntegrationReady false, and the APK assembly step is gated by that value. Native builds and host checks may continue. Nothing here establishes an interactive original menu.

Four native GUIComponent bindings are implemented: ActualWidth, ActualHeight, ActualLeft and ActualTop. Original EU GUI.dll helpers are at 0x1001a730, 0x1001a8a0, 0x1001aa10 and 0x1001abc0. Values <=1 are normalized; values >1 are pixels. Width/height optionally scale against MenuOwner when bScaleToParent is set. Left/top optionally use and add the owner's rectangle when bBoundToParent is set. A missing Controller yields zero. Original float signatures and native index zero are checked against GUI.u.

The port resolves properties reflectively and uses the current full Canvas Size for the single Android viewport, never a clipped control region. Viewport/Controller initialization is still required. Cyclic owner chains are guarded; exact corrupt-input/exception parity is not claimed. No style drawing, initialization, focus/input, or page flow is supplied by these four bindings.

Host tests cover viewport and nested parent geometry, independent scaling and binding, pixel coordinates, the inclusive normalized boundary, negative values, NaN, missing context, cyclic chains and several viewport sizes. Both changed C++ translation units pass syntax checks. This is source/helper coverage, not original binary execution or a rendered menu.

Original GUI.dll SHA256: f23540ac83e1172e920fa47be25e40509a2b7c8cad13b52202dda6a668af44b5
