# Prisoner of Azkaban compatibility tooling

## Current EU baseline

The active profile generator uses `reference-profile/hp3-pc-eu.json`, derived from
the supplied EU installation bundled in 0.4.9. It verifies executable and script
hashes before assigning the EU identity. The earlier North American profile and
the historical checkpoints below remain available as separate inspection data.

The EU installation has 11 script packages. It supplies `KWGame.dll` without
`KWGame.u`. Startup inspection records `KWGame.KWConsole` as an external native
dependency whose Android implementation is still pending. The launcher prepares
`UnrealEd.ini` from this installation's `DefUnrealEd.ini` before native startup.
See `docs/POA-EU-EDITION-AUDIT.md` in the repository root.


This directory adds UE2 package-table inspection and a non-playable HP3 profile.
It does not change the runtime version gate or claim UE2 gameplay support.

Run from the repository root:

```
python3 tools/poa/inspect_packages.py /path/to/hp3-game /path/to/audit
python3 tools/poa/build_profile.py /path/to/hp3-game /path/to/audit /path/to/profile
python3 -m unittest discover -s tools/poa -p 'test_*.py'
```

The uploaded North American PC disc was checked: 110 packages parse without
table errors, most using version 129. The runtime's Package.cpp accepts only
versions 60–99. A runtime loader must add version-specific object layouts,
the UE2 VM/native interfaces and dependent systems before this profile can
be made playable. Never merely raise the accepted version range.

Profiles preserve original command strings from defuser.ini, including the
context-sensitive jump/wing-flap and interact/cutscene/possession commands.
Touch proposals are data for an eventual adapter, not working controls.

Game files remain user-provided and are not included in this tooling.

## Object-reader milestone

`object_payloads.py GAME OUTPUT` reads tagged-property prefixes and HP3
function headers. It distinguishes a declared bytecode memory size from
serialized bytes: compact object indices expand to four bytes in bytecode.
Only verified NativeParm/Nothing stubs are walked; ordinary script bodies
are explicitly pending. Classes and objects with state frames are excluded.

`src/compat/ue2/ObjectReader.h` implements the corresponding bounded C++
readers. It is not yet connected to live object allocation or the VM.
Run the real-object fixture test from the repository root:

```
python3 tools/poa/make_object_fixtures.py GAME fixtures.bin
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_object_reader.cpp -o test-reader
ASAN_OPTIONS=detect_leaks=0 ./test-reader fixtures.bin
```

The native-candidate audit searches existing registration declarations and
patch additions. A matching name/index is not a verified signature or working
implementation. Conditional registrations require runtime profile analysis.
No generated native stubs are substituted for missing game behavior.

## Bytecode and isolated execution milestone

`bytecode.py GAME OUTPUT` reads every Function export's ordinary UE2 script,
expands compact references to four-byte VM indices, records serialized/VM
offsets, and checks absolute jump targets against top-level statements. It
checks the complete function tail and requires the exact export end. This
does not resolve calls, instantiate actors or execute ordinary game behavior.

The C++ equivalent is src/compat/ue2/BytecodeReader.h. Test the entire uploaded
game's function corpus, including expanded bytes and each token's offset map:

```
python3 tools/poa/make_bytecode_fixtures.py GAME bytecode-fixtures.bin
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_bytecode_reader.cpp -o test-bytecode
ASAN_OPTIONS=detect_leaks=0 ./test-bytecode bytecode-fixtures.bin
```

ScalarVM.h implements only returns, scalar constants, caller-provided local
slots, local assignments, stop, unconditional jumps and bool branches. It
has an instruction budget and rejects unsupported execution semantics and
uninitialized locals. It is an isolated compatibility test component; the
production game VM does not call it. Calls, actors, instance properties,
containers, delegates, casts and world systems are still unimplemented here.
The test executes constant-return functions and original empty functions
from the disc, plus synthetic local/branch/failure cases. It does not call
unimplemented natives or fabricate a successful game session.

`function_schemas.py GAME OUTPUT` reads owned property chains, argument flags,
types, locals and return metadata for every function. For native stubs it
verifies the native parameter references match schema argument order. The
native platform ABI and candidate C++ handlers remain unverified.

## Typed calls milestone

