# Production package table validation checkpoint

4 November 2026. Continues the v0.4.5 compatibility branch, including the launcher/control fixes. Source changes only; no new APK or live PoA startup is claimed.

## Implemented in the production loader

`Package::ReadTables` now validates signed name/import/export/generation counts and minimum table extents before iterating. Nonempty tables must begin after the parsed summary and fit the file. Generation records are bounded; legacy heritage summaries retain their existing path.

Names, class/super/outer references and import identities are checked against their owning package's serialized tables before object construction. This validation uses the original serialized counts, before synthetic native exports are appended. Table regions must be disjoint. Nonempty export payloads must fit the file and must not overlap the parsed summary or any serialized table.

Local outer chains are checked in linear time, including chains crossing import/export tables. Self-cycles, mixed cycles and excessive nesting fail with the package/reference context. Zero/null object references remain valid. No cross-package object resolution or real class construction is claimed by these checks.

`PackageStream` validates reads, seeks and skips against the file size captured when the read stream opens. Caching that size avoids a file-size query for each operand byte. The underlying file reader remains responsible for rejecting a truncated physical read if a file changes after opening.

`OpenObjectStream` rejects invalid table indices, negative sizes and out-of-file ranges before allocation, and performs allocation-size arithmetic in 64 bits. Zero-payload native exports still open without requiring a physical backing file.

The existing version gate is unchanged. This does not establish support for every version/licensee combination it admits, compression, resource variants, UE2 metadata/native layouts or bytecode execution.

## Verification

`python3 -m unittest discover -s native-runtime/tools/poa -p 'test_*.py'`

- 95 Python regressions, including the new production table test.
- The new test compiles actual `Package::ReadTables`, `Package::OpenObjectStream` and the reader half of `PackageStream.cpp` with an in-memory File implementation and table containers. It runs 100 cases under AddressSanitizer and UndefinedBehaviorSanitizer.
- Valid synthetic version-67 and version-69 UE1 packages and version-129 PoA-format packages load through these exact method bodies; UTF-16 table names and zero-payload native exports are covered.
- Malformed summary counts/offsets, truncated headers, bad name/object references, overlapping table/payload regions, outer cycles, excessive nesting, negative/oversized payloads and stream bounds fail explicitly.
- The existing launcher/control host runner also passes, including 22 Java path/preflight/timing checks and native executable detection with sanitizers.

These tests substitute only the surrounding file/table environment. They do not compile the full engine, decode original class/default/resource payloads, resolve a complete original dependency graph or render a scene.

## Remaining acceptance

PKG-02/03/04 advance at source/host-test level; they remain open. Limits of one million records per table and 1,024 outer levels require validation against original assets; counts of test cases are not a completion percentage.

Next work remains original PoA summary/table/metadata verification through the production path, then the required Class/Struct/State/Function/default/native adaptations. Android build/signing, both existing games' regressions and G0–G6 remain open. The full 21-workstream roadmap remains authoritative.
