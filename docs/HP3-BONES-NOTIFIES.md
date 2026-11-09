# 9 November — bone queries, attachments and native notifies

This source checkpoint implements six EU Actor declarations: HasBone, GetBoneCoords, GetBoneRotation, BoneRefresh, AttachToBone and DetachFromBone. Original declarations determine parameter counts and native indices. Bone queries use the current blended pose and the same mesh/actor transforms as rendering. Attachments apply relative position/rotation, follow animated parents, reject cycles, retain objects across callbacks and synchronize the reflected Attached array. PoA actors no longer write the absent UE1 StandingCount field. HP1/HP2 keep their existing base behavior.

Script-backed notify objects retain their Notify(Owner) dispatch. Native AnimNotify_Script invokes a named zero-argument owner function; unsupported delegate dispatch fails explicitly. Native AnimNotify_Sound uses the stored volume/radius, slot None and pitch 1; zero radius uses the original Core default of 80. Unknown native notify classes fail explicitly. The five inspected EU animation packages contain two object notifies, both KWSound script subclasses; this does not establish coverage of dynamically created effects or every native notify class.

Production-body adapter tests cover six declarations, animated bone transforms, scaled/rotated parents, attach/reparent/detach, cycle rejection, reflected lists, legacy base behavior, script routing and sound arguments. They do not prove original attachment collision behavior, full affine child rendering, frame ordering or device performance.

## Root motion remains open

The EU call audit finds one static GetRootLocation candidate, HGame.MonsterBook.SpawnPage. It finds no static LockRootMotion/GetRootLocationDelta/GetRootRotation candidates in the twelve script packages; native/dynamic calls are excluded. Original GetRootLocation reads cached mesh-instance state, and GetFrame updates that cache. Original physRootMotion does not simply integrate velocity or consume a translation delta. Implementing a guessed velocity path would therefore be incorrect.

The EU EPhysics values 7–11 also differ from the inherited UE1 enumeration (EU Interpolating=7, MovingBrush=8, Spider=9, Trailer=10, Ladder=11; RootMotion=12). Dispatch mapping and root-lock/cache/movement semantics require implementation and validation together. No root-motion completion is claimed here.

## Validation

All 137 compatibility tests and 29 packaging tests pass locally. Changed production C++ files pass syntax checks; the engine patch passes reverse-application verification. The original EU mesh/animation corpus regressions remain green.

## Acceptance still required

Root-motion and other animation/bone modification APIs, native effect notify classes where used, original pose/event parity, Android wand/item placement, complete game saves and broader terrain/material/world/physics work remain open. The published v0.4.14 APK is unchanged. Previous extended animation source passed ARM64 Actions 42528353602; this checkpoint requires its own ARM64 build and device acceptance.
