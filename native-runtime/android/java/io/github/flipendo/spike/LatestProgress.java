package io.github.flipendo.spike;

/** Coalesces worker updates so a large texture pack cannot fill the UI queue. */
final class LatestProgress {
    interface Queue {
        void postDelayed(Runnable task, long delayMillis);
        void remove(Runnable task);
    }
    interface Sink { void show(String text); }

    private final Queue queue;
    private final Sink sink;
    private String latest;
    private boolean pending, closed;
    private final Runnable update = this::drain;

    LatestProgress(Queue queue, Sink sink) {
        this.queue = queue;
        this.sink = sink;
    }

    synchronized void report(String text) {
        if (closed) return;
        latest = text;
        if (!pending) {
            pending = true;
            queue.postDelayed(update, 100);
        }
    }

    private synchronized void drain() {
        pending = false;
        if (!closed) sink.show(latest);
    }

    // The caller shows its final result after this. A delayed progress callback
    // must never replace that result or keep a destroyed Activity alive.
    synchronized void close() {
        closed = true;
        pending = false;
        latest = null;
        queue.remove(update);
    }
}
