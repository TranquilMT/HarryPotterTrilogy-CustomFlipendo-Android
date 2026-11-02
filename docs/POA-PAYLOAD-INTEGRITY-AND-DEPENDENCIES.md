# Extracted package integrity and dependency checkpoint

Local source 0.4.3; checkpoint `tq-0.4.3-payload-integrity`. Advances BASE-04, DATA-04, DATA-05 and DATA-08. These tasks remain open at their full stated scope.

## Original data checked

The local analysis set now contains 24 of the 110 inventoried Unreal packages: all 12 scripts, two selected maps, five additional texture packages and five additional static-mesh packages. The ten added payloads were extracted from the original disc archive, with exact size and CRC checks, then hashed with SHA-256 and inspected for bounded package tables. They are not included in the source ZIP.

| Added texture packages | Added mesh packages |
| --- | --- |
| KWMenu.utx | HP3_SkyBoxes.usx |
| HP3_LoadLevel.utx | HP3_Particles_SM.usx |
| SpellShapes.utx | HP3_CutScenes_SM.usx |
| HP3_SystemIcons.utx | HP3_HogwartsExpress_SM.usx |
| GUIContent.utx | HP3_Furniture_SM.usx |

The checked tables expose Texture, Palette, MovieTexture, WetTexture, Shader, ConstantColor, FadeColor, Combiner, TexPanner, FinalBlend, StaticMesh and KMeshProps exports. This identifies required asset kinds; it does not deserialize their native payloads or establish rendered behavior. These ten packages include versions 120, 123, 128 and 129, licensee version 0. A universal version-129 layout is therefore insufficient.

`POA-ADDITIONAL-ASSET-IDENTITIES.json` records hashes and aggregate table counts. `POA-PACKAGE-INTEGRITY-VERIFICATION.json` records the full current verification/dependency report.

## Verification tool

```sh
python3 native-runtime/tools/poa/verify_game_data.py /path/to/extracted-game \
  docs/POA-FULL-DISC-INVENTORY.json package-integrity.json \
  --reference docs/POA-STARTUP-IDENTITY-VERIFICATION.json \
  --root HP_Preamble --root HP3_Adv1Express
```

The command checks all expected package paths, sizes, archive CRCs and any supplied pinned reference SHA-256 hashes. It calculates actual SHA-256 hashes, validates package tables and preserves actual path spelling. A standalone file without a CRC or supplied reference hash remains unanchored. Invalid tables do not become valid merely because their CRC matches a supplied record.

It rejects ambiguous package identities and unsafe inventory paths, flags unexpected package files, and distinguishes missing payloads from damaged or unanchored files. It returns a nonzero status when required packages or a requested static dependency route remain incomplete. The current original-data run intentionally returns nonzero: 24 valid, 86 still unextracted, zero invalid/unanchored packages and no unexpected package files.

## Dependency frontier

The tool traces root-package imports transitively, handles legitimate cycles without looping, and retains missing and outside-inventory nodes. Unknown dependencies are not silently treated as built-in native packages.

The preamble's currently inspected chain reaches 16 unextracted packages, and the first story map's chain reaches 21. These are **current static frontiers**, not final totals of remaining work. Inspecting the added packages exposed further imports; inspecting the remaining packages can expose still more edges. Dynamic loads, original native resources, configuration and movies are not covered by this graph, and neither route has complete closure.

The earlier zero missing direct script roots in the preamble table did not establish startup readiness: its script packages import additional GUI, material, character, mesh and sound packages. This checkpoint makes that distinction explicit without changing the master scope.

Ten new regressions cover valid/missing data, same-size corruption, pinned SHA mismatch, unanchored standalone files, malformed packages, ambiguous/unsafe paths, unexpected packages, cyclic dependencies and unknown transitive imports. All 83 PoA Python tests pass. C++ is unchanged; prior sanitizer results remain historical.

No full task is newly completed by this partial extraction/verification batch. BASE-03 build inputs and BASE-04 full identities remain open; DATA-04/05/08 still require complete inspection, closure and production/Android integration. G0–G6 remain open. No APK, live UE2 loading or gameplay result was produced; PoA remains disabled.
