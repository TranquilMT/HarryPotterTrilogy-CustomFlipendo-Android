# PoA named-call source checkpoint

Base: Harry-Potter-TQ-0.4.1-Both-Games-Full-Source.zip supplied on 31 October 2026.

Changed ScalarVM.h and TypedProgram.h; added test_named_calls.cpp and updated
the PoA tooling notes and project roadmap. No release version, signing,
HP1/HP2 production runtime, or launcher availability changes.

Validation: 17 Python unit tests passed; the existing typed-program corpus
harness compiled with strict warnings. New data-free named-call tests passed
with strict warnings, AddressSanitizer and UndefinedBehaviorSanitizer.
The retail corpus and Android build were not run: the supplied archive does
not include PoA retail packages or a ready Android build environment.

Next: metadata-backed receiver/class/state linking, object/property access,
state execution and native dependencies before world/rendering integration.
PoA remains disabled and not playable. This source archive contains no game
ISOs and is not an installable APK.
