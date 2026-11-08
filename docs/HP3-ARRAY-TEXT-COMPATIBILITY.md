# UE2 dynamic array text and bounded replacement

Source follow-up, 8 November 2026. Published v0.4.14 does not contain this batch.

Dynamic reflected arrays now import and export parenthesized element lists, including nested arrays, supported structs, quoted strings, names and primitive values. The shared property tokenizer preserves commas inside nested values and quoted text. PoA struct config export now permits supported array members. Object references, maps and delegates remain excluded to avoid saving text that cannot be read back.

Text import constructs a replacement array before swapping it into the destination. Binary array reads likewise preserve the destination if a count, element read, fixed index or tagged size is invalid. Negative and impossible counts are rejected before allocation; tagged reads require an exact payload size. Stream position is not rolled back when a read fails.

Validation: the host suite ran 113 tests with 112 passing and one skipped. The new compiled ASan/UBSan harness executes production array method bodies with stream, storage and reflection doubles. It covers nested array/string round trips, malformed text, unsupported/cyclic metadata, negative/impossible/truncated counts, tagged size mismatch, invalid fixed indexes, empty arrays and successful tagged/untagged replacement. Native UArrayProperty.cpp and UClass.cpp also pass host syntax checks. This is not an Android gameplay or full object-loader integration test.

Still open: repeated-key top-level dynamic-array INI handling, map/object/delegate text import, complete UE2 mesh/animation/rendering compatibility, ARM64 validation of this batch and phone launch/gameplay validation. No claim that HP3 launches or is playable.

## String/name follow-up

Legacy UStringProperty previously exported quotes but imported them literally, unlike UStrProperty. Both now share PropertyText escaping/unquoting. UNameProperty now quotes delimiter-containing or empty names and unquotes imported names; normal names such as Engine.Actor retain their output. Production method-body tests cover both property types, including list tokenization. This fixes a round-trip gap in the preceding array capability whitelist; it does not establish UE2 gameplay compatibility.
