# PoA non-null object-context checkpoint

Adds opaque ObjectRef values, Self/None expressions and non-null Context
reads/calls to the isolated receiver interpreter. Object handles cannot
alias int32 values. Known-handle checks apply to object arguments/results
and property writes; host defaults must be null. Registry identities are
not native UObjects or game actors.

Context access temporarily selects another receiver. A function's Self
remains stable while its expression context changes; a target method gets
the target as Self. Arguments evaluate in the caller's frame scope, with
the target restored before invocation. Context/frame stacks unwind on
exceptions, and shared execution/depth limits remain in force.

Explicit context bindings allow another class's primitive field references
while checking the actual access receiver's ownership. Called methods can
mutate that target's fields. Direct Context assignment targets, class
contexts and null-context recovery are not implemented. Null access throws
instead of synthesizing a successful result. Generic object references
lack original class constraints, destruction and garbage collection.

Retail fixture selection verifies original object-property declared target
class ancestry and primitive leaf layouts across checked packages. It
executes extracted Context/property bytes in synthetic return harnesses
with host receivers/values, not whole original functions or original actors.
2,117 selected fragments pass 4,890 invocations under ASan/UBSan. All 50
Python tests and nine C++ sanitizer suites pass, along with the existing
404 instance-struct and eight default-struct fragment corpora and the
3,576-image typed-frame regression. Smoke success is not gameplay coverage.
Details are in POA-OBJECT-CONTEXT-VERIFICATION.json.

Serialized defaults, actual actor creation/lifetime, native object behavior,
state lifecycle and Android integration remain pending. No APK or phone
test was performed. PoA is still disabled and unplayable.

Apply this cumulative source overlay to the supplied 0.4.1 source root or
a later checkpoint. No game data, APK or signing files are included.
