# PoA function script metadata and token-reader checkpoint

4 November 2026. Continues the v0.4.5 compatibility branch. Source changes only; no APK or live gameplay acceptance.

## Verified metadata slice

The retained original-data inspectors (`ObjectReader.h` and `object_payloads.py`) place a zero-valued extension byte after a version-129 PoA Function export's friendly name. Production `UStruct::Load` previously read that byte as part of Line, shifting the following metadata and script size.

The production loader now selects this narrow extension only when the running game identifies as PoA and the object is a UFunction. It checks version 129 and rejects unverified nonzero extension values. Shared helpers read/write Line, TextPos and the declared bytecode memory size, enforcing the existing isolated reader's 64 MiB limit. Other games retain the 12-byte header without the extension.

The corresponding function-header write helper includes the extension for version-129 output and rejects unverified PoA output versions. This is header consistency, not complete game-save support. Class/State/other Struct layouts are not inferred from the verified Function layout and still require their own adaptation.

## Production bytecode deserialization fixes

- PoA token 0x60 is handled as an extended native-call token, matching the isolated verified decoder. The production reader previously consumed it through UE1's overlapping conversion range, shifting the stream. This corrects decoding; it does not verify the native binding or call ABI.
- PoA tokens 0x39–0x5f are rejected explicitly while their production evaluation remains unsupported. These values overlap UE1 conversions but have different UE2 meanings. Interpreting them as UE1 expressions would be incorrect.
- Decoded bytecode growth is bounded before appending both bytes and expanded operands. Oversized declared scripts fail before parsing their bodies.
- Unicode script strings now serialize UTF-16 code units individually. Copying `(length + 1) * 2` bytes from `wstring` memory corrupts multi-character strings when `wchar_t` occupies four bytes, as on Android/Linux. Surrogate pairs remain pairs of serialized UTF-16 units.

## Evidence and limits

The new `test_production_struct_scripts.py` compiles the actual production `UStruct::ReadToken` and append methods against stubbed engine identity/stream interfaces. It runs 75 cases with AddressSanitizer and UndefinedBehaviorSanitizer:

- Function-header positions/fields agree with the retained isolated reader; legacy metadata remains unshifted, truncated headers and unknown versions/extensions are rejected, and oversized writes fail.
- Extended and direct native-call syntax, compact-reference expansion and UTF-16 strings agree byte-for-byte with the isolated decoder for the covered expressions.
- Unsupported UE2 token overlaps fail; legacy UE1 conversion and HP1 array-length deserialization paths retain covered behaviour.
- Recursion and decoded-memory limits fail explicitly.

The complete PoA Python regression suite now has 96 passing tests, including the prior 100-case production package-table test. The isolated C++ runtime suites' earlier results remain historical; no unchanged full-suite rebuild is claimed here.

This test does not compile the complete UStruct metadata/property layout, engine, native registry or Android app. Stubs do not establish native object ownership, GC, property offsets, VM execution or scene rendering. Original functions have not been run through the complete production path in this workspace.

PKG-05/06 and VM compatibility remain in progress. Required Class/State/default/property/native/resource work, original-data loading, Android build/device checks and G0–G6 stay open under the full 21-workstream roadmap.
