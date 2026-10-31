package io.github.flipendo.spike;
import java.io.*;
import java.nio.file.*;
import java.util.*;
public class ExternalHDTests {
 public static void main(String[] args)throws Exception {
  File base=Files.createTempDirectory("external-hd-test-").toFile();
  byte[] dds=new byte[136]; dds[0]='D';dds[1]='D';dds[2]='S';dds[3]=' ';dds[4]=124;dds[12]=1;dds[16]=1;dds[28]=1;
  System.arraycopy("DXT1".getBytes("US-ASCII"),0,dds,84,4);
  try {for(boolean hp2:new boolean[]{false,true}) {
   File root=new File(base,hp2?"hp2":"hp1"),cache=new File(base,"cache");root.mkdirs();cache.mkdirs();
   BundledHD.Assets absent=new BundledHD.Assets(){public String[] list(String p){return new String[0];}public InputStream open(String p)throws IOException{throw new FileNotFoundException(p);}};
   LauncherTests.ok(BundledHD.apply(root,hp2,cache,absent,t->{},false)==0);
   byte[] pack=LauncherTests.module(hp2?"HP2":"HP1","Textures/Test/Skin.dds",dds,false);
   LauncherTests.ok(HDPackInstaller.install(root,hp2,Arrays.asList(()->new ByteArrayInputStream(pack)),cache,t->{})==1);
   LauncherTests.ok(BundledHD.apply(root,hp2,cache,absent,t->{},true)==0);
   LauncherTests.ok(HDPackInstaller.installed(root).size()==1);
   BundledHD.apply(root,hp2,cache,absent,t->{},false);
   LauncherTests.ok(HDPackInstaller.installed(root).isEmpty());
   LauncherTests.ok(!new File(root,"Textures/Test/Skin.dds").exists());
  }System.out.println("PASS "+LauncherTests.checks+" external-HD checks: both profiles, no bundled assets, imported overrides survive enabled launch, disabled restores originals.");
  }finally{HDPackInstaller.erase(base);}
 }
}
