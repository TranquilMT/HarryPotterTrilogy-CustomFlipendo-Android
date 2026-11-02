# HP3 / UE2 implementation notes

These notes guide the incremental UE2 compatibility work. HP3 remains disabled.
See `docs/HP3-UE2-COMPATIBILITY-MATRIX.md` and `docs/HP3-POA-UE2-FULL-COMPATIBILITY-ROADMAP.md`.

## Module layout (planned)

- `src/hp3/` — HP3-specific game layer (Batch C), mirroring `src/hp1/` and `src/hp2/`, guarded by `HP3_ENABLED` (default off).
- `src/compat/` — shared UE1/UE2 compatibility shims (existing).
- Engine changes go through the production profile path (see `docs/POA-PRODUCTION-ARCHITECTURE.md`), never a parallel actor system.

## UE2 package reading (Batch B)

- Validate magic `0x9E2A83C1` plus version/licensee; version 129 alone is not game identification.
- Parse name/export/import/depends tables with bounded reads; reject corrupt/missing.
- Produce `docs/POA-PACKAGE-INTEGRITY-VERIFICATION.json`-style evidence.

## UE2 runtime (Batch C/D)

- `HP3_ENABLED` compile-time guard, default off; launcher entry stays disabled.
- Native functions: map and verify before use; unsupported natives fail closed with identities.
- Bytecode VM: version-aware decode; validate against retail packages.
- No APK/playability claims until a real PoA startup test passes.
