/* SPDX-License-Identifier: GPL-3.0-or-later */
package io.github.flipendo.spike;
import java.io.*;
import java.nio.charset.Charset;
import java.util.*;
/** Bounded ISO9660/Joliet reader. Extracts only game directories, never installers. */
public final class IsoExtractor {
 public interface Progress {void update(String file,long done,long total);}
 public static final class Entry {public final String path;public final long offset,size;Entry(String p,long o,long s){path=p;offset=o;size=s;}}
 private final RandomAccessFile iso;private final long length;private final Set<Long> visited=new HashSet<Long>();private final ArrayList<Entry> entries=new ArrayList<Entry>();private boolean joliet;
 public IsoExtractor(RandomAccessFile f)throws IOException{iso=f;length=f.length();}
 private static long le(byte[] b,int i){return (b[i]&255L)|((b[i+1]&255L)<<8)|((b[i+2]&255L)<<16)|((b[i+3]&255L)<<24);}
 private void range(long off,long size)throws IOException{if(off<0||size<0||off>length||size>length-off)throw new IOException("Disc record extends outside ISO");}
 public List<Entry> list()throws IOException{
  entries.clear();visited.clear();byte[] chosen=null;boolean found=false;
  for(int sector=16;sector<80;sector++){
   byte[] b=new byte[2048];range(sector*2048L,2048);iso.seek(sector*2048L);iso.readFully(b);
   if(!new String(b,1,5,Charset.forName("US-ASCII")).equals("CD001"))throw new IOException("Not an ISO9660 disc");
   int type=b[0]&255;if(type==255)break;
   if(type==1 && chosen==null){chosen=b;joliet=false;}
   if(type==2 && b[88]==37&&b[89]==47&&(b[90]==64||b[90]==67||b[90]==69)){chosen=b;joliet=true;}
   found=true;
  }
  if(!found||chosen==null)throw new IOException("No supported disc filesystem");
  if((chosen[128]&255)!=0||(chosen[129]&255)!=8)throw new IOException("Unsupported sector size");
  directory(le(chosen,158)*2048,le(chosen,166),"",0);return Collections.unmodifiableList(entries);
 }
 private void directory(long off,long size,String parent,int depth)throws IOException{
  range(off,size);if(depth>16||size>16*1024*1024||!visited.add(off))throw new IOException("Invalid directory tree");byte[] data=new byte[(int)size];iso.seek(off);iso.readFully(data);int pos=0;
  while(pos<data.length){int n=data[pos]&255;if(n==0){pos=(pos/2048+1)*2048;continue;}
   if(n<34||pos+n>data.length)throw new IOException("Invalid directory record");int names=data[pos+32]&255;
   if(names==0||33+names>n)throw new IOException("Invalid filename record");
   if(names==1&&(data[pos+33]==0||data[pos+33]==1)){pos+=n;continue;}
   if(joliet&&(names&1)!=0)throw new IOException("Invalid Joliet filename");
   String name=new String(data,pos+33,names,Charset.forName(joliet?"UTF-16BE":"US-ASCII"));int semi=name.indexOf(';');if(semi>=0)name=name.substring(0,semi);
   if(name.isEmpty()||name.equals(".")||name.equals("..")||name.indexOf('/')>=0||name.indexOf('\\')>=0||name.indexOf('\0')>=0||name.indexOf(':')>=0)throw new IOException("Unsafe disc filename");
   long start=le(data,pos+2)*2048,bytes=le(data,pos+10);range(start,bytes);int flags=data[pos+25]&255;
   if((flags&128)!=0)throw new IOException("Multi-extent ISO files are not supported");String path=parent+"/"+name;
   if((flags&2)!=0)directory(start,bytes,path,depth+1);else {entries.add(new Entry(path,start,bytes));if(entries.size()>20000)throw new IOException("Too many disc files");}pos+=n;
  }
 }
 public static boolean gameFile(String p){String l=p.toLowerCase(Locale.ROOT);return l.startsWith("/system/")||l.startsWith("/maps/")||l.startsWith("/textures/")||l.startsWith("/sounds/")||l.startsWith("/music/")||l.startsWith("/help/");}
 public long extract(File dest,Progress progress)throws IOException{
  List<Entry> all=list();long total=0;boolean exe=false,pack=false,map=false;
  for(Entry e:all){String l=e.path.toLowerCase(Locale.ROOT);if(l.equals("/system/hp.exe"))exe=true;if(l.equals("/system/harrypotter.u"))pack=true;if(l.startsWith("/maps/")&&l.endsWith(".unr"))map=true;if(gameFile(e.path))total+=e.size;}
  if(!exe||!pack||!map)throw new IOException("This is not a supported Harry Potter 1 PC disc");
  if(!dest.isDirectory()&&!dest.mkdirs())throw new IOException("Cannot create import directory");if(dest.getUsableSpace()<total+32*1024*1024)throw new IOException("Not enough free space for game data");
  String root=dest.getCanonicalPath()+File.separator;byte[] buf=new byte[262144];long done=0;
  for(Entry e:all){if(!gameFile(e.path))continue;File target=new File(dest,e.path.substring(1));if(!target.getCanonicalPath().startsWith(root)||target.exists())throw new IOException("Import destination is not empty");File dir=target.getParentFile();if(!dir.isDirectory()&&!dir.mkdirs())throw new IOException("Cannot create game folder");iso.seek(e.offset);
   try(OutputStream out=new BufferedOutputStream(new FileOutputStream(target))){long left=e.size;while(left>0){if(Thread.currentThread().isInterrupted())throw new IOException("Import cancelled");int n=(int)Math.min(buf.length,left);iso.readFully(buf,0,n);out.write(buf,0,n);left-=n;done+=n;} }
   progress.update(e.path,done,total);
  }return total;
 }
}
