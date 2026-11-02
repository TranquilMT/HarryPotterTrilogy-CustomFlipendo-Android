# PoA primitive default-property checkpoint

Added DefaultVariable reads and Boolean wrappers around default variables.
TypedProgram propagates an independent default-read callback through nested
calls. ReceiverProgram supplies explicit typed class overrides, selects the
nearest inherited override and copies defaults into new receiver fields.
Mutable fields and read-only script defaults are separate. Default writes
throw; unknown links, bad ownership, duplicate overrides and invalid values
are rejected. Host defaults are frozen once a receiver is created.

Synthetic tests verify base/derived defaults, nearest override, inherited
Boolean values, instance/default separation, independent receivers, nested
and imported references, missing environments and error handling.
All 44 Python tests and six C++ sanitizer suites pass.

No complete direct scalar default getters were found in the checked corpus.
Instead, 95 distinct original default-read instruction fragments execute in
synthetic return harnesses: 240 invocations test two receiver identities and
both Boolean values. This does not execute their whole original functions,
load their real defaults or prove native default-object behavior. Existing
24 retail getters also pass 76 invocations with host-supplied test values.

Remaining: serialized class-default loading, validated native/packed layouts,
struct members, actual object/class contexts, default-write semantics, actors
and state lifecycle. No APK or phone test was performed. PoA is still disabled
and unplayable. See POA-DEFAULT-PROPERTIES-VERIFICATION.json for evidence.

Apply this cumulative overlay to the supplied 0.4.1 source root or a later
checkpoint in this conversation. Source only; no game data/signing files.
