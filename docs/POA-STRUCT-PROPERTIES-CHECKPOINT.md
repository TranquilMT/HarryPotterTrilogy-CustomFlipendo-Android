# PoA primitive instance struct-member checkpoint

Completed isolated StructMember reads/writes for explicitly declared scalar
leaves rooted in instance properties. Nested paths, Boolean wrappers,
inherited schemas, caller-local/imported bindings and nested scalar calls
are supported. The store separates receiver values and checks class ownership,
types, byte bounds and schema/path identity conflicts. Path depth is bounded
and access consumes the shared instruction budget.

Synthetic tests cover nested float reads, primitive/Boolean writes, inherited
defaults, per-receiver values, imported refs, nested calls, byte/type failures,
unsupported roots, missing bindings and primitive/struct alias conflicts.

Retail fixture selection verifies the StructProperty's declared Struct is
the member's owner, both dimensions are one and the leaf is a supported
primitive. 95 distinct original struct-read fragments run in synthetic
return harnesses: 198 invocations using two receivers and explicit host
values. Whole original functions, original struct values/defaults and native
memory layouts are not verified. Generated game-derived fixtures are excluded.

Remaining: actual serialized class/struct defaults, local/default/returned
struct values, arrays, object/class contexts, actor allocation and state
lifecycle/native behavior. No APK or phone test was performed. PoA is still
disabled and unplayable. Verification is in the accompanying JSON.

Apply this cumulative overlay over the supplied 0.4.1 source root or any
later checkpoint in this conversation. Source only; game data, signing
files and unchanged HD tooling are excluded.