TypedProgram.h builds strict primitive frames and supports explicit final-call
links, imported-call links and a shared instruction budget/call-depth limit.
Locals and omitted optional primitive arguments have declared zero defaults.
Arguments, assignments and results are type-checked; bytes are range-checked.
An explicit successful Nothing return uses the declared default result, matching
the existing VM's Frame::CallFunction rule. Missing implementations still throw.

An HP3-only whitelist implements 11 Core arithmetic/comparison intrinsics.
Each registration checks its original qualified name, index, primitive argument
types and return type. This does not claim compatibility for the existing
engine's native platform ABI or other native handlers. Integer arithmetic has
explicit 32-bit wrap semantics; floating operations use float32 values.

Generate and run the typed call corpus:

```
python3 tools/poa/make_typed_fixtures.py GAME AUDIT typed-fixtures.bin
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_typed_program.cpp -o test-typed
ASAN_OPTIONS=detect_leaks=0 ./test-typed typed-fixtures.bin smoke.tsv
```

AUDIT is the directory containing function-schemas.json. Layouts with objects,
strings, names, structs, containers, fixed arrays, enum constraints, out or
lazy arguments are deliberately excluded. Only 11 known native functions are
registered. A resolved final-call link may target an excluded function; calls
to it fail. Virtual/global dispatch and live object receivers remain pending.

The test's oracle cases have independently expected return values. Its broader
bounded smoke probe uses supplied example arguments; a finished probe is not
evidence of correct gameplay semantics. None of this is wired into the live
Android runtime yet.


## Explicit named-call compatibility checkpoint

The isolated scalar evaluator now accepts VirtualFunction (0x1b) and
GlobalFunction (0x38) with independently supplied per-caller name-index links.
Nested arguments use the same typed frames and shared execution budget as
final calls. Unresolved links and unavailable implementations throw.

This is a linker interface, not automatic class/state lookup. A future live
linker must resolve virtual calls against the receiver/state and global calls
against the proper class scope; it must never reuse another package's name
indices. No receiver allocation, state machine, actor lifecycle, or Android
integration is added. Existing fixture files retain their original format.

Data-free regression check:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_named_calls.cpp -o test-named
ASAN_OPTIONS=detect_leaks=0 ./test-named
```

## Metadata dispatch linker checkpoint

DispatchResolver.h resolves the named-call instructions in a FunctionImage
from explicitly supplied class/state scopes. Virtual calls search the supplied
state inheritance chain before the receiver class chain; global calls search
only the receiver class chain. Names are matched with ASCII case folding;
non-ASCII identifiers fail until their folding rules are verified.

Lookup checks the complete supplied ancestry for cycles and missing parents
before resolving. Per-package name indices become canonical names before
lookup. Existing function links are replaced only after every call resolves.

This is an isolated metadata model. Retail Class/State deserialization,
import resolution, state ownership validation, receiver allocation, dynamic
state changes, explicit object contexts and live Android integration remain
unimplemented. Supplied scope ancestry and ownership are the caller's
responsibility. Tests establish the model's behavior, not retail equivalence.

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_dispatch_resolver.cpp -o test-dispatch
ASAN_OPTIONS=detect_leaks=0 ./test-dispatch
```

## Retail package-table extraction checkpoint

`dispatch_metadata.py GAME OUTPUT.json` reads actual package tables and builds
qualified class/state/function identities from outer/import references. It
validates class/state inheritance kinds and cycles, function/state owners and
state super ownership within the class ancestry. Missing imported supers
remain explicit and prevent complete fixture generation. This is table
extraction, not Class/State payload deserialization or a live object linker.

Only dispatch objects share its identity namespace: the retail Engine package
contains both a Texture and a Class named SubActionFade at package scope.
Rejecting that legal asset/class collision would prevent extraction.

Core.u and Engine.u from the uploaded North American disc were CRC32-checked
after decompression. Their tables yield 240 classes, 65 states and 2,151
functions, with no unresolved super references. HGame.u remains unchecked
in this checkpoint. The C++ corpus checks metadata-model agreement only.
Retail package bytes and generated object dumps are excluded from the source.
Verification summary: ../../../docs/POA-RETAIL-METADATA-VERIFICATION.json
(relative to this directory).

