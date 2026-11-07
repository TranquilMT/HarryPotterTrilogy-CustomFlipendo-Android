package io.github.flipendo.spike;
import java.io.*;
import java.nio.file.*;
import java.util.*;
public class ThreeGameLaunchTests {
 static int checks;
 static void ok(boolean value){++checks;if(!value)throw new AssertionError("check "+checks);}
 interface Action {void run()throws Exception;}
 static void rejects(Action action)throws Exception{boolean failed=false;try{action.run();}catch(IOException|IllegalArgumentException e){failed=true;}ok(failed);}
 static void put(File root,String path,byte[] bytes)throws Exception {File file=new File(root,path);file.getParentFile().mkdirs();Files.write(file.toPath(),bytes);}
 static byte[] header(int version){return new byte[]{(byte)0xc1,(byte)0x83,0x2a,(byte)0x9e,(byte)version,0,0,0};}
 public static void main(String[] args)throws Exception {
  Path tmp=Files.createTempDirectory("three-game-launch-");
  try{
   File hp1=tmp.resolve("stone").toFile(),hp2=tmp.resolve("chamber").toFile(),hp3=tmp.resolve("poa/Original").toFile();
   put(hp1,"System/HP.exe",new byte[]{1});put(hp2,"system/game.EXE",new byte[]{1});put(hp3,"sYsTeM/HPPOA.EXE",new byte[]{1});
   ok(GameFiles.find("hp1",hp1,0).equals(hp1));ok(GameFiles.find("hp2",hp2,0).equals(hp2));
   ok(GameFiles.find("hp3",hp3.getParentFile(),2).equals(hp3));
   ok(GameFiles.find("hp3",new File(hp3,"sYsTeM"),0).equals(hp3));
   ok(GameFiles.find("hp1",hp3,2)==null);ok(GameFiles.find("hp3",hp1,2)==null);ok(GameFiles.find("hp3",hp2,2)==null);
   ok(new HashSet<>(Arrays.asList(GameFiles.folderKey("hp1"),GameFiles.folderKey("hp2"),GameFiles.folderKey("hp3"))).size()==3);
   rejects(()->GameFiles.folderKey("unknown"));rejects(()->GameFiles.find("unknown",hp1,0));
   for(String name:new String[]{"Core.u","Engine.u","HGame.u","GUI.u","UWindow.u","GamePlay.u"})put(hp3,"sYsTeM/"+name,header(129));
   put(hp3,"sYsTeM/Default.ini",new byte[]{1});put(hp3,"sYsTeM/DefUser.ini",new byte[]{1});
   put(hp3,"maps/preamble.unr",header(129));put(hp3,"textures/test.utx",header(120));
   ok(GameFiles.missingHp3(hp3).isEmpty());GameFiles.validatePackages("hp3",hp3);++checks;
   byte[] original=Files.readAllBytes(GameFiles.path(hp3,"System/Default.ini").toPath());
   ok(!GameFiles.missingHp3(hp3).contains("System/HProps.u"));
   put(hp3,"sYsTeM/Core.u",header(69));rejects(()->GameFiles.validatePackages("hp3",hp3));
   put(hp3,"sYsTeM/Core.u",new byte[]{1,2});rejects(()->GameFiles.validatePackages("hp3",hp3));
   ok(Arrays.equals(original,Files.readAllBytes(GameFiles.path(hp3,"System/Default.ini").toPath())));
   for(int hz:new int[]{30,60,120}){float steps=0;long before=1000;for(int i=1;i<=hz;i++){long now=1000+Math.round(i*1000.0/hz);steps+=ControlTiming.step(now,before);before=now;}ok(Math.abs(steps-62.5f)<0.001f);}
   ok(ControlTiming.step(1000,2000)==0);ok(ControlTiming.step(5000,1000)==3.125f);ok(ControlTiming.step(1000,0)==1);
   System.out.println("PASS "+checks+" three-game path, PoA preflight and refresh-rate checks");
  }finally{try(java.util.stream.Stream<Path> walk=Files.walk(tmp)){walk.sorted(Comparator.reverseOrder()).forEach(p->{try{Files.delete(p);}catch(IOException e){throw new UncheckedIOException(e);}});}}
 }
}
