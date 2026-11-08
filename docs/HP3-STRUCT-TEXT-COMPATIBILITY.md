# UE2 property text and PoA struct-config compatibility

8 November 2026. Source follow-up; not included in the published 0.4.14 APK.

## Repairs

The shared property-text parser previously truncated at the first closing parenthesis and split every comma, including commas inside nested structures and quoted strings. It now tracks nesting and single/double-quoted literals, preserves escaped characters, trims whitespace, and rejects unbalanced or malformed text. Assignments retain their original order so duplicate case-insensitive NameString keys keep last-value-wins behavior.

UStructProperty text import/export now walks the complete reflected Properties list, including inherited fields. It supports fixed-array members with bracketed indices and accepts the parenthesized index spelling on import. Omitted members retain their existing values. Field offsets are checked without multiplication overflow before accessing memory. These shared import/export improvements also apply to the existing UE1 property-text paths.

UStrProperty text output escapes quotes and backslashes; import reverses those escapes while preserving unknown backslash sequences such as ordinary Windows paths. This provides nested-string round trips without splitting embedded commas or parentheses.

PoA class config initialization now uses the reflected struct importer, so nested and partial config values preserve serialized defaults instead of clearing omitted members. UE1 keeps its existing class-config initialization branch. PoA config saving supports scalar/fixed-array/nested structs only when every member has a supported round-trippable scalar or struct type. Dynamic array/map/other unsupported members cause that config property to be skipped, preserving the existing INI value.

## Validation

- Two compiled ASan/UBSan harnesses exercise production property parse/import/export methods and the config-export eligibility guard with reflected metadata test doubles.
- Checks cover nested structs, quoted commas/parentheses, escaped quotes/backslashes, fixed-array import and export, partial defaults, duplicate case-insensitive keys, malformed values, overflow/out-of-bounds rejection, unsupported member types and recursive metadata.
- All 44 struct-style values identified in the original bundled EU INI files parse with the production parser. This checks syntax consumption, not complete object construction from every value.
- Host C++20 syntax checks pass for UProperty.cpp, UStrProperty.cpp, UStructProperty.cpp and UClass.cpp with engine headers.
- Host suite: 113 tests run, 112 passed, one optional original-data regression skipped.
- The canonical follow-up patch reproduces the four edited engine files and new parser header from the retained baseline.

These checks do not execute the full production VM, Android UI, renderer or game. Dynamic-array/map config text, native asset decoding, mesh animation, terrain and gameplay acceptance remain open. The earlier five-native extension has passed its ARM64 build; this property-text follow-up requires its own ARM64 and phone validation. The previous release source lock remains historical until a new full-source package is assembled.