```sh
python3 tools/poa/dispatch_metadata.py GAME metadata.json
python3 tools/poa/make_dispatch_fixtures.py metadata.json dispatch.bin
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_dispatch_corpus.cpp -o test-corpus
ASAN_OPTIONS=detect_leaks=0 ./test-corpus dispatch.bin
```

## Gameplay-package call audit

`audit_dispatch_calls.py GAME OUTPUT.json` decodes every Function export and
checks named calls against the function owner's lexical class/state hierarchy.
Calls nested in object/class contexts or delegates are deferred because their
receiver can differ from self. A missing lexical target remains unresolved;
it is never replaced with an empty implementation. Dynamic receiver overrides
and active state changes are not validated by this static audit.

A single command now runs the Python regressions, C++ data-free sanitizer
checks, metadata extraction, C++ lookup corpus and strict call decode audit:

```sh
python3 tools/poa/check_compatibility.py --game GAME --output AUDIT
```

Omit both options for data-free tests. The command downloads nothing and does
not execute retail scripts or launch Android. Retail-derived dumps/fixtures
are temporary local analysis files and are excluded from source checkpoints.

Retail follow-up: Core, Engine, HGame, GUI, GamePlay and UWindow now decode
5,807 functions without errors. The audit finds 2,970 lexical targets, defers
3,490 explicit-receiver calls and leaves 642 calls needing dynamic receivers
or missing definitions. No execution compatibility is implied by those counts.

The table graph has 1,693 class/state scopes. It remains incomplete: 429
super links and 2 state-ownership checks depend on native KWGame definitions.
KWGame.dll is present in the archive; KWGame.u is absent from its central
directory. Missing native ancestry is not proof of invalid ownership.
The check runner writes audit reports and exits unsuccessfully at this gate
instead of generating a misleading complete-corpus fixture.

## KWGame extraction correction (current checkpoint)

The previous native-ancestry blocker was caused by an incomplete extraction.
System/kwGame.u is a standalone file in the disc filesystem, not an entry in
compressed.zip. With it and UDebugMenu.u included, eight checked packages
yield 1,633 classes, 536 states and 7,933 decoded functions. There are zero
unresolved metadata relationships and zero missing lexical named-call targets.
The 5,215 calls in explicit receiver contexts remain deferred. These numbers
do not establish working native behavior or live dispatch.

`disc_packages.py DISC OUTPUT.json` inventories both standalone .u files and
.u entries in ZIP archives, supporting 2048-byte ISO or 2352-byte Mode 1 BIN.
It neither runs installers nor extracts game bytes. On the uploaded disc it
finds 12 script packages: 11 archived, one standalone. Eight were checked in
this checkpoint; Decorations, AmbientCreatures, Editor and Fire remain outside
this corpus. Directory names/extents, repeated directories and record bounds
are checked; multi-extent files and other disc formats are rejected.

The original directory/volume sectors and ZIP central directory were reused
in a sparse local image to verify inventory without re-downloading the disc.
No unread file bodies are claimed verified. The standalone KWGame package was
read in full and SHA256-hashed; ZIP packages had CRC32/size verification too.

The C++ corpus count bound was increased to four million to handle over one
million lexical lookup cases; per-scope, string and function bounds remain.

## Isolated receiver-call execution checkpoint

ReceiverProgram.h gives the scalar interpreter explicit receiver identities,
class ownership and independently selected active state scopes. TypedProgram
accepts an optional CallEnvironment that validates every function entry and
resolves named calls from the current caller's name table. Final, native and
named calls share this environment and the existing call-depth/instruction
limits. A missing implementation or binding throws; static link tables are
not used as fallback when a receiver environment is supplied.

The receiver adapter checks function class ownership and the owner classes
of the complete selected state ancestry. Invalid state selection leaves the
previous selection intact. State selection changes between invocations only;
this is not BeginState/EndState, label execution, latent actions or script
state transitions. Object fields, object values and Context/Self execution
are not added. Receivers are IDs and metadata, not allocated game actors.

