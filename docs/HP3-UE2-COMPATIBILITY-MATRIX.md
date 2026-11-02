# HP3 (Prisoner of Azkaban) — UE1 vs UE2 compatibility matrix

Status: **design only**. HP3 remains disabled/experimental. No APK, first frame, or gameplay support is claimed. This matrix tracks engine-level differences between the UE1 runtime used for HP1/HP2 and the UE2 runtime required for HP3, so the port can add an explicit UE2 profile instead of overloading UE1 paths.

## Version and identity

| Item | HP1 / HP2 | HP3 |
| --- | --- | --- |
| Engine | Unreal Engine 1 (KnowWonder) | Unreal Engine 2 |
| Package magic | `0x9E2A83C1` | `0x9E2A83C1` (same magic, different version/licensee) |
| Package version (observed) | 61–68 | 129 (PoA) |
| Licensee version | KnowWonder | PoA-specific |
| Identification rule | Executable SHA1 + package version | Executable `hppoa.exe` (SHA-256 `1ac9c1e3...`) + package version 129 + file set; version 129 alone is insufficient |

## Package format

| Area | UE1 (HP1/HP2) | UE2 (HP3) | Port action |
| --- | --- | --- | --- |
| Name table | `FName` array with flags | Same shape, different flags/indices | Reuse reader, validate flags |
| Export table | `UObject` exports with class/outer/name | Same shape, more fields | Reuse reader, add fields |
| Import table | package/object references | Same shape | Reuse reader |
| Depends table | Optional | Required for UE2 | Add depends parsing |
| Bulk data / lazy arrays | Limited | Expanded | Add bounded reader |
| Compression | None typical | Often zlib/lzo | Add decompression path |

## Metadata and objects

| Area | UE1 | UE2 | Port action |
| --- | --- | --- | --- |
| UClass layout | UE1 fields | Additional UE2 fields (interfaces, etc.) | Profile-specific decode |
| UProperty | UE1 property types | More property types, arrays, structs | Extend property enum |
| UFunction | UE1 bytecode | UE2 bytecode version | VM version handling |
| UState | UE1 state labels | UE2 state tables | Extend state decoder |
| Defaults | Serialized class defaults | Same concept, different layout | Validate layouts first |
| Object factory | `Package::NewObject` | Reuse | One factory, profile gate |

## Script VM

| Area | UE1 | UE2 | Port action |
| --- | --- | --- | --- |
| Bytecode version | UE1 | UE2 | Version-aware decoder |
| Native function set | UE1 + KnowWonder | UE2 + PoA natives | Map & verify each native |
| Latent actions / state | UE1 semantics | UE2 semantics | Validate separately |
| Calling convention | UE1 | UE2 (out/optional args) | Preserve caller Self vs context |

## Rendering / animation / audio / physics

| Area | UE1 | UE2 | Port action |
| --- | --- | --- | --- |
| Materials | UE1 materials | UE2 materials (more shader types) | Add adapters |
| Skeletal animation | Limited | Full skeletal meshes | New animation path |
| Particles | ParticleFX | UE2 emitter system | New particle path |
| Collision | UE1 collision | UE2 collision (oval cylinders, etc.) | Verify against retail |
| Audio | OpenAL/OpenSL | Same backend, different assets | Reuse backend |

## Android integration

| Item | Status |
| --- | --- |
| Launcher entry | Disabled/experimental |
| Build flag | `HP3_ENABLED` (default off) — to be added in Batch C |
| First scene | Not achieved |
| Gameplay | Not achieved |

## Acceptance gates (do not tick without on-device proof)

- [ ] UE2 package headers parse from original disc with rejection of corrupt/missing.
- [ ] First map loads through production path.
- [ ] First frame renders on Android.
- [ ] Input/camera functional.
- [ ] Save/load functional.
- [ ] Wider compatibility confirmed.

## References

- `docs/HP3-POA-UE2-FULL-COMPATIBILITY-ROADMAP.md` (master backlog)
- `docs/POA-PRODUCTION-ARCHITECTURE.md` (architecture decision)
- `native-runtime/docs/re/hp3/` (implementation notes)
