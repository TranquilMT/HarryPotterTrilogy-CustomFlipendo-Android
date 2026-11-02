# PoA KWGame package recovery checkpoint

Corrects the previous extraction finding: kwGame.u exists directly in the
disc's System directory, outside compressed.zip. The archive also supplies
UDebugMenu.u. Adding those resolves all 431 formerly missing relationships
in the checked package set; no fabricated class/native definitions were used.

Eight packages now provide 1,633 classes, 536 states and 7,933 decoded
functions. All functions decode without errors. The static audit finds
5,462 lexical named-call targets and defers 5,215 explicit-receiver calls.
Zero named calls remain missing from lexical scope lookup. This is not
proof of correct dynamic receivers, original runtime semantics or gameplay.

Added a bounded ISO/Mode 1 BIN inventory tool that searches both standalone
files and ZIP directories. It finds all 12 script packages in the sampled
original filesystem/central-directory sectors. Eight packages were analyzed;
Decorations, AmbientCreatures, Editor and Fire are not yet in this corpus.

44 Python tests pass, including hybrid ISO/BIN layouts, Joliet names,
ZIP extent reads, malformed extents, unsafe names and repeated directories.
Named-call and metadata sanitizer regressions pass. The C++ corpus capacity
was adjusted for the expanded package set. The ASan/UBSan corpus passed
2,059,010 resolved lookups and 276 correctly rejected absent lookups.

No APK or live-object integration was produced. PoA remains disabled and
unplayable. Next: receiver allocation and context/state execution; verify
native behavior against the original DLLs and extend the remaining corpus.

The compact update archive overlays the supplied 0.4.1 source root. It
contains cumulative PoA compatibility changes from this conversation plus
roadmap/evidence updates. It excludes game data and unchanged HD tooling.
