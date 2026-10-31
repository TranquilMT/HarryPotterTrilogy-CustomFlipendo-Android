package io.github.flipendo.spike;
import java.io.*;import java.nio.file.*;import java.util.*;
/** Built-in HD modules are optional, controlled independently for each game. */
public final class BundledHD {
 public interface Assets { String[] list(String path)throws IOException; InputStream open(String path)throws IOException; }
 static final String MARKER=".flipendo-bundled-hd";
 static String read(File f)throws IOException{return f.isFile()?new String(Files.readAllBytes(f.toPath()),java.nio.charset.StandardCharsets.UTF_8).trim():"";}
 static void state(File root,String value)throws IOException{File temp=File.createTempFile(".hd-state-",".tmp",root);try{try(FileOutputStream out=new FileOutputStream(temp)){out.write(value.getBytes(java.nio.charset.StandardCharsets.UTF_8));out.getFD().sync();}Files.move(temp.toPath(),new File(root,MARKER).toPath(),StandardCopyOption.REPLACE_EXISTING);}finally{temp.delete();}}
 public static int ensure(File root,boolean hp2,File cache,Assets assets,HDPackInstaller.Progress progress,boolean force)throws Exception{
  String folder="bundled-hd/"+(hp2?"hp2":"hp1");String stamp;
  try(InputStream in=assets.open(folder+"/.stamp")){stamp=new String(HDPackInstaller.bytes(in,1024),java.nio.charset.StandardCharsets.UTF_8).trim();}catch(FileNotFoundException absent){return 0;}
  String old=read(new File(root,MARKER));if(!force&&(old.equals("disabled")||old.equals(stamp)))return 0;
  List<HDPackInstaller.Source> sources=new ArrayList<>();String[] names=assets.list(folder);if(names!=null){Arrays.sort(names);for(String name:names)if(name.endsWith(".zip")){final String path=folder+"/"+name;sources.add(()->assets.open(path));}}
  if(sources.size()!=6)throw new IOException("Built-in HD modules are incomplete");
  int count=HDPackInstaller.install(root,hp2,sources,cache,progress);state(root,stamp);return count;
 }
 public static synchronized int apply(File root,boolean hp2,File cache,Assets assets,HDPackInstaller.Progress progress,boolean enabled)throws Exception {
  if(!enabled){if(!HDPackInstaller.installed(root).isEmpty())HDPackInstaller.remove(root);if(!read(new File(root,MARKER)).equals("disabled"))disable(root);return 0;}
  return ensure(root,hp2,cache,assets,progress,read(new File(root,MARKER)).equals("disabled"));
 }
 public static void disable(File root)throws IOException{state(root,"disabled");}
}
