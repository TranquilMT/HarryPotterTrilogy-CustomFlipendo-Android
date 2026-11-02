# PoA isolated receiver-call checkpoint

Added ReceiverProgram.h and test_receiver_program.cpp. Extended TypedProgram
with an optional call environment for receiver-aware named resolution and
function ownership validation. Shared DispatchResolver lookup logic now
serves both static image links and receiver-aware invocation.

Receiver identities, class inheritance and state selection are isolated
metadata. The same script image can dispatch differently for two receiver
instances without rewriting its static links. The environment is retained
through final/native/named calls, with strict argument checks and shared
execution limits. Missing behavior is rejected without fabricated success.

State selection checks receiver compatibility and every state ancestor's
owner class. It changes only between invocations. No game actors, instance
properties, object contexts, BeginState/EndState or latent state execution
were implemented. Actual retail receiver behavior remains unverified.

Validation: 44 Python tests; synthetic receiver, prior named-call and
metadata tests under ASan/UBSan; 2,059,010 retail-table lookup agreements
and 276 rejected absent lookups. Existing primitive-frame regressions pass
against 3,576 supported retail function images, separately from the new
receiver path. Details are in POA-RECEIVER-VERIFICATION.json.

The compatibility runner includes the new test and uses -O1 with sanitizers.
No Android build or gameplay test was run. PoA remains disabled/unplayable.
Next: object properties and explicit Context/Self execution, followed by
state lifecycle and validation of native behavior against retail code.

Apply this cumulative overlay to the supplied 0.4.1 source root or a later
checkpoint in this conversation. It contains source and evidence only.
