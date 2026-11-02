# PoA host receiver-retirement checkpoint

Adds explicit host-only retirement of isolated receiver identities and struct
field storage. Retired identities cannot be reused in the same registry.
Invocations, object arguments/results and field/context reads reject stale
handles. Other owners' fields remain stale until explicitly replaced; they
are not silently cleared. Assigning null or a valid live handle can recover
such a field. Schema/default registration remains frozen after the first
receiver, including when every receiver has retired.

Retirement is permitted only between invocations in this single-threaded
model. An invocation guard unwinds on failure. No Unreal Destroy native,
pending-kill flags, event lifecycle, GC, world removal, save/load semantics
or original actor allocation is added. Handles remain registry-local.

Synthetic tests cover stale field/context reads, retired calls/arguments,
identity reuse rejection, repeated/unknown retirement, explicit clearing,
new valid receivers, struct storage cleanup and permanent schema/default
freeze. The metadata property corpus now tests stale-field rejection after
retiring targets. Original destruction functions and game actors are not run.
All 54 Python tests and twelve C++ sanitizer suites pass. The 1,290-schema
property corpus rejects 1,290 stale field reads and permits explicit clearing.
The 2,117-fragment context regression also passes 4,890 invocations.
Verification results are in POA-RECEIVER-LIFETIME-VERIFICATION.json.

No APK/phone test was performed. PoA remains disabled and unplayable.
Actual actor lifetime, serialized defaults, state lifecycle/native behavior
and Android integration are still pending.

Apply this cumulative overlay to the supplied 0.4.1 source root or a later
checkpoint. Game data, signing files and APK are excluded.
