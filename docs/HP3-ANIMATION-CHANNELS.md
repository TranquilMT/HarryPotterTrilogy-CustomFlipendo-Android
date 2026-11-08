# PoA actor animation channels

## Production integration

The EU Actor class has no UE1 AnimFrame/AnimSequence/AnimRate/bAnimLoop properties. PoA now owns a separate lazily allocated native channel state on each actor. Its TickAnimation route bypasses the incompatible UE1 property path, and its skeletal draw route supplies the resulting layered pose to CPU skinning. Actors without a played sequence retain the reference pose.

Fourteen PoA-only native replacements match the original EU declaration arities and native indices: PlayAnim, LoopAnim, TweenAnim, IsAnimating, FinishAnim, StopAnimating, HasAnim, GetAnimParams, GetAnimSequence, GetAnimNumFrames, AnimIsInGroup, LinkSkelAnim, AnimBlendParams and AnimBlendToAlpha. TweenAnim is index 294, GetAnimSequence 603 and GetAnimNumFrames 604; these are checked against the original Engine.u native stubs. All typed handlers compile with the production NativeFunc templates. This does not prove every runtime ABI or every animation API.

The port currently bounds stages to 0–15. Base stage starts at full weight; additional stages require blend configuration. Timing supports positive/zero playback rates, one-shot and looping sequences, frozen TweenAnim transitions, normalized phase queries, alpha ramps, local-space layered interpolation and descendant masks by bone name. Layering occurs before hierarchical composition and inverse-bind skinning. Sequence lookup is case-insensitive and does not silently substitute the first sequence for a missing name.

Per-actor LinkSkelAnim overrides can optionally replace that actor's mesh without changing another actor sharing the original mesh. Animation relinking resets the actor's channel state. Actor GC marking retains its override resource; native skeletal marking retains the default resource. Native channel state is not serialized into game saves yet.

The tick queues function and object notify events and calls AnimEnd with the integer channel argument. Script-backed object notifies receive their declared Owner actor argument. A missing native-only Notify dispatcher fails explicitly rather than pretending to execute sound/effects behavior. Channel generations suppress stale events after playback changes; relinking replaces the state identity, cancelling old queued events. Actor and resource GC roots remain live during callbacks. FinishAnim tracks the requested channel and resumes the actor's latent execution when playback/tweening finishes, before end-event callbacks can start another sequence. Relinking releases an old latent animation wait. Loop waiting exits at the current loop's last-frame boundary.

## Validation

- 132 compatibility tests and 29 Android packaging tests passed locally; production C++ syntax checks passed.
- A declaration regression reads original EU Engine.u and verifies the fourteen adapter argument counts and native indices.
- Production handler-body tests use object, event, latent-frame and GC-root adapters. They exercise playback, queries, per-actor isolation, resource linking, script notify arguments, AnimEnd channel arguments, latent completion, tweening and explicit unsupported cases. They are not a whole VM/GC/world test.
- Production model tests cover timing, loop event counts, tween source poses, bone masks, alpha ramps, hierarchy composition, state isolation and invalid inputs under ASan/UBSan.
- The compiled original-data test decodes all 167 meshes and 140 animation resources. It drives the first sequence for all 146 meshes with a local default animation reference through channel evaluation and skinning (104,781 points), checking finite output. The other 21 meshes are not claimed animated by this check. These samples are not independently expected original poses.
- The preceding skeletal checkpoint passed ARM64 Actions 42522582035. This channel checkpoint has a separate ARM64 gate. No Android device is attached; no live playable-scene or original-animation-parity claim.

## Remaining scope

- Original-game timing/pose comparisons and full Android VM/world/GPU acceptance, including all script paths and HP1/HP2 regression checks.
- Native animation APIs not listed above, including exact time/frame/rate setters and getters, bone transforms/attachments, special character animation behavior and replication where needed.
- Global-pose blending, automatic OutTime fade-out, backwards rates and root-motion extraction. Unsupported global-pose/fade-out/backwards requests currently fail explicitly.
- Interrupted tween continuity: a new tween snapshots the previous sequence/phase, not the already blended multi-channel pose. There is no claim of interruption parity.
- Exact within-frame callback resumption and cross-channel event ordering. Events are collected per channel before script callbacks; generation guards prevent stale callbacks after a sequence/state replacement.
- Native-only notify execution, original sound/effects behavior, save/load persistence of channel state, optimized LOD/caching/lighting and mobile performance.
- The sixteen-channel bound is a documented port limit, not a claim about all UE2 versions. Excessive loop-event work is bounded and fails explicitly.

The source update does not modify the published v0.4.14 APK or its release claims.
