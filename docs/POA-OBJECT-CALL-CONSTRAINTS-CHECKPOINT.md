# PoA object argument/return constraint checkpoint

Adds optional class constraints to Object-valued argument and return slots.
Exact classes, subclasses and null are accepted. Incompatible arguments fail
before script execution, including nested final/named calls. Returned objects
are checked after execution. Optional omitted objects default to null.
Missing validators and unknown class/receiver IDs fail explicitly. Constraints
on non-object slots and unsupported constrained locals are rejected.

Return errors do not roll back earlier successful script side effects.
Object lifetimes, serialized defaults, native frame ABI and actual actors
are not implemented. Generic unconstrained slots retain their prior behavior.
Existing typed fixture files remain compatible through a zero default for
the new class-constraint field.

The metadata corpus includes 2,891 slots (2,667 arguments, 224 returns),
verified with synthetic identity functions against original property/class
metadata. Checks pass for 2,891 exact classes, 2,252 available subclasses,
2,891 nulls, 142 omitted optional arguments and 2,855 unrelated rejections.
Original containing game functions are not executed by this corpus.

Return properties commonly include an output flag. The fixture selector now
distinguishes return slots from unsupported out/lazy arguments, with Python
regressions for this distinction. Unsupported layouts/imports remain deferred.

All 54 Python tests and eleven C++ sanitizer suites pass. The existing
1,290-schema property constraint corpus and 3,576-image typed-frame
regression also pass under ASan/UBSan.

No APK or phone test was performed. PoA is disabled and unplayable. Actor
lifetime, state lifecycle, serialized defaults and Android integration remain.
Evidence is in POA-OBJECT-CALL-CONSTRAINTS-VERIFICATION.json.

Apply this cumulative source overlay to the supplied 0.4.1 source root or
a later checkpoint. Game data, signing files and APK are excluded.
