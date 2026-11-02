# PoA class-constrained object-property checkpoint

Adds explicit target-class constraints to isolated object fields. Writes
accept null, exact target classes and subclasses; unknown receiver handles
or unrelated classes fail before replacing the field. Inherited owners and
nested setters preserve the constraint. Generic ObjectRef fields remain
explicitly unconstrained for compatibility. Registration rejects missing
target classes and constraints on non-object fields.

Synthetic tests check exact/subclass/null writes, preserved values after
failed writes, inherited owners, nested calls, unknown handles and generic
compatibility. Original metadata selection resolves 1,290 class-owned
ObjectProperty schemas against the checked 1,633-class graph. Synthetic
setters pass 1,290 exact-class, 890 available subclass and 1,290 null
assignments, and reject 1,286 unrelated classes under ASan/UBSan.

All 50 Python tests and ten C++ sanitizer suites pass. The 2,117-fragment
context corpus also passes 4,890 invocations with constrained reference fields.

These tests use original property/class metadata with supplied receiver
identities. They do not execute whole original setters, instantiate native
actors, deserialize defaults or establish native ABI/gameplay equivalence.
Function argument/return class constraints, casts, class objects, null-context
recovery, actor lifetime, state lifecycle and Android integration are pending.
No APK/phone test was performed. PoA remains disabled and unplayable.

Apply this cumulative overlay to the supplied 0.4.1 source root or a later
checkpoint. No game data, signing files or APK are included.
