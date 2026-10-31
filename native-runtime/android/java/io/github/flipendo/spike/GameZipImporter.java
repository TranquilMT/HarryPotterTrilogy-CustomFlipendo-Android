package io.github.flipendo.spike;
import java.io.*;import java.util.*;import java.util.zip.*;

/** Extracted PC game ZIPs, including our outer bundle with nested game-data ZIP. */
public final class GameZipImporter {
 public interface Progress { void update(String file,long bytes); }
 private final byte[] buffer=new byte[262144];private final Set<String> paths=new HashSet<>();
 private long readBytes,writtenBytes;private int entries;
 public int extract(InputStream source,File staging,Progress progress) throws IOException {
  if(!staging.isDirectory()&&!staging.mkdirs())throw new IOException("Cannot create import storage");
  unzip(source,staging,progress,0);return paths.size();
 }
 private void account(int count,boolean written) throws IOException {
  readBytes+=count;if(written)writtenBytes+=count;
  if(readBytes>8L*1024*1024*1024||writtenBytes>4L*1024*1024*1024)throw new IOException("Archive exceeds import size limit");
  if(Thread.currentThread().isInterrupted())throw new IOException("Import cancelled");
 }
 private void unzip(InputStream source,File staging,Progress progress,int depth) throws IOException {
  ZipInputStream zip=new ZipInputStream(new BufferedInputStream(source,262144));ZipEntry entry;
  while((entry=zip.getNextEntry())!=null){
   if(++entries>20000)throw new IOException("Archive contains too many entries");
   String name=entry.getName().replace('\\','/');
   if(name.startsWith("/")||name.matches("^[A-Za-z]:.*"))throw new IOException("Unsafe absolute archive path");
   String[] parts=name.split("/");for(String p:parts)if(p.equals(".."))throw new IOException("Unsafe parent archive path");
   if(entry.isDirectory()){zip.closeEntry();continue;}
   String lower=name.toLowerCase(Locale.ROOT);
   if(lower.endsWith("original-game-data.zip")){
    if(depth>=1)throw new IOException("Nested game archive depth exceeded");
    unzip(new FilterInputStream(zip){public void close(){}},staging,progress,depth+1);
   }else{
    int at=-1;String folder=null;
    for(int i=0;i<parts.length-1&&at<0;i++)for(String allowed:new String[]{"System","Maps","Textures","Sounds","Music","Save"})if(parts[i].equalsIgnoreCase(allowed)){at=i;folder=allowed;break;}
    if(at>=0){
     StringBuilder relative=new StringBuilder(folder);for(int i=at+1;i<parts.length;i++){if(parts[i].isEmpty()||parts[i].equals("."))throw new IOException("Invalid archive path");relative.append('/').append(parts[i]);}
     if(!paths.add(relative.toString().toLowerCase(Locale.ROOT)))throw new IOException("Duplicate game file in archive: "+relative);
     File target=new File(staging,relative.toString());if(!target.getCanonicalPath().startsWith(staging.getCanonicalPath()+File.separator))throw new IOException("Archive path escapes game directory");
     if(!target.getParentFile().isDirectory()&&!target.getParentFile().mkdirs())throw new IOException("Cannot create game folder");
     try(OutputStream out=new BufferedOutputStream(new FileOutputStream(target))){int n;while((n=zip.read(buffer))!=-1){account(n,true);out.write(buffer,0,n);}}
     progress.update(relative.toString(),writtenBytes);
    }
   }
   int n;while((n=zip.read(buffer))!=-1)account(n,false);zip.closeEntry();
  }
 }
}
