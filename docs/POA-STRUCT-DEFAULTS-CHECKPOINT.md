# PoA default-rooted struct checkpoint

Adds read-only default struct-member paths, including nested members and
Boolean wrappers. Explicit primitive per-class leaf overrides use nearest
ancestor lookup. New receiver values copy those defaults, while subsequent
instance writes leave defaults and other receivers unchanged. Late default
registration, invalid schemas/types, duplicate overrides, unrelated owners
and script default writes are rejected.

Added PackageObjects to resolve checked import/export references by path and
object kind. Struct fixtures now follow imported roots/members and verify
the declared struct owns the field across packages. Missing references are
not fabricated. New tests cover cross-package field resolution, same-path
different-kind identities, absent imports, invalid references and cycles.

All 50 Python tests and eight C++ sanitizer suites pass. The expanded
instance-struct corpus passes 404 fragments (309 imported), 816 invocations.
Eight default-rooted retail-derived reads pass 16 sanitizer invocations in
synthetic return harnesses. Original whole functions, class ownership during
execution, native layouts and retail default values are not validated by
those harnesses; host metadata/values are explicit. Original cross-package
struct ownership is verified separately by fixture selection.

No original serialized class/struct default loader, actors, local/returned
struct values, arrays, object contexts or state lifecycle is implemented.
No APK or phone test was performed. PoA remains disabled and unplayable.
Evidence is in POA-STRUCT-DEFAULTS-VERIFICATION.json.

Apply this cumulative source overlay to the supplied 0.4.1 source root or
a later checkpoint in this conversation. Game data and signing files excluded.
