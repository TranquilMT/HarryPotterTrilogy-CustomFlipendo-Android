# 0.4.3 — Complete local source checkpoint

Version name: **0.4.3**. Version code reserved/defaulted for the next build: **44**. Package: `com.tranquil.hp1native`.

This is the complete local source, including the canonical 0.4.2 HP2 menu clipping correction, the refreshed saved engine patch and all subsequent isolated PoA compatibility work. It supersedes the need to combine the 0.4.1 base ZIP with checkpoint recovery overlays for source extraction.

## Included

- Complete current project source, native runtime, patched vendored SurrealEngine and routed patch set.
- Vendored SDL dependency and other engine dependencies present in the source checkpoint.
- HP1/HP2 Android launcher, installer, build and packaging code; existing production fixes preserved.
- Cumulative PoA readers, isolated runtime tests, configuration-driven profile generation, original-data identity evidence and architecture/route decisions.
- Master PoA checklist, project roadmap, historical verification reports and complete source hash lock.
- `VERSION.json`, Android manifest version metadata and current Linux builder defaults set to 0.4.3 / code 44. Older historical build utilities and reports retain their original versions.

## Extract and verify

Extract `Harry-Potter-TQ-0.4.3-Full-Source.zip` into a new directory. Its contents are the project root; no previous source ZIP or checkpoint overlay is needed.

From that root, with Python 3.11 or later:

```sh
python3 native-runtime/tools/poa/verify_source_lock.py
python3 -m unittest discover -s native-runtime/tools/poa -p 'test_*.py'
```

`docs/POA-SOURCE-LOCK.json` records the complete included source file hashes; the lock excludes itself to avoid a circular hash. The accompanying SHA256 text file identifies the archive bytes.

## Build and compatibility limits

No 0.4.3 APK was built or signed by creating this archive. Building still requires the Android SDK/NDK/JDK and build tools, original permanent signing material, previous signed APK, and complete HP1/HP2 original game payload inputs. The Linux builder accepts the verified source-lock checkpoint when Git history is absent; it rejects modified or missing locked files. The original signing key has been recovered and its certificate verified privately. SDK/NDK/compiler tools, previous APK and full payload inputs remain unprepared. These build/release acceptance gaps remain open under BASE-03/G0.

The source ZIP contains no game payloads, ISOs, HD texture packs, private signing keys, passwords or compiled test executables. Those exclusions do not remove source needed for the engine; game data and private release inputs remain separate.

PoA remains disabled and unplayable. Offline inspection and isolated tests do not establish live UE2 loading, rendering, controls, saving or gameplay. All remaining compatibility work and release gates are in the included master checklist.

The prior 0.4.2 release information is retained in README and its historical release note. Local source 0.4.3 is not published as a new GitHub release by this packaging task.

## Subsequent local 0.4.3 work

Complete disc/archive metadata inventory (841 records), safe inventory-path checks and read-only Android build preflight are now included. The standalone source-archive build-info dependency on Git history is fixed with verified checkpoint fallback. See [inventory and build input evidence](POA-INVENTORY-AND-BUILD-INPUTS.md). Host checks: 73 PoA Python tests and 18 Android packaging/preflight tests pass. No APK or live PoA result is claimed.

Further local data work adds package-integrity verification and static dependency tracing, with ten more original assets CRC-checked/hashed/table-inspected (24 of 110 package payloads). The source contains metadata evidence only, not those asset bytes. See [payload/dependency evidence](POA-PAYLOAD-INTEGRITY-AND-DEPENDENCIES.md). Current checks: 83 PoA Python tests and 18 Android packaging/preflight tests pass; live compatibility remains unverified.
