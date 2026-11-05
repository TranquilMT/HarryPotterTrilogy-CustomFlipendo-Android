package io.github.flipendo.spike;
import java.io.*;import java.nio.charset.StandardCharsets;import java.nio.file.*;import java.security.*;import java.util.*;
/** Checked bundle installation; preserves existing saves and nonempty player configuration. */
public final class BundledGameInstaller {
 public interface Source{InputStream open(String path)throws IOException;}
 public interface Progress{void update(String path,int done,int total);}
 public static void install(Source source,File game,Progress progress)throws IOException{
  if(!game.isDirectory()&&!game.mkdirs())throw new IOException("Cannot create game storage");
  ArrayList<String[]> rows=new ArrayList<>();HashSet<String> seen=new HashSet<>();long total=0;
  try(BufferedReader in=new BufferedReader(new InputStreamReader(source.open(".inventory.tsv"),StandardCharsets.UTF_8))){String line;
   while((line=in.readLine())!=null){String[] r=line.split("\\t",3);
    if(r.length!=3||!r[0].matches("[0-9a-f]{64}")||!r[1].matches("[0-9]{1,12}")||r[2].startsWith("/")||r[2].contains("\\")||r[2].contains(":"))throw new IOException("Invalid bundle inventory");
    long size=Long.parseLong(r[1]);if(size>4L*1024*1024*1024||total>8L*1024*1024*1024-size)throw new IOException("Bundle exceeds size limit");total+=size;
    for(String part:r[2].split("/",-1))if(part.isEmpty()||part.equals(".")||part.equals(".."))throw new IOException("Unsafe bundle path");
    if(!seen.add(r[2].toLowerCase(Locale.ROOT))||rows.size()>=20000)throw new IOException("Duplicate or excessive bundle files");rows.add(r);
   }
  }
  if(rows.isEmpty())throw new IOException("Empty bundle inventory");String prefix=game.getCanonicalPath()+File.separator;byte[] buffer=new byte[262144];int done=0;
  for(String[] r:rows){if(Thread.currentThread().isInterrupted())throw new IOException("Installation cancelled");File target=new File(game,r[2]);
   if(!target.getCanonicalPath().startsWith(prefix))throw new IOException("Bundle path escapes storage");String low=r[2].toLowerCase(Locale.ROOT);
   boolean keep=GameFiles.present(target)&&(low.startsWith("save/")||low.endsWith(".ini"));
   if(!keep){
    if(target.exists()&&!target.isFile())throw new IOException("Bundle destination is not a file");File parent=target.getParentFile();if(!parent.isDirectory()&&!parent.mkdirs())throw new IOException("Cannot create bundle folder");File tmp=File.createTempFile(".bundle-",".tmp",parent);
    try{MessageDigest hash;try{hash=MessageDigest.getInstance("SHA-256");}catch(NoSuchAlgorithmException e){throw new IOException(e);}long bytes=0;
     try(InputStream in=source.open(r[2]);FileOutputStream out=new FileOutputStream(tmp)){int n;while((n=in.read(buffer))!=-1){bytes+=n;if(bytes>Long.parseLong(r[1]))throw new IOException("Bundled file exceeds expected size");hash.update(buffer,0,n);out.write(buffer,0,n);}out.getFD().sync();}
     StringBuilder hex=new StringBuilder();for(byte b:hash.digest())hex.append(String.format(Locale.ROOT,"%02x",b&255));if(bytes!=Long.parseLong(r[1])||!r[0].equals(hex.toString()))throw new IOException("Bundled checksum mismatch: "+r[2]);
     try{Files.move(tmp.toPath(),target.toPath(),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);}catch(AtomicMoveNotSupportedException e){Files.move(tmp.toPath(),target.toPath(),StandardCopyOption.REPLACE_EXISTING);}
    }finally{tmp.delete();}
   }progress.update(r[2],++done,rows.size());
  }
 }
 private BundledGameInstaller(){}
}
