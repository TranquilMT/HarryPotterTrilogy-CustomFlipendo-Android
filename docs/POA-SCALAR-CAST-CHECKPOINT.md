# PoA scalar primitive-cast bridge

5 November 2026. Continues the v0.4.5 compatibility branch. Source implementation and host tests; no new APK or live PoA acceptance.

## Change

PoA's UE2 primitive cast uses opcode `0x39`, a one-byte conversion code, then an operand expression. The production loader previously rejected this prefix. It now preserves both bytes while expanding compact references in the operand. The expression-tree builder recognizes the prefix and selects the existing scalar conversion expression corresponding to the second byte.

Only the following 12 conversions are connected:

| From | To | Cast code |
| --- | --- | --- |
| Byte | Int | 0x3A |
| Byte | Bool | 0x3B |
| Byte | Float | 0x3C |
| Int | Byte | 0x3D |
| Int | Bool | 0x3E |
| Int | Float | 0x3F |
| Bool | Byte | 0x40 |
| Bool | Int | 0x41 |
| Bool | Float | 0x42 |
| Float | Byte | 0x43 |
| Float | Int | 0x44 |
| Float | Bool | 0x45 |

The prefix is not removed from the decoded script: statement and operand offsets retain their original memory positions. Normalization happens only when selecting the existing production expression node. Nested casts and following statements therefore do not shift because of a shortened opcode.

Non-scalar casts (including object, string, vector, rotator, interface and delegate variants) remain unsupported by this bridge. Unknown cast codes, truncated casts and bare UE2 opcodes overlapping UE1 conversions fail explicitly. Legacy UE1 conversion tokens bypass the new prefix handling and retain their original size.

## Sources

The project's retained `BytecodeReader.h`/`bytecode.py` already deserialize the prefix and one-byte code. Scalar conversion identities were checked against UELib at commit `8236c39b79e30887a487ecf9b018204e430a5ebc`:

- [Cast token enum](https://github.com/EliotVU/Unreal-Library/blob/8236c39b79e30887a487ecf9b018204e430a5ebc/src/UnrealTokens.cs)
- [PrimitiveCastToken deserialization](https://github.com/EliotVU/Unreal-Library/blob/8236c39b79e30887a487ecf9b018204e430a5ebc/src/Core/Tokens/CastTokens.cs)

This source agreement establishes encoding/type identity for the bridge. It does not establish retail PoA numeric edge-case behaviour or native ABI compatibility. No original PoA payload was recovered from the matching retained-file search during this batch, so original-data validation remains open.

## Tests

`python3 -m unittest discover -s native-runtime/tools/poa -p 'test_*.py'`

- All 97 Python regressions pass.
- The actual UStruct token-reader suite now runs 88 sanitizer cases; accepted scalar casts and a nested cast match the retained independent decoder byte-for-byte.
- A new 312-case sanitizer harness compiles the shared prefix bridge and the exact existing scalar/constant AST-construction cases from production Bytecode.cpp. It checks all 12 selections, nested operands, following-statement offsets, unsupported codes, missing operands, native-token noninterference and legacy UE1 forms.
- The earlier 100-case actual-production package-table test still passes.

The AST harness substitutes surrounding stream and node-allocation interfaces. It does not compile the entire Bytecode class or execute its numerical evaluators; those evaluator implementations are reused, not rewritten. Full engine compilation, original-value comparisons and Android gameplay remain unverified.

## Remaining scope

PKG-06 and VM decoding/expression integration advance at source/host-test level. The remaining Class/State/default/property/native/resource layouts and bindings are still required. Other UE2 opcodes and non-scalar casts need separate implementation/evidence. G0–G6 and the full 21-workstream scope remain open.
