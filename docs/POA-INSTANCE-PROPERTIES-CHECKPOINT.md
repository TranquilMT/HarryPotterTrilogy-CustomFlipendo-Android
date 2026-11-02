# PoA primitive instance-property checkpoint

Extends the isolated receiver path with primitive property schemas, inherited
host-supplied defaults and separate mutable property maps for each receiver.
ScalarVM reads InstanceVariable and assigns bare instance/local targets.
TypedProgram carries property callbacks through nested calls, preserving
package-local reference bindings and strict ownership/type checks.

Supported values: Boolean, int32, float32 and range-checked bytes. Local
variables do not alias instance fields. Schemas must be registered before
creating receivers; missing links and unsupported accesses throw. Failed
target writes do not change the target; complete calls are not transactions.

44 Python tests and all data-free C++ sanitizer regressions pass, including
the new persistence, receiver separation, inherited defaults, type/range,
local separation and nested-call tests. Ten original retail scalar getters
execute under ASan/UBSan against explicit host property values on two
receivers. Fixture selection verifies the return/property type and excludes
imported/unverified property layouts. Game package bytes and generated
fixtures are not distributed in this update.

No original class-default deserialization, packed Boolean wrapper/mask
handling, arrays, structs, object values, Context/Self or live actor allocation
is added. State lifecycle and native behavior remain pending.
No Android build or gameplay claim: PoA remains disabled and unplayable.

Apply this cumulative overlay to the supplied 0.4.1 source root or any
later checkpoint in this conversation. It is source only, not an APK.
