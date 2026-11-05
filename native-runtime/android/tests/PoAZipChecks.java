package io.github.flipendo.spike;
import java.io.*;import java.nio.file.*;import java.util.*;import java.util.zip.*;
public class PoAZipChecks {
 static byte[] zip(String... names)throws Exception{ByteArrayOutputStream b=new ByteArrayOutputStream();try(ZipOutputStream z=new ZipOutputStream(b)){for(String n:names){z.putNextEntry(new ZipEntry(n));z.write(new byte[]{1,2,3});z.closeEntry();}}return b.toByteArray();}
 static int extract(File dir,String... paths)throws Exception{return new GameZipImporter().extract(new ByteArrayInputStream(zip(paths)),dir,(p,b)->{});}
 public static void main(String[] args)throws Exception{
  int checks=0;File root=Files.createTempDirectory("poa-zip").toFile();String[] paths={"EU\\System\\hppoa.exe","EU/Animations/Harry.ukx","EU/StaticMeshes/Hogwarts.usx","EU/Movies/Intro.bik","EU/Maps/Start.unr"};if(extract(root,paths)!=5)throw new AssertionError();checks++;
  for(String n:new String[]{"System/hppoa.exe","Animations/Harry.ukx","StaticMeshes/Hogwarts.usx","Movies/Intro.bik","Maps/Start.unr"}){if(!new File(root,n).isFile())throw new AssertionError(n);checks++;}
  if(!"hp3".equals(GameFiles.importedGame(root,"hp3")))throw new AssertionError();checks++;
  try{GameFiles.importedGame(root,"hp2");throw new AssertionError();}catch(IOException expected){checks++;}
  for(String bad:new String[]{"../System/escape.u","/System/escape.u","C:/System/escape.u"})try{extract(Files.createTempDirectory("bad-zip").toFile(),bad);throw new AssertionError(bad);}catch(IOException expected){checks++;}
  try{extract(Files.createTempDirectory("duplicate-zip").toFile(),"System/Core.u","System/core.u");throw new AssertionError();}catch(IOException expected){checks++;}
  System.out.println(checks+" PoA ZIP importer checks passed");
 }
}
