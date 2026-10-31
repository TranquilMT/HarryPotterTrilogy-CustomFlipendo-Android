package io.github.flipendo.spike;

import java.io.File;

/** Shares HP2's existing flat six-slot save layout without treating thumbnails as saves. */
final class HP2SaveSlots {
    static File folder(File game) { return GameFiles.child(game, "Save"); }
    static File save(File game, int slot) {
        if (slot < 0 || slot >= 6) throw new IllegalArgumentException("Save slot must be 0..5");
        return GameFiles.child(folder(game), "Save" + slot + ".usa");
    }
    static boolean exists(File game, int slot) { return GameFiles.present(save(game, slot)); }
    private HP2SaveSlots() { }
}
