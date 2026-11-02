# HP3 / UE2 native function mapping (tracking)

Status: **tracking only**. No native bodies implemented. HP3 remains disabled.
Each entry must be verified against retail PoA data before implementation.

## Priority 1 — startup / package / object

- `StaticLoadObject` / `DynamicLoadObject` — UE2 package loading with profile selection.
- `FindObject` / `FindObjectFast` — name-based lookup in UE2 name tables.
- `ConstructObject` / `NewObject` — factory allocation for UE2 classes.
- `GetPackageName` / `GetPathName` — identity strings.

## Priority 2 — math / vector / rotator (likely reusable)

- `Sin`, `Cos`, `Tan`, `Atan`, `Sqrt`, `Abs`, `Clamp`, `Min`, `Max`, `Lerp`
- `VRand`, `Normal`, `VSize`, `Dot`, `Cross`

## Priority 3 — actor / world

- `Spawn`, `Destroy`, `SetLocation`, `SetRotation`, `Move`, `SetPhysics`
- `Trace`, `FastTrace`, `Actors`, `DynamicActors`, `VisibleActors`
- `MakeNoise`, `PlaySound`, `StopSound`

## Priority 4 — rendering / animation (UE2-specific)

- `GetAnimParams`, `PlayAnim`, `LoopAnim`, `FinishAnim`
- `SetDrawScale`, `SetDrawScale3D`, `SetRotation` (render)
- Material / texture helpers

## Priority 5 — audio / UI / save

- `ConsoleCommand`, `ClientMessage`
- `SaveConfig`, `ResetConfig`
- Save/load serialization helpers

## Rules

- Reuse a UE1 body only after signature and behavior verification.
- Profile-specific bindings prevent PoA indices from replacing HP1/HP2 registrations.
- Unsupported natives fail closed with useful identities; never return dummy success.
- No bodies until retail data confirms semantics.
'@, "C:\\Users\\TSGxR\\Documents\\Default Project\\hp-tps-work\\native-runtime\\docs\\re\\hp3\\NATIVE-FUNCTION-MAPPING.md")