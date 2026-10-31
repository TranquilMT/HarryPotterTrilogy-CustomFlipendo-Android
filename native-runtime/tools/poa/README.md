# Prisoner of Azkaban compatibility tooling

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
