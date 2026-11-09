# EU based-actor physics transitions

PoA SetPhysics now clears an existing base for modes outside the retail retention set: None (0), Walking (1), Rotating (5), Spider (9) and RootMotion (12). The test uses EU raw values; the UE1 Spider constant must not be used here. Repeating a mode leaves its base unchanged.

Detaching calls the existing production SetBase path with callbacks enabled, updating BasedActors and the reflected Attached list. A GC root retains the child across script callbacks. If callbacks delete it, the setter stops; otherwise stopping-vector behavior checks its current Physics value, since callbacks may change the mode or rebase the actor. A callback's replacement base is preserved. HP1/HP2 retain their previous setter path.

## Original reference

EU Engine.dll SHA256: 73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a. The native retention/clear branch is at 0x1047f7e6–0x1047f83f. Its clear call passes a null base, floor normal (0,0,1) and callback flag 1. Later vector clearing reads the actual current mode at 0x1047f8b6.

Actions 42645356037 passed 338 original setter cases: all old/new modes 0–12, with and without an initial base. The runner supplies synthetic actor/base storage, a null mesh/static-mesh instance, a floor-lookup stub and a recording SetBase callback. It validates raw mode, resulting base, callback count/flag and floor argument. It does not run real actor events, collision, floor discovery, root locks or gameplay.

Captured outputs are retained in tools/poa/reference-profile/eu-physics-base.json. Artifact ZIP SHA256: 9274518e6b125d4d4aecb65c06a5a5737d6a2816468abd32da7669a14c4b3108.

## Production coverage and remaining work

All 144 compatibility tests pass locally, including the original EU asset corpus. Changed production C++ passes syntax checks; canonical patch 0216 passes reverse-application verification. This checkpoint passed ARM64 Actions 42646197180 and still requires device acceptance.

Production SetPhysics and SetBase bodies are exercised together against the 338 outputs for base retention/detachment, callbacks and list maintenance. Separate cases cover reentrant mode changes, rebasing, deletion, GC retention and the previous UE1 setter behavior. The port's SetBase does not implement the original floor-normal argument; that part of the reference is recorded, not claimed implemented.

Floor discovery/replacement, complete base-event parity, root-lock transitions, root queries/translation/save state, Karma and broader UE2 movement/collision behavior remain open. Android attachment/platform behavior still requires device validation. No APK is published by this source checkpoint.
