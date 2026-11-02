# PoA Boolean-variable wrapper checkpoint

Added bounded execution for BoolVariable around local and instance variables,
and wrapped assignment targets. Reads require actual Boolean values; writes
require Boolean values and matching destination types. Unsupported wrapper
children, missing environments and malformed operands still fail explicitly.
Wrapper evaluation consumes the shared instruction budget.

Original bytecode audit: 5,602 local/instance wrapper occurrences, 110
struct-member wrappers and 79 default-variable wrappers. The last two forms
remain pending. LetBool object/class context targets remain pending too.
These are static structural counts, not gameplay coverage.

Validation: all 44 Python tests and five C++ data-free sanitizer suites pass.
The expanded retail property corpus passes 24 getters (14 wrapped Boolean),
76 invocations on two receivers, checking both true and false with explicit
host values. The 3,576-image primitive-frame corpus passes its independent
oracles and bounded smoke probe under ASan/UBSan.

No original class defaults, native Boolean bit masks/packing, struct member
layouts or explicit Context/Self execution were implemented. Synthetic
receiver objects are not original engine actors. PoA remains disabled and
unplayable; no Android build or phone test was performed.

Next: verified default-property and struct-member access, explicit object
contexts, then state lifecycle and native behavior integration.

This archive is a cumulative source overlay. Extract over the supplied
0.4.1 source root or a later checkpoint in this conversation. No game data,
APK or signing files are included.
