> Superseded extraction finding: see POA-KWGAME-CHECKPOINT.md. kwGame.u exists directly on the disc; native-only ancestry was not the blocker.

# PoA HGame and receiver-call audit checkpoint

Continues the retail metadata source checkpoint.

Retrieved HGame.u, GUI.u, GamePlay.u and UWindow.u from the uploaded disc's
compressed.zip. All were decompressed with original sizes and CRC32 verified.
Together with Core and Engine, the checked set contains 1,693 class/state
scopes and 5,807 decoded functions. Package SHA256 and count evidence is in
POA-HGAME-VERIFICATION.json. Extracted retail bytes/dumps are excluded.

Added lexical named-call auditing. Calls nested in explicit object/class
contexts or delegates are deferred instead of incorrectly bound to self.
2,970 calls have lexical targets, 3,490 have deferred receiver contexts,
and 642 need dynamic receivers or missing definitions.

Fixed metadata ownership validation: if a class ancestor is unavailable,
state-super ownership remains unresolved rather than being declared invalid.
38 Python tests and the named-call/metadata C++ sanitizer regressions pass.

The full compatibility check intentionally stops on 431 unresolved KWGame
metadata relationships (429 supers and 2 ownership checks), after writing
metadata and call reports. The archive directory contains KWGame.dll but no
KWGame.u. Native DLL analysis is still needed; no metadata has been invented.
This correct rejection is not a complete retail-corpus pass.

No APK was built. PoA remains disabled and unplayable; the live object VM,
state transitions, native behavior and world integration are still pending.
Next: reconstruct KWGame registration metadata and verify it against the
import references before live receiver dispatch integration.
