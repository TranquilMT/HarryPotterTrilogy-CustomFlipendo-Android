package io.github.flipendo.spike;

import java.io.*;
import java.nio.file.*;
import java.util.*;

/** Pure Java validation/repair shared by launch, bundle install and ZIP import. */
public final class GameFiles {
 public interface Defaults { InputStream open(boolean hp2,String name) throws IOException; }
 public static boolean supported(String id){return "hp1".equals(id)||"hp2".equals(id);}
 public static File child(File dir,String name){
  File exact=new File(dir,name);if(exact.exists())return exact;
  File[] list=dir.listFiles();if(list!=null)for(File f:list)if(f.getName().equalsIgnoreCase(name))return f;
  return exact;
 }
 public static File path(File root,String relative){File p=root;for(String part:relative.split("/"))p=child(p,part);return p;}
 public static boolean present(File f){return f.isFile()&&f.length()>0;}
 public static File find(boolean hp2,File dir,int depth){
  if(present(path(dir,"System/"+(hp2?"Game.exe":"HP.exe"))))return dir;
  File[] list=depth>0?dir.listFiles():null;
  if(list!=null)for(File f:list)if(f.isDirectory()&&!f.getName().startsWith(".")){File found=find(hp2,f,depth-1);if(found!=null)return found;}
  return null;
 }
 public static List<String> missing(boolean hp2,File root){
  List<String> result=new ArrayList<>();
  String[] system=hp2?new String[]{"Game.exe","Core.u","Engine.u","HGame.u","HProps.u","HPModels.u","HPSounds.u","Default.ini","DefUser.ini"}:new String[]{"HP.exe","Core.u","Engine.u","HarryPotter.u","HPBase.u","HPPuzzle.u","HPMenu.u","Default.ini","DefUser.ini"};
  for(String n:system)if(!present(path(root,"System/"+n)))result.add("System/"+n);
  for(String n:new String[]{"Maps","Textures"}){File d=child(root,n);File[] files=d.listFiles();boolean found=false;if(files!=null)for(File f:files)if(present(f)&&f.getName().toLowerCase(Locale.ROOT).endsWith(n.equals("Maps")?".unr":".utx"))found=true;if(!found)result.add(n+"/ (original game packages)");}
  if(!present(path(root,"Sounds/AllDialog.uax")))result.add("Sounds/AllDialog.uax");
  return result;
 }
 public static void validatePackages(boolean hp2,File root) throws IOException {
  for(String name:new String[]{"Core.u","Engine.u",hp2?"HGame.u":"HarryPotter.u"}){
   File p=path(root,"System/"+name);byte[] h=new byte[8];
   try(DataInputStream in=new DataInputStream(new FileInputStream(p))){in.readFully(h);}
   if((h[0]&255)!=0xc1||(h[1]&255)!=0x83||(h[2]&255)!=0x2a||(h[3]&255)!=0x9e)throw new IOException("Invalid original game package: "+p);
   int version=(h[4]&255)|((h[5]&255)<<8);
   if(version<60||version>=100)throw new IOException("Unsupported engine package version "+version+". Prisoner of Azkaban is not working yet.");
  }
 }
 /** Missing/empty only. Never replace a player's nonempty ini or remove files. */
 public static int repairDefaults(boolean hp2,File root,Defaults source) throws IOException {
  File system=child(root,"System");if(!system.isDirectory()&&!system.mkdirs())throw new IOException("Cannot create "+system);
  int repaired=0;
  for(String name:new String[]{"Default.ini","DefUser.ini"}){
   File target=child(system,name);if(present(target))continue;
   if(target.exists()&&!target.isFile())throw new IOException(target+" is a directory, not a configuration file");
   File temp=File.createTempFile(".config-", ".tmp",system);
   try {
    try(InputStream in=source.open(hp2,name);FileOutputStream out=new FileOutputStream(temp)){
     byte[] buffer=new byte[16384];int n;while((n=in.read(buffer))!=-1)out.write(buffer,0,n);out.getFD().sync();
    }
    if(temp.length()==0)throw new IOException("Empty built-in "+name);
    try{Files.move(temp.toPath(),target.toPath(),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);}catch(AtomicMoveNotSupportedException e){Files.move(temp.toPath(),target.toPath(),StandardCopyOption.REPLACE_EXISTING);}
    ++repaired;
   }finally{temp.delete();}
  }
  return repaired;
 }
 private GameFiles(){}
}
