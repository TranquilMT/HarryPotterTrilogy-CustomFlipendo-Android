package io.github.flipendo.spike;

import java.util.ArrayList;
import java.util.List;

public final class LatestProgressTests {
    static final class Queue implements LatestProgress.Queue {
        final List<Runnable> pending = new ArrayList<>();
        public void postDelayed(Runnable task, long delay) {
            if (delay != 100) throw new AssertionError("unexpected update interval");
            pending.add(task);
        }
        public void remove(Runnable task) { pending.remove(task); }
        void drain() { pending.remove(0).run(); }
    }
    static void check(boolean ok) { if (!ok) throw new AssertionError(); }

    public static void main(String[] args) throws Exception {
        Queue queue = new Queue();
        List<String> shown = new ArrayList<>();
        LatestProgress progress = new LatestProgress(queue, shown::add);
        Thread worker = new Thread(() -> {
            for (int i = 1; i <= 3505; i++) progress.report("Verified " + i);
        });
        worker.start(); worker.join();
        check(queue.pending.size() == 1);
        queue.drain();
        check(shown.size() == 1 && shown.get(0).equals("Verified 3505"));
        progress.report("Installing");
        check(queue.pending.size() == 1);
        queue.drain();
        check(shown.get(1).equals("Installing"));
        progress.report("Finishing");
        Runnable stale = queue.pending.get(0);
        progress.close();
        shown.add("Complete");
        stale.run(); // A callback already dequeued when the Activity finishes.
        progress.report("late worker update");
        check(queue.pending.isEmpty());
        check(shown.size() == 3 && shown.get(2).equals("Complete"));
        progress.close(); // Lifecycle cleanup is idempotent.
        System.out.println("PASS: 3,505 updates coalesced; phase changes delivered; completion protected; lifecycle closed.");
    }
}
