# PoA retail metadata checkpoint

Continues the metadata-dispatch source checkpoint.

Added package-table metadata extraction, malformed/ownership/inheritance tests,
fixture generation and a C++ retail-table corpus harness.

Retrieved Core.u and Engine.u from compressed.zip on the original uploaded
HPPoA_Na.bin, stripping Mode 1 raw-sector headers. Verified each ZIP entry's
uncompressed length and CRC32; SHA256 values are in the verification JSON.

Validated 240 classes, 65 states and 2,151 functions. The C++ resolver agreed
with Python graph lookup on 247,065 resolved calls and rejected 23 absent
lookups under AddressSanitizer and UndefinedBehaviorSanitizer.
All 31 Python tests pass. No game scripts were executed in the corpus test.

HGame and other packages remain unchecked. Class defaults and State payloads
are not deserialized. Original runtime equivalence, dynamic receivers, live
state transitions and world integration remain pending.

PoA remains disabled and unplayable. No Android build was produced. This
archive contains source, excludes extracted retail data, and is not an APK.
Next: HGame table checks and metadata-backed receiver/state ownership before
connecting dispatch to live object instances.
