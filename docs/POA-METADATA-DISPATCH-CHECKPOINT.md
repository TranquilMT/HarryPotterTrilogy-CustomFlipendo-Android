# PoA metadata dispatch checkpoint

Continues the named-call source checkpoint, based on the supplied 0.4.1 source.

Added src/compat/ue2/DispatchResolver.h and tools/poa/test_dispatch_resolver.cpp.
Updated the roadmap and PoA tooling notes. No production engine files, Android
version, signing, saves or launcher availability changed.

Verified with strict C++17 warnings and ASan/UBSan: state inheritance and
overrides, inherited class functions, global calls bypassing states, ASCII
case matching, package-local name tables, missing functions/parents, cyclic
class/state ancestry, duplicate definitions, truncated call operands and
transactional failure. Linked synthetic functions execute through TypedProgram.
The previous sanitizer named-call regression and 17 Python tests also pass.

No retail corpus or Android launch test was run. This model does not deserialize
retail classes/states or validate state ownership. Dynamic receivers, object
contexts, live state transitions and world integration remain pending.
PoA remains disabled and unplayable. Archive contains source, not game ISOs
or an APK. Next: retail metadata extraction and independently checked
receiver/state ownership before live dispatch integration.
