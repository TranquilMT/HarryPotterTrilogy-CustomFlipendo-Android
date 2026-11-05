# HP:Trilogy Mobile 0.4.7

- Version code: 48. Package: `com.tranquil.hp1native`.
- Base: GitHub 0.4.5 plus retained compatibility work.
- Three matching launcher covers, independent game selection and PoA folder/ZIP support.
- Three mandatory game payloads; HP3 uses the newly uploaded EU disc only.
- PoA installer checks SHA-256 and size, uses atomic file replacement, and preserves saves and nonempty configuration.
- APK verification checks payload contents, AArch64 libraries, 16 KiB alignment and the permanent signing certificate.
- Native compilation, Java compilation and host checks are build evidence only.
- Phone acceptance remains open for all games. PoA startup, rendering, native functions, character swapping, save/load and optimisation remain unverified.
- Build state: signed three-game APK verified; Published as GitHub latest release [v0.4.7](https://github.com/TranquilMT/HarryPotterTrilogy-CustomFlipendo-Android/releases/tag/v0.4.7). Phone acceptance remains open.

## PC controls reference

The PC campaign selects the active character through story sequences; it does not use the console version’s free character-swap button. Keep story transitions and creature-control return as separate phone acceptance checks. Reference: https://www.supercheats.com/pc/walkthroughs/harrypotterandtheprisonerofazkaban-walkthrough04.txt . The EU DefUser.ini maps Enter to Use, Tab to ShowMap, Space to Jump and middle mouse to Zoom.

## Verified build evidence

- ARM64 native build and Android API 36 Java compilation passed.
- 97 PoA regressions, 28 release checks, 13 installer checks and 22 launcher/path/timing checks passed.
- All three payloads checked by size and SHA-256 after signing.
- Four Android native libraries are AArch64 with 16 KiB LOAD alignment; packaged library alignment checked.
- APK certificate matches the permanent 0.4.2 pin.
- Original PC game libraries under assets are data, not Android native libraries.
- No Android device execution was performed.
