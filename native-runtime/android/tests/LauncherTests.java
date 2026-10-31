package io.github.flipendo.spike;
import java.io.*;import java.nio.file.*;import java.util.*;import java.util.zip.*;import org.json.*;
public class LauncherTests {
 static int checks;static void ok(boolean b)throws Exception{checks++;if(!b)throw new Exception("Assertion "+checks);}
 interface Job{void run()throws Exception;}static void rejects(Job j)throws Exception{boolean bad=false;try{j.run();}catch(IOException e){bad=true;}ok(bad);}
 static byte[] zip(Map<String,byte[]> entries)throws Exception{ByteArrayOutputStream b=new ByteArrayOutputStream();try(ZipOutputStream z=new ZipOutputStream(b)){for(Map.Entry<String,byte[]> e:entries.entrySet()){z.putNextEntry(new ZipEntry(e.getKey()));z.write(e.getValue());z.closeEntry();}}return b.toByteArray();}
 static Map<String,byte[]> entries(String name,byte[] data){Map<String,byte[]> m=new LinkedHashMap<>();m.put(name,data);return m;}
 static byte[] module(String game,String path,byte[] data,boolean bad)throws Exception{JSONObject row=new JSONObject().put("path",path).put("sha256",bad?"bad":HDPackInstaller.sha(data));JSONObject meta=new JSONObject().put("game",game).put("textures",new JSONArray().put(row));Map<String,byte[]> m=entries("pack.json",meta.toString().getBytes("UTF-8"));m.put(path,data);return zip(m);}
 static List<HDPackInstaller.Source> source(byte[] data){return Arrays.asList(()->new ByteArrayInputStream(data));}
 public static void main(String[] args)throws Exception{
  File base=Files.createTempDirectory("launcher-tests").toFile();try{
   for(boolean hp2:new boolean[]{false,true}){File root=new File(base,hp2?"hp2":"hp1");new File(root,"sYsTeM").mkdirs();Files.write(new File(root,"sYsTeM/DEFAULT.INI").toPath(),new byte[]{1});ok(GameFiles.repairDefaults(hp2,root,(g,n)->new ByteArrayInputStream(new byte[]{2,3}))==1);ok(Files.readAllBytes(new File(root,"sYsTeM/DEFAULT.INI").toPath())[0]==1);Files.write(new File(root,"sYsTeM/DEFAULT.INI").toPath(),new byte[0]);ok(GameFiles.repairDefaults(hp2,root,(g,n)->new ByteArrayInputStream(new byte[]{4}))==1);ok(GameFiles.repairDefaults(hp2,root,(g,n)->{throw new IOException();})==0);}
   ok(!GameFiles.supported("hp3"));
   File extract=new File(base,"import");byte[] payload={1,2,3};byte[] inner=zip(entries("wrapper/system/HP.exe",payload));byte[] outer=zip(entries("HP1-original-game-data.zip",inner));ok(new GameZipImporter().extract(new ByteArrayInputStream(outer),extract,(p,n)->{})==1);ok(Arrays.equals(Files.readAllBytes(new File(extract,"System/HP.exe").toPath()),payload));
   rejects(()->new GameZipImporter().extract(new ByteArrayInputStream(zip(entries("../escape",payload))),new File(base,"unsafe"),(p,n)->{}));
   Map<String,byte[]> duplicate=entries("System/HP.exe",payload);duplicate.put("system/hp.exe",payload);rejects(()->new GameZipImporter().extract(new ByteArrayInputStream(zip(duplicate)),new File(base,"duplicate"),(p,n)->{}));
   File actual=new File(args[0]);GameFiles.validatePackages(true,actual);ok(GameFiles.missing(true,actual).isEmpty());
   File ue2=new File(base,"ue2");new File(ue2,"System").mkdirs();byte[] header={(byte)0xc1,(byte)0x83,0x2a,(byte)0x9e,120,0,0,0};for(String f:new String[]{"Core.u","Engine.u","HGame.u"})Files.write(new File(ue2,"System/"+f).toPath(),header);rejects(()->GameFiles.validatePackages(true,ue2));
   // Read one real DDS to validate the exact format distributed to the phone.
   byte[] dds=null;try(ZipFile z=new ZipFile(args[1])){Enumeration<? extends ZipEntry> en=z.entries();while(en.hasMoreElements()){ZipEntry e=en.nextElement();if(e.getName().endsWith(".dds")){dds=z.getInputStream(e).readAllBytes();break;}}}ok(dds!=null);HDPackInstaller.dds(dds);final byte[] image=dds;String path="Textures/Test/Skin.dds";File root=new File(base,"hd"),cache=new File(base,"cache");root.mkdirs();cache.mkdirs();byte[] hp1=module("HP1",path,image,false),hp2=module("HP2",path,image,false);
   rejects(()->HDPackInstaller.install(root,false,source(hp2),cache,t->{}));ok(HDPackInstaller.installed(root).isEmpty());
   Map<String,byte[]> both=entries("HP1/01.zip",hp1);both.put("HP2/01.zip",hp2);ok(HDPackInstaller.install(root,false,source(zip(both)),cache,t->{})==1);ok(HDPackInstaller.installed(root).size()==1);ok(Arrays.equals(Files.readAllBytes(new File(root,path).toPath()),image));
   rejects(()->HDPackInstaller.install(root,false,source(module("HP1",path,image,true)),cache,t->{}));ok(Arrays.equals(Files.readAllBytes(new File(root,path).toPath()),image));
   rejects(()->HDPackInstaller.install(root,false,source(module("HP1","Textures/../bad.dds",image,false)),cache,t->{}));
   // Force a failure after the first file is replaced; prior overrides must return intact.
   File directory=new File(root,"Textures/Z.dds");directory.mkdirs();byte[] changed=image.clone();changed[128]^=1;Map<String,byte[]> combo=entries("01.zip",module("HP1",path,changed,false));combo.put("02.zip",module("HP1","Textures/Z.dds",image,false));rejects(()->HDPackInstaller.install(root,false,source(zip(combo)),cache,t->{}));ok(Arrays.equals(Files.readAllBytes(new File(root,path).toPath()),image));ok(HDPackInstaller.installed(root).size()==1);
   HDPackInstaller.remove(root);ok(!new File(root,path).exists());ok(HDPackInstaller.installed(root).isEmpty());ok(directory.isDirectory());
   System.out.println("PASS "+checks+" checks: configuration repair, profile validation, nested imports, traversal, HD metadata/checksums/mips, profile isolation, rollback and removal.");
  }finally{HDPackInstaller.erase(base);}
 }
}
