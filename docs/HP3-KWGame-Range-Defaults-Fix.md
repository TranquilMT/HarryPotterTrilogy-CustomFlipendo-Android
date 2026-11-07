# HP3 EU KWGame defaults fix — 0.4.14

The supplied 0.4.13 phone report fails while loading `KWGame.CompanionCombatManager`, export 222 (zero based), through `UClass::Load` and `PropertyDataBlock::Load`.

## Reproduced cause

The EU package's `EnemyLoc_DestPositionRange` default contains a 55-byte tagged `RangeVector`. Its X, Y and Z values each contain a tagged `Range` with Min/Max float properties. The old engine reads this as 24 flat float bytes.

| Read step | Original reader | Corrected reader |
| --- | --- | --- |
| RangeVector payload starts | Object offset 500 | Object offset 500 |
| RangeVector ends | Offset 524, inside nested data | Offset 555, exact declared end |
| Following name | Incorrect `SerializeFrom` | `LeadCharLeaveFightRadius` |
| Later name index | -405680, outside 5,153 names | Valid subsequent defaults |

This reproduces both the misleading unknown-property log and the fatal error from the phone.

## Implementation

- Read EU PoA Range/RangeVector properties through the shared tagged-property reader.
- Preserve native Vector and UE1 flat Range serialization.
- Retain property-offset bounds and enforce declared tagged-struct byte counts.
- Save the matching tagged layout. Extend the production package writer with bounded nested-property frames.
- Retain the seven Core bindings, EU kwGame.u, optional IpDrv guards and font fix from 0.4.13.

## Validation

- Production reader consumes the exact original EU defaults and matches the six range floats and following float/bool defaults.
- Production writer and reader pass a save/reload round trip; invalid offsets, unmatched property ends and excessive nesting are rejected.
- Legacy flat Range and native Vector cases pass.
- 107 compatibility tests and 29 Android packaging checks pass.
- Serialized class-default audit: 1,685 classes inspected, 90 tagged Range/RangeVector values verified, no audit errors.
- The canonical follow-up patch reproduces the modified engine source.

Phone startup beyond this failure, rendering, controls and gameplay remain unverified. This fixes the reproduced loader cause; it does not establish full UE2 compatibility.