Data-free C++ tests execute the same script on different receiver classes,
state overrides/global bypass, nested final/native calls, separate caller
name tables, invalid ownership/bindings and shared recursion/budget failures.
Only synthetic scripts test this new receiver path. The retail metadata
corpus and existing primitive-frame corpus are separate regression checks.

check_compatibility.py now includes test_receiver_program.cpp and compiles
sanitizer checks with -O1. This keeps the million-case metadata check practical
while retaining address/undefined-behavior instrumentation.

## Primitive instance-property checkpoint

ScalarVM supports InstanceVariable (0x01) reads and bare local/instance
targets for Let (0x0f) and LetBool (0x14). Optional typed-program environment
callbacks resolve property references using the current function's package
bindings. Without a property environment, access throws explicitly.

ReceiverProgram accepts explicitly supplied primitive schemas/defaults before
receiver creation and copies inherited defaults into each receiver's own
field map. Per-function property reference links distinguish identically
numbered references from different packages and accept imported negative
references only when explicitly bound. Reads/writes enforce class ownership,
Boolean/int/float types and byte range. Invocation now mutates receiver data.
Schema registration after receiver creation is rejected to avoid partial
layouts. Failed writes leave the target value unchanged; whole invocations
are not transactional and earlier successful side effects are not rolled back.

No retail class defaults, fixed/native memory offsets, packed Boolean masks,
BoolVariable wrappers, structs, arrays, object references, Context/Self or
state lifecycle are implemented by this change. Host-supplied defaults and
receiver IDs must not be mistaken for full Unreal object allocation.

make_instance_fixtures.py selects direct scalar getters from supplied retail
packages and function schemas. test_instance_corpus.cpp executes ten such
getters from the checked package set against explicit host values on two
isolated receivers. This checks original script property-read instructions,
not original class layouts/defaults, owner allocations or native behavior.

```sh
python3 tools/poa/make_instance_fixtures.py GAME function-schemas.json getters.bin
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_instance_corpus.cpp -o test-getters
ASAN_OPTIONS=detect_leaks=0 ./test-getters getters.bin
```

check_compatibility.py includes the synthetic instance-property regression.
Retail getter fixtures need the separately produced function schemas and
are run with the commands above.

## BoolVariable wrapper checkpoint

ScalarVM now handles BoolVariable (0x2d) around local and instance variables,
including wrapped Let/LetBool assignment targets. Reads require Boolean
storage; wrapped writes require Boolean values and the destination's schema
must agree. Unsupported wrapper children throw instead of returning a default.
Native Boolean bit masks and original C++ field packing remain unimplemented.

The bytecode audit across 7,933 functions finds 4,700 instance and 902 local
wrapper occurrences supported by this model. It also finds 110 struct-member
and 79 default-variable wrappers, which remain unsupported. LetBool target
forms include 1,629 Boolean wrappers, 700 object contexts and 6 class contexts.
Those counts identify structural forms, not executable gameplay coverage.

`audit_bool_variables.py GAME OUTPUT.json` reproduces that inventory.
`make_instance_fixtures.py` now selects wrapped Boolean getters too. The
retail property-read corpus contains 24 getters, including 14 wrapped Boolean
getters: 76 invocations test two receivers and both Boolean values. Test values
are explicitly supplied, not read from retail class defaults.

check_compatibility.py includes test_bool_variables.cpp. Existing local/instance
property, receiver, named-call and metadata regressions pass under sanitizers.
The original 3,576-image typed-frame corpus also passes: its bounded smoke
probe finishes 776 and rejects 2,789 scripts; completion is not a gameplay or
correctness score.

## Primitive default-property reads checkpoint

DefaultVariable (0x02) reads now use a separate optional environment callback.
Boolean wrappers accept default-variable reads as well. Default writes are
explicitly rejected until their original engine behavior is validated.

ReceiverProgram.bindClassDefault registers typed primitive overrides before
receiver creation. Lookup selects the nearest receiver-class ancestor with an
override, then falls back to the property's explicit initial value. Receiver
creation copies these defaults; later instance writes do not alter them.
The class-default store is read-only to scripts in this model. Unknown schemas,
unrelated owners, wrong types, duplicate overrides and late registrations fail.
This is an explicit host-default model, not deserialization of original CDOs,
UClass execution contexts or validation of the native UE2 default-object ABI.

No complete direct scalar default getter was selected from the current corpus.
make_default_fixtures.py instead extracts original DefaultVariable leaf bytes
into synthetic return harnesses. It finds 95 distinct primitive reads across
493 original occurrences; 72 nonprimitive/nonclass properties, 5 unsupported
layouts and 68 imported properties are deferred (distinct references).
The harness passes 240 invocations using supplied values on two receivers and
both bool values. It does not execute the containing original functions.

```sh
python3 tools/poa/make_default_fixtures.py GAME defaults.bin
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_instance_corpus.cpp -o test-defaults
ASAN_OPTIONS=detect_leaks=0 ./test-defaults defaults.bin
```

The getter generator also accepts --default-getters for future corpora.
The data-free compatibility runner includes test_default_properties.cpp.

## Primitive instance struct-member checkpoint

StructMember (0x36) can now read or assign primitive leaves rooted in an
InstanceVariable. Nested member paths and Boolean wrappers are accepted.
Each instruction's member references are resolved through caller-local
bindings to explicit identities; no native offsets are guessed.

StructProperties.h stores independent scalar leaves per receiver, with
inherited host schemas and explicit defaults. Definitions reject empty,
overlapping or invalid paths. Writes enforce types and byte range. Primitive
and struct bindings cannot occupy the same property reference. Failed writes
leave the target unchanged; complete script invocations are not transactional.

Supported leaves: Boolean, int32, float32 and bytes. Paths are limited to 32
members and consume the shared execution budget. Roots in local/default
variables, returned struct values, arrays, object contexts and native packing
are still unsupported. Struct values cannot be passed as scalar arguments.

make_struct_fixtures.py extracts original direct instance struct-read bytes,
verifying the root StructProperty references the owning Struct of the member
and checking primitive field type/dimension. It selects 95 distinct local
primitive fragments; imported references and unsupported forms are deferred.
The C++ corpus executes 198 invocations in synthetic return harnesses using
host values, two receivers and both Boolean values. No whole original
function, native layout or retail default is validated by that test.

```sh
python3 tools/poa/make_struct_fixtures.py GAME structs.bin
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_struct_corpus.cpp -o test-structs
ASAN_OPTIONS=detect_leaks=0 ./test-structs structs.bin
```

The compatibility runner includes test_struct_properties.cpp. Original game
bytes and generated fixtures are excluded from source updates.

## Default-rooted struct reads and imported metadata checkpoint

Struct paths may now end in DefaultVariable (0x02) for read-only access.
StructProperties.bindDefault supplies primitive leaf overrides before any
receiver is created; the nearest class ancestor's override wins per leaf.
New receivers copy those values, and mutable instance fields remain separate
from defaults. Attempts to assign a default-rooted path throw before evaluating
the assignment's right side. No original default-object loader is added.

The parser propagates the default-root marker through nested struct members
and Boolean wrappers. Synthetic tests cover inherited and nearest overrides,
instance/default separation, copied receiver values, nested paths, imported
bindings, invalid schemas and rejected default writes.

package_objects.py resolves import/export identities across checked packages,
using both qualified path and object kind so unrelated assets sharing a name
do not collide. The struct fixture selector now follows imported root/member
properties and checks that the declared Struct owns the field across packages.
Missing references remain deferred.

```sh
python3 tools/poa/make_struct_fixtures.py GAME defaults.bin --default-roots
ASAN_OPTIONS=detect_leaks=0 ./test-structs defaults.bin
```

Eight distinct retail-derived default struct reads pass in synthetic return
harnesses (16 invocations). Test values and class ownership in the execution
harness are explicitly supplied, not original engine objects or defaults.
The metadata selector separately verifies the original cross-package struct
relationships. Local/returned structs, arrays, object contexts, native layouts,
serialized defaults and state lifecycle remain pending.

With imported metadata resolved, the instance struct-read corpus expands to
404 distinct fragments (309 imported), passing 816 invocations. The full
regression run passes 50 Python tests and eight C++ sanitizer suites. These
remain metadata/isolated-execution checks, not Android gameplay evidence.

## Isolated non-null object contexts checkpoint

