package io.github.flipendo.spike;

import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;

public class PoAEditorConfigChecks {
    private static int checks;
    private static void require(boolean value) {
        checks++;
        if (!value) throw new AssertionError("Check " + checks);
    }
    public static void main(String[] args) throws Exception {
        Path root = Files.createTempDirectory("poa-eu-config-");
        try {
            Path system = Files.createDirectories(root.resolve("sYsTeM"));
            Path defaults = system.resolve("defunrealed.INI");
            byte[] original = "[Editor.EditorEngine]\r\nGridSize=16\r\n".getBytes("UTF-8");
            Files.write(defaults, original);
            require(GameFiles.repairHp3EditorConfig(root.toFile()));
            Path target = system.resolve("UnrealEd.ini");
            require(Arrays.equals(original, Files.readAllBytes(target)));
            require(Arrays.equals(original, Files.readAllBytes(defaults)));

            byte[] customized = "[Editor.EditorEngine]\nGridSize=32\n".getBytes("UTF-8");
            Files.write(target, customized);
            require(!GameFiles.repairHp3EditorConfig(root.toFile()));
            require(Arrays.equals(customized, Files.readAllBytes(target)));

            Files.delete(target);
            Path differentlyCased = system.resolve("unrealed.INI");
            Files.write(differentlyCased, customized);
            require(GameFiles.repairHp3EditorConfig(root.toFile()));
            require(Arrays.equals(customized, Files.readAllBytes(target)));
            require(Arrays.equals(customized, Files.readAllBytes(differentlyCased)));
            Files.delete(target);
            Files.delete(differentlyCased);
            ExecutorService pool = Executors.newFixedThreadPool(2);
            try {
                Future<Boolean> first = pool.submit(() -> GameFiles.repairHp3EditorConfig(root.toFile()));
                Future<Boolean> second = pool.submit(() -> GameFiles.repairHp3EditorConfig(root.toFile()));
                require(first.get() != second.get());
            } finally {
                pool.shutdownNow();
            }
            require(Arrays.equals(original, Files.readAllBytes(target)));
            Files.delete(target);
            Files.delete(defaults);
            boolean missing = false;
            try { GameFiles.repairHp3EditorConfig(root.toFile()); }
            catch (IOException expected) { missing = true; }
            require(missing && !Files.exists(target));
            Files.createDirectory(target);
            boolean directory = false;
            try { GameFiles.repairHp3EditorConfig(root.toFile()); }
            catch (IOException expected) { directory = true; }
            require(directory);
            try (java.util.stream.Stream<Path> contents = Files.list(system)) {
                require(contents.noneMatch(p -> p.getFileName().toString().startsWith(".editor-config-")));
            }
            System.out.println("PASS " + checks + " EU config repair checks");
        } finally {
            try (java.util.stream.Stream<Path> walk = Files.walk(root)) {
                for (Path p : walk.sorted(Comparator.reverseOrder()).toArray(Path[]::new)) Files.delete(p);
            }
        }
    }
}
