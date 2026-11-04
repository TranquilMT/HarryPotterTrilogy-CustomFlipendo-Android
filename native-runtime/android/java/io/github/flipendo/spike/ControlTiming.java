package io.github.flipendo.spike;

/** Keep relative input speed stable across display refresh rates and UI scheduling stalls. */
public final class ControlTiming {
    public static float step(long now, long previous) {
        if (previous == 0) return 1f;
        return Math.max(0L, Math.min(50L, now - previous)) / 16f;
    }
    private ControlTiming() {}
}