Scalar now has an opaque ObjectRef alternative distinct from int32 and a
corresponding Object frame/property type. Self (0x17) returns the current
function frame's receiver; NoObject (0x2a) returns the null handle. Receiver
properties/arguments/results validate non-null handles against the current
isolated registry. Non-null host defaults are rejected pending a real loader.
This registry has no native actor destruction, garbage collection or original
UObject ABI; host-only retirement is added in the later checkpoint below.
Specific ObjectProperty constraints are added by the subsequent constraint
checkpoint below; object argument/return class constraints are added in the call-slot checkpoint below.

Context (0x19) evaluates a non-null object expression, temporarily changes
the access receiver, evaluates the context body and restores the previous
context, including on exceptions. Self inside a context expression remains
the caller frame's Self; a called function receives the target as its own
Self. Function arguments evaluate on the caller frame receiver and then
restore the call target. Calls retain shared depth/instruction budgets.

Explicit context property bindings may refer to fields of another class;
the target receiver's class is checked at access time. Ordinary self-property
bindings retain their stricter ownership checks. Reads and calls through
Context are supported, including mutation within a called script method;
Context assignment targets and class contexts are still rejected. Null
context recovery/skip-result behavior is deliberately unimplemented and
throws, rather than fabricating a default result.

make_context_fixtures.py extracts original Context/property-read instructions
and checks declared ObjectProperty target class ancestry across packages.
The harness supplies receiver identities and field values explicitly. It
does not instantiate original actors, execute containing retail functions
or verify original null/native behavior. Source updates exclude the generated
retail-derived fixtures and all game bytes.

```sh
python3 tools/poa/make_context_fixtures.py GAME contexts.bin
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_context_corpus.cpp -o test-contexts
ASAN_OPTIONS=detect_leaks=0 ./test-contexts contexts.bin
```

The compatibility runner includes test_object_context.cpp.

## Class-constrained object-property checkpoint

ReceiverProgram.bindObjectProperty(id, ownerClass, targetClass) declares a
null-initialized object field constrained to an explicit class. Non-null writes
require an existing receiver whose class is that target or a subclass. Null
is accepted. Invalid assignments throw before changing the stored value.
Inherited field owners and nested script writes retain the same constraint.
The generic bindProperty Object form remains explicitly unconstrained.

Class constraints may also be supplied through the optional objectClass
parameter to bindProperty; a constraint on a non-object field is invalid.
Non-null host defaults remain blocked pending a verified object loader.
The subsequent call-slot checkpoint adds function argument/return constraints;
this property-only checkpoint does not add
class objects, casts, allocation/destruction, GC or original UObject ABI.

make_object_constraint_fixtures.py resolves actual class-owned ObjectProperty
metadata and its target classes across checked packages. The corpus uses
synthetic setters and explicit receiver IDs against that original class graph.
It checks 1,290 schemas: 1,290 exact-class and 890 available subclass assignments,
1,290 null assignments and 1,286 unrelated-class rejections. A lack of an
unrelated class for a root-Object target is valid, not missing coverage hidden
as a rejection. This test does not execute original setters or game actors.

```sh
python3 tools/poa/make_object_constraint_fixtures.py GAME constraints.bin
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_object_constraint_corpus.cpp -o test-constraints
ASAN_OPTIONS=detect_leaks=0 ./test-constraints constraints.bin
```

The compatibility runner includes test_object_constraints.cpp.

## Class-constrained object arguments and returns checkpoint

Slot.objectClass optionally constrains Object-valued arguments and results.
TypedProgram asks the receiver environment to validate both class existence
and non-null receiver ancestry. Argument checks occur before script execution;
return checks occur after execution and do not roll back earlier successful
side effects. Null is valid; an omitted optional object argument defaults to
null. Missing validators fail explicitly rather than bypassing the constraint.

Constraints on primitive/Void slots are rejected. Local-slot class constraints
remain unsupported and are rejected at registration. Generic object slots
retain their previous known-handle semantics. Existing typed fixture records
still omit this field and therefore retain objectClass=0. No original frame
ABI or serialized class-default loading is introduced.

