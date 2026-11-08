# Extended PoA animation APIs and native channel persistence

## Native behavior traced and implemented

The PoA Actor binding set now contains 22 scoped handlers. This follow-up adds GetAnimTime, SetAnimTime, GetAnimFrame, SetAnimFrame, GetAnimRate, SetAnimRate, FreezeAnimAt and GetRootRotationDelta. Original Engine.u validates all binding arities and indices. Optional channel arguments now use the actor's reflected DefaultAnimChannel, matching the retail Actor wrappers' default-channel initialization rather than assuming zero.

Original EU Engine.dll SHA-256: `73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a`.

| API | Observed helper behavior | Reference body address |
| --- | --- | --- |
| GetAnimTime | Return stored normalized phase | 0x104e44c0 |
| SetAnimTime | Call SetAnimFrame with unit flag 1, storing phase directly | 0x104e2140 |
| GetAnimFrame | Return frame count multiplied by stored phase | 0x104e43b0 |
| SetAnimFrame | Nonzero unit flag stores phase directly; zero flag divides by frame count minus one | 0x104e8ee0 |
| GetAnimRate | Return stored normalized playback rate | 0x104e4370 |
| SetAnimRate | Multiply requested factor by sequence base normalized rate | 0x104e8d90 |
| FreezeAnimAt | Store requested phase and clear rate/tween rate | 0x104e4040 |
| GetRootRotationDelta | Construct a zero rotator after an optional pose refresh | 0x104e1670 |

The frame setter/getter asymmetry is retained. The original one-frame zero-unit setter divides by zero; the port keeps finite values for that case. Non-finite inputs and backwards playback rates are rejected explicitly. GetRootRotationDelta supplies the observed zero result; it is not translational root-motion extraction, root tracking, physics, or proof of the original helper's optional pose-refresh side effects.

The isolated reference workflow **42527223513** succeeded. It executed 70 original getter/setter cases using synthetic skeletal-instance storage, frame-count/name callbacks and a Linux i386 SEH segment. No Windows entry point, Actor VM or game loop ran. The committed numerical profile `native-runtime/tools/poa/reference-profile/eu-animation-helpers.json` is checked against production handler bodies. Seven original non-finite one-frame cases are documented finite corrections. Artifact 13635613306 had ZIP SHA-256 `57b2eb23d634c64d7754d4c18f0e3d776a9155c9549180511a8a8b394239619d`.

## Blending and tween corrections

Retail SetBlendParams at 0x104e56e0 writes alpha immediately, stores In/Out coefficients clamped to at least one, and clears the BlendToAlpha timer. The prior port interpretation of InTime as that timer's duration was incorrect and is replaced. Nonzero OutTime values are now retained as coefficients. No automatic fade-out behavior is asserted from coefficient storage; how other paths consume those coefficients still needs verification. AnimBlendToAlpha remains the explicit timed alpha-ramp route.

Retail GetFrame tests the global-pose flag at the selected blend root (branch at 0x104f7228 and root comparison at 0x104f7236). The new pose path converts that root's channel-global target into the current parent's local space; descendants continue local-space blending. An analytic masked-parent test distinguishes the global-root result from a local-root result. This does not replace a visual comparison with original whole-character animation.

Interrupted tweens capture the current channel's evaluated local pose before starting the next transition. Local-channel interruption continuity is tested instead of snapping back to the previous sequence's unblended phase. Snapshots are tied to the skeleton bone count; changing the skeleton without relinking fails explicitly. Global/multi-channel interruption parity still requires original comparisons.

## Native actor save integration

PoA UActor Save/Load now includes an optional port-owned trailer after reflected properties. Its two-word magic and version 1 distinguish it from original map data. Actors without state/overrides add no trailer; unknown original native tails are left at their original stream position. Recognized malformed/unsupported trailers fail explicitly.

The trailer stores the linked animation object reference using the package writer, sixteen channel records, names/masks, exact double phases, rates, alpha state, tween state, coefficients, global-pose flags, interrupted-tween snapshots and latent wait channel. Event generation counters restart because no queued frame callbacks persist across load. Channel data replaces the previous channel state only after successful validation. This is not whole-actor transactional rollback or original PC save interchange.

Loading a saved PoA state frame now preserves its actual UState separately from its bytecode owner, as the existing HP2 path already does. This prevents an inherited function from becoming the state identity during reload. A production UObject::Load-body test checks distinct code/state objects, FinishAnim restoration, HP2 behavior and unchanged legacy selection.

Host tests cold-load a newly constructed actor/stream, serialize it identically again, preserve tween snapshots/global flags/coefficient data, reject truncated and unknown-version trailers, and check channel validation. They use bounded stream/object/frame adapters. They do not demonstrate a real map save, cold Android restart, full VM continuation or phone reload.

## Verification and remaining work

134 compatibility tests and 29 packaging tests pass locally; changed production C++ files pass syntax checks. The preceding channel checkpoint passed ARM64 Actions 42524780799. This follow-up has its own build gate.

Remaining work includes translational/root-motion tracking and collision integration, bone/socket attachments, native-only notifies and special animation behavior, exact within-frame/cross-channel callback ordering, other animation APIs, original pose/timing comparisons, full map-save/VM/device acceptance, optimized LOD/lighting/performance and the wider terrain/world/rendering/gameplay workstreams. The source update does not publish or alter the v0.4.14 APK.
