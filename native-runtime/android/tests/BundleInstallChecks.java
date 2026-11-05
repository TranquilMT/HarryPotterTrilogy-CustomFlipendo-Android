package io.github.flipendo.spike;
import java.io.*;import java.nio.file.*;import java.security.*;import java.util.*;
public class BundleInstallChecks {
 static String hash(byte[] b)throws Exception{StringBuilder s=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))s.append(String.format("%02x",x&255));return s.toString();}
 static Map<String,byte[]> data=new HashMap<>();static File root;static int checks;
 static void manifest(String path,String sha,long size){data.put(".inventory.tsv",(sha+"\t"+size+"\t"+path+"\n").getBytes(java.nio.charset.StandardCharsets.UTF_8));}
 static void install()throws Exception{BundledGameInstaller.install(p->{byte[] b=data.get(p);if(b==null)throw new FileNotFoundException(p);return new ByteArrayInputStream(b);},root,(p,d,t)->{});}
 static void rejected()throws Exception{try{install();throw new AssertionError("accepted invalid bundle");}catch(IOException expected){checks++;}}
 public static void main(String[] args)throws Exception{
 root=Files.createTempDirectory("poa-install").toFile();byte[] good="package".getBytes("UTF-8");data.put("System/HGame.u",good);manifest("System/HGame.u",hash(good),good.length);install();if(!Arrays.equals(Files.readAllBytes(new File(root,"System/HGame.u").toPath()),good))throw new AssertionError();checks++;
 data.put("System/HGame.u",new byte[]{1});rejected();if(!Arrays.equals(Files.readAllBytes(new File(root,"System/HGame.u").toPath()),good))throw new AssertionError();checks++;
 for(String bad:new String[]{"../escape","/absolute","System/../escape","C:/escape","System\\escape","System//escape"}){manifest(bad,hash(good),good.length);rejected();}
 data.put("System/HGame.u",good);manifest("System/HGame.u",hash(good),good.length);install();checks++;
 File ini=new File(root,"System/Default.ini");Files.write(ini.toPath(),"custom".getBytes("UTF-8"));data.put("System/Default.ini",good);manifest("System/Default.ini",hash(good),good.length);install();if(!new String(Files.readAllBytes(ini.toPath()),"UTF-8").equals("custom"))throw new AssertionError();checks++;
 Thread.currentThread().interrupt();try{rejected();}finally{Thread.interrupted();}
 manifest("System/HGame.u",hash(good),good.length);data.put(".inventory.tsv",new byte[0]);rejected();
 System.out.println(checks+" bundle installer checks passed");
 }
}