make_object_constraint_fixtures.py --function-slots --schemas SCHEMAS extracts
ObjectProperty argument/return target classes from the checked package set.
A return property's CPF_OutParm flag is not treated as an out argument. True
out/lazy arguments and unsupported/unresolved layouts remain excluded.
Synthetic identity functions validate 2,891 metadata slots (2,667 arguments,
224 returns) against the 1,633-class graph; original functions are not run.

```sh
python3 tools/poa/make_object_constraint_fixtures.py GAME slots.bin \
  --function-slots --schemas function-schemas.json
g++ -std=c++17 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer tools/poa/test_object_call_constraint_corpus.cpp -o test-slots
ASAN_OPTIONS=detect_leaks=0 ./test-slots slots.bin
```

The compatibility runner includes test_object_call_constraints.cpp and four
Python regressions for optional/out/lazy argument versus return flags.

## Host receiver-retirement checkpoint

ReceiverProgram.retire(identity) removes an isolated receiver and its struct
field storage between invocations. Retired identities are tombstoned and
cannot be reused within that registry, preventing stale IDs from naming new
receivers. Unknown identities fail explicitly. Retirement during an active
invocation is rejected; the invocation guard unwinds on exceptions.

Other receivers' ObjectRef fields are not silently cleared. Reads reject a
retired target; the owner can explicitly overwrite the field with null or
a valid live handle. Arguments, results and contexts retain known-handle
checks. Schema/default registration stays frozen permanently after first
receiver creation, even when every receiver has subsequently retired.

This is host-only, single-threaded registry lifecycle, not Unreal Destroy,
pending-kill, actor events, world removal, GC or save/load lifetime behavior.
ObjectRef values are registry-local and must not be mixed across registries.

The data-free runner includes test_receiver_lifetime.cpp. The property
constraint corpus now also retires target receivers and checks stale-field
rejection followed by explicit clearing. It uses synthetic receiver identities
and original property metadata, not original actor-destruction code.

## Original startup and identity checkpoint

`startup_profile.py GAME OUTPUT.json` reads original Default.ini instead of
assuming a startup map or game classes. It verifies selected maps, configured
Class exports and server packages, records file hashes and preserves actual
path spelling. Missing/ambiguous settings, missing dependencies required by
this narrow check and unsupported map URLs fail explicitly.

```
python3 tools/poa/startup_profile.py /path/to/game startup-identity.json
```

`build_profile.py` uses this evidence, rejects stale package audits and gives
unknown executable/script sets a separate disabled profile ID. It remains an offline
profile generator. The selected route is original preamble → front end →
New Game → HP3_Adv1Express; no live transition or input is established.
See the project-level `docs/POA-STARTUP-ROUTE.md` and
`docs/POA-STARTUP-IDENTITY-VERIFICATION.json` for checkpoint evidence.

All 12 scripts have now passed table/dispatch checks and 8,090 function decodes
and schema checks. Sixty-six Python regressions pass, including twelve new
startup/profile tests. Those counts establish neither game execution nor
native ABI compatibility. The complete asset/dependency inventory is pending.

## Complete disc file-list inventory

Use `disc_packages.py DISC OUTPUT --all-files` for every standalone and ZIP
member, including all asset/config/localization types. Default invocation still
returns the script-only report. Complete inventory is metadata-only: declared
ZIP CRCs are not verified payload hashes. The retained original directory and
central records cover 841 file records and all 110 Unreal package files.
Embedded asset inspection and complete payload extraction remain pending.
See `docs/POA-INVENTORY-AND-BUILD-INPUTS.md` at project level. Current checks:
73 PoA Python tests and 18 Android packaging/preflight tests pass.

## Extracted-package integrity and static dependencies

`verify_game_data.py GAME INVENTORY OUTPUT --reference REFERENCE --root PACKAGE`
checks package sizes/CRCs, optional pinned SHA-256, actual tables and transitive
root imports. Repeat --root for multiple routes. Missing, corrupt, unanchored
and unexpected files remain explicit. Incomplete data or requested static
closure returns nonzero. It does not verify dynamic loads or execute natives.

The original-data set now has 24 checked package payloads, including ten more
texture/mesh assets. 86 remain unextracted. See project-level
`docs/POA-PAYLOAD-INTEGRITY-AND-DEPENDENCIES.md`; no original assets enter source
archives. Current PoA Python regressions: 83 passing.
