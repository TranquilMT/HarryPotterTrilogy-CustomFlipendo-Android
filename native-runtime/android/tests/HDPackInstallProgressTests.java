package io.github.flipendo.spike;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** Exercises real ZIP installation and rollback; no Android framework needed. */
public final class HDPackInstallProgressTests {
    static void check(boolean ok) { if (!ok) throw new AssertionError(); }

    public static void main(String[] args) throws Exception {
        File base = Files.createTempDirectory("hd-progress-test-").toFile();
        try {
            File root = new File(base, "hp2"), cache = new File(base, "cache");
            root.mkdirs(); cache.mkdirs();
            byte[] dds = new byte[136];
            dds[0]='D'; dds[1]='D'; dds[2]='S'; dds[3]=' '; dds[4]=124;
            dds[12]=1; dds[16]=1; dds[28]=1;
            System.arraycopy("DXT1".getBytes("US-ASCII"), 0, dds, 84, 4);
            String first = "Textures/Test/A.dds", second = "Textures/Test/B.dds";
            byte[] original = LauncherTests.module("HP2", first, dds, false);
            List<String> phases = new ArrayList<>();
            HDPackInstaller.install(root, true,
                Arrays.asList(() -> new ByteArrayInputStream(original)), cache, phases::add);
            check(phases.get(0).startsWith("Extracting and verifying"));
            check(phases.stream().anyMatch(t -> t.startsWith("Installing 1 / 1")));
            check(phases.get(phases.size()-2).startsWith("Saving installed"));
            check(phases.get(phases.size()-1).startsWith("Cleaning temporary"));

            byte[] changed = dds.clone(); changed[128]=1;
            byte[] replacement = LauncherTests.module("HP2", first, changed, false);
            byte[] addition = LauncherTests.module("HP2", second, changed, false);
            boolean cancelled = false;
            try {
                HDPackInstaller.install(root, true, Arrays.asList(
                    () -> new ByteArrayInputStream(replacement),
                    () -> new ByteArrayInputStream(addition)), cache,
                    text -> { if (text.startsWith("Installing 1 /")) Thread.currentThread().interrupt(); });
            } catch (java.io.IOException expected) {
                cancelled = expected.getMessage().equals("Installation cancelled");
            } finally {
                Thread.interrupted();
            }
            check(cancelled);
            check(Arrays.equals(dds, Files.readAllBytes(new File(root, first).toPath())));
            check(!new File(root, second).exists());
            check(HDPackInstaller.installed(root).size() == 1);
            check(cache.list().length == 0);
            check(Arrays.stream(root.list()).noneMatch(n -> n.startsWith(".hd-backup-")));
            System.out.println("PASS: install phases, interrupted commit rollback, original DDS/manifest preserved, temporary files cleaned.");
        } finally {
            HDPackInstaller.erase(base);
        }
    }
}
