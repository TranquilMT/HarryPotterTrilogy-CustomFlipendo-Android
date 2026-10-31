package io.github.flipendo.spike;
import java.io.*;import java.nio.file.*;import java.security.*;import java.util.*;import java.util.zip.*;import org.json.*;
/** Stage and verify profile-labelled DDS modules before modifying loose overrides. */
public final class HDPackInstaller {
 public interface Source { InputStream open() throws IOException; }
 public interface Progress { void report(String text); }
 static final String MANIFEST=".flipendo-texpacks.list";
 static String safe(String p)throws IOException{if(!p.startsWith("Textures/")||!p.endsWith(".dds")||p.indexOf('\\')>=0||p.indexOf('\0')>=0)throw new IOException("Unsafe texture path: "+p);for(String c:p.split("/",-1))if(c.isEmpty()||c.equals(".")||c.equals(".."))throw new IOException("Unsafe texture path");return p;}
 static File target(File root,String p)throws IOException{File f=new File(root,safe(p));if(!f.getCanonicalPath().startsWith(root.getCanonicalPath()+File.separator))throw new IOException("Texture path leaves game folder");return f;}
 static Set<String> installed(File root)throws IOException{Set<String> result=new TreeSet<>();File f=new File(root,MANIFEST);if(f.isFile())try(BufferedReader r=new BufferedReader(new FileReader(f))){String s;while((s=r.readLine())!=null)if(!s.trim().isEmpty())result.add(safe(s.trim()));}return result;}
 static void check()throws IOException{if(Thread.currentThread().isInterrupted())throw new IOException("Installation cancelled");}
 static byte[] bytes(InputStream in,int limit)throws IOException{ByteArrayOutputStream out=new ByteArrayOutputStream();byte[] b=new byte[65536];int n;while((n=in.read(b))!=-1){check();if(out.size()+n>limit)throw new IOException("Pack entry exceeds limit");out.write(b,0,n);}return out.toByteArray();}
 static String sha(byte[] b)throws IOException{try{byte[] h=MessageDigest.getInstance("SHA-256").digest(b);StringBuilder s=new StringBuilder();for(byte x:h)s.append(String.format(Locale.ROOT,"%02x",x&255));return s.toString();}catch(NoSuchAlgorithmException e){throw new IOException(e);}}
 static int u32(byte[] b,int p){return (b[p]&255)|((b[p+1]&255)<<8)|((b[p+2]&255)<<16)|((b[p+3]&255)<<24);}
 static void dds(byte[] b)throws IOException{if(b.length<128||b[0]!='D'||b[1]!='D'||b[2]!='S'||b[3]!=' '||u32(b,4)!=124)throw new IOException("Invalid DDS header");int h=u32(b,12),w=u32(b,16),m=u32(b,28);String fmt=new String(b,84,4,java.nio.charset.StandardCharsets.US_ASCII);if(w<1||h<1||w>8192||h>8192||(w&(w-1))!=0||(h&(h-1))!=0||m<1||m>14||!(fmt.equals("DXT1")||fmt.equals("DXT3")||fmt.equals("DXT5")))throw new IOException("Unsupported DDS dimensions/format");long size=128;int levels=0;while(true){size+=(long)Math.max(1,(w+3)/4)*Math.max(1,(h+3)/4)*(fmt.equals("DXT1")?8:16);levels++;if(w==1&&h==1)break;w=Math.max(1,w/2);h=Math.max(1,h/2);}if(m!=levels||size!=b.length)throw new IOException("Incomplete DDS mip chain");}
 static final class Scan {final File stage;final String game;final Progress progress;final Map<String,String> files=new TreeMap<>();final Set<String> folded=new HashSet<>();long total;int entries;Scan(File s,boolean hp2,Progress p){stage=s;game=hp2?"HP2":"HP1";progress=p;}}
 static void scan(InputStream input,Scan s,int depth)throws Exception{
  if(depth>2)throw new IOException("Too many nested ZIPs");ZipInputStream z=new ZipInputStream(new FilterInputStream(input){public void close(){}});Map<String,JSONObject> expected=null;Set<String> seen=new HashSet<>();ZipEntry e;
  while((e=z.getNextEntry())!=null){check();if(++s.entries>30000)throw new IOException("Too many pack entries");String name=e.getName();if(name.startsWith("/")||name.indexOf('\\')>=0||Arrays.asList(name.split("/")).contains(".."))throw new IOException("Unsafe ZIP path");if(e.isDirectory())continue;
   if(name.equals("pack.json")){
    JSONObject meta=new JSONObject(new String(bytes(z,16*1024*1024),java.nio.charset.StandardCharsets.UTF_8));String game=meta.optString("game","");
    if(!game.equals("HP1")&&!game.equals("HP2"))throw new IOException("Pack needs HP1/HP2 compatibility metadata. Use the supplied updated packs.");if(!game.equals(s.game))return;
    expected=new HashMap<>();JSONArray rows=meta.getJSONArray("textures");for(int i=0;i<rows.length();i++){JSONObject row=rows.getJSONObject(i);String p=safe(row.getString("path"));if(expected.put(p,row)!=null)throw new IOException("Duplicate manifest path");}
   }else if(name.toLowerCase(Locale.ROOT).endsWith(".zip")){scan(z,s,depth+1);
   }else if(name.toLowerCase(Locale.ROOT).endsWith(".dds")){
    if(expected==null)throw new IOException("DDS module has no compatibility manifest");safe(name);JSONObject row=expected.get(name);if(row==null||!seen.add(name))throw new IOException("Unexpected or duplicate texture: "+name);
    byte[] data=bytes(z,128*1024*1024);s.total+=data.length;if(s.total>8L*1024*1024*1024)throw new IOException("Packs exceed 8 GiB");String hash=sha(data);if(!hash.equals(row.getString("sha256")))throw new IOException("Texture checksum mismatch: "+name);dds(data);
    String old=s.files.get(name);if(old!=null){if(!old.equals(hash))throw new IOException("Conflicting pack versions: "+name);continue;}if(!s.folded.add(name.toLowerCase(Locale.ROOT)))throw new IOException("Case-conflicting texture paths");File dest=target(s.stage,name);if(!dest.getParentFile().isDirectory()&&!dest.getParentFile().mkdirs())throw new IOException("Cannot create texture folder");try(FileOutputStream out=new FileOutputStream(dest)){out.write(data);}s.files.put(name,hash);s.progress.report("Verified "+s.files.size()+" textures: "+name);
   }
   z.closeEntry();
  }
  if(expected!=null&&!seen.equals(expected.keySet()))throw new IOException("Incomplete texture module");
 }
 static void move(File from,File to)throws IOException{if(!to.getParentFile().isDirectory()&&!to.getParentFile().mkdirs())throw new IOException("Cannot create "+to.getParent());Files.move(from.toPath(),to.toPath(),StandardCopyOption.REPLACE_EXISTING);}
 static void manifest(File root,Set<String> paths)throws IOException{File temp=File.createTempFile(".hd-list-",".tmp",root);try{try(FileOutputStream out=new FileOutputStream(temp)){for(String p:paths)out.write((p+"\n").getBytes(java.nio.charset.StandardCharsets.UTF_8));out.getFD().sync();}Files.move(temp.toPath(),new File(root,MANIFEST).toPath(),StandardCopyOption.REPLACE_EXISTING);}finally{temp.delete();}}
 public static int install(File root,boolean hp2,List<Source> sources,File cache,Progress progress)throws Exception{
  File stage=Files.createTempDirectory(cache.toPath(),"hd-stage-").toFile();File backup=Files.createTempDirectory(root.toPath(),".hd-backup-").toFile();List<String> replaced=new ArrayList<>(),written=new ArrayList<>();Set<String> original=installed(root);Scan s=new Scan(stage,hp2,progress);
  try{for(Source source:sources)try(InputStream in=source.open()){if(in==null)throw new IOException("Cannot open pack");scan(in,s,0);}if(s.files.isEmpty())throw new IOException("No "+s.game+" texture modules found");check();
   try{for(String p:s.files.keySet()){File dest=target(root,p);if(dest.exists()){if(!dest.isFile())throw new IOException("Texture destination is a directory");move(dest,target(backup,p));replaced.add(p);}move(target(stage,p),dest);written.add(p);}Set<String> all=new TreeSet<>(original);all.addAll(s.files.keySet());manifest(root,all);
   }catch(Exception ex){for(String p:written)target(root,p).delete();for(String p:replaced)move(target(backup,p),target(root,p));throw ex;}return s.files.size();
  }finally{erase(stage);erase(backup);}
 }
 public static void remove(File root)throws IOException{Set<String> paths=installed(root);for(String p:paths){File f=target(root,p);if(f.exists()&&(!f.isFile()||!f.delete()))throw new IOException("Could not remove "+p);}manifest(root,Collections.emptySet());}
 static void erase(File f){File[] list=f.listFiles();if(list!=null)for(File c:list)erase(c);f.delete();}
}
