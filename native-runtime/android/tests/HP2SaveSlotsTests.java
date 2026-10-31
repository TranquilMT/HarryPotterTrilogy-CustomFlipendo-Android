package io.github.flipendo.spike;
import java.io.File;
import java.nio.file.Files;

public final class HP2SaveSlotsTests {
    static void check(boolean ok) { if (!ok) throw new AssertionError(); }
    public static void main(String[] args) throws Exception {
        File game = Files.createTempDirectory("hp2-slots-").toFile();
        try {
            check(IsoExtractor.gameFile("/Help/Background.bmp"));
            check(!IsoExtractor.gameFile("/Setup/install.exe"));
            byte[] menu = LauncherTests.zip(LauncherTests.entries("Disc/Help/Background.bmp", new byte[]{1,2,3}));
            File imported = new File(game, "imported");
            check(new GameZipImporter().extract(new java.io.ByteArrayInputStream(menu), imported, (p,n)->{}) == 1);
            check(new File(imported,"Help/Background.bmp").isFile());
            HDPackInstaller.erase(imported);
            File saves = new File(game, "sAvE"); saves.mkdir();
            Files.write(new File(saves, "Save2.bmp").toPath(), new byte[]{1});
            check(!HP2SaveSlots.exists(game, 2));
            Files.write(new File(saves, "SAVE2.USA").toPath(), new byte[]{1});
            check(HP2SaveSlots.exists(game, 2));
            check(!HP2SaveSlots.exists(game, 1));
            Files.write(new File(saves, "Save1.usa").toPath(), new byte[0]);
            check(!HP2SaveSlots.exists(game, 1));
            for (int n : new int[]{-1,6,100}) {
                boolean rejected=false;
                try { HP2SaveSlots.save(game,n); } catch (IllegalArgumentException expected) { rejected=true; }
                check(rejected);
            }
            System.out.println("PASS: mixed-case paths, thumbnails excluded, empty/missing saves, six-slot bounds.");
        } finally {
            for (File save : HP2SaveSlots.folder(game).listFiles()) save.delete();
            HP2SaveSlots.folder(game).delete(); game.delete();
        }
    }
}
