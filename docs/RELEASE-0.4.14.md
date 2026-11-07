# HP:Trilogy Mobile 0.4.14

Fix the phone-reported `Name index out of bounds!: KWGame` during HP3 startup.

The report identifies Class `CompanionCombatManager`, export 222 (zero based), in the EU `kwGame.u`. Its `EnemyLoc_DestPositionRange` default is a 55-byte tagged `RangeVector` containing three tagged `Range` members. The former loader read 24 bytes as flat floats, then misread `SerializeFrom` and name index -405680. The corrected loader consumes the tagged values and the following defaults in order.

## Changes

- Add shared tagged-property reading for class defaults and EU PoA Range/RangeVector values.
- Add the matching tagged save path and bounded nested-property framing in the production package writer; preserve UE1 flat Range and native Vector layouts.
- Keep property-offset bounds and verify tagged struct byte counts.
- Retain all seven Core native bindings, EU kwGame.u, optional IpDrv guards and the font fix from 0.4.13.
- Include an optional original-EU regression, a canonical engine patch and complete source.

## Validation and phone testing

The regression reproduces the exact former phone failure using the supplied EU package. The production reader passes nested defaults, save/reload, native-vector and legacy-Range cases, and rejects an invalid property offset. 107 compatibility tests and 29 Android packaging checks pass.

The Android ARM64 engine and launcher are rebuilt for 0.4.14 / version code 56 with the permanent signing certificate. All three game payloads and native alignment are verified separately.

Install over the existing app without uninstalling. This targets the reported startup failure; progressing beyond it, rendering, controls, saves and gameplay still need phone testing. No complete UE2 compatibility or playability claim is made.

