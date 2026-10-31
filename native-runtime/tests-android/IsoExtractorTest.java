import io.github.flipendo.spike.IsoExtractor;import java.io.*;import java.nio.file.*;
public class IsoExtractorTest {
 public static void main(String[] args)throws Exception{
  File base=new File(args[0]);File out=new File(base,"out-"+System.nanoTime());
  try(RandomAccessFile r=new RandomAccessFile(new File(base,"valid.iso"),"r")){long n=new IsoExtractor(r).extract(out,(f,d,t)->{});if(n!=12)throw new AssertionError("size");}
  if(!new String(Files.readAllBytes(new File(out,"System/HP.exe").toPath()),"US-ASCII").equals("EXE"))throw new AssertionError("content");
  try(RandomAccessFile r=new RandomAccessFile(new File(base,"valid.iso"),"r")){new IsoExtractor(r).extract(out,(f,d,t)->{});throw new AssertionError("overwrite allowed");}catch(IOException expected){}
  for(String name:new String[]{"outside.iso","traversal.iso","truncated.iso"})try(RandomAccessFile r=new RandomAccessFile(new File(base,name),"r")){new IsoExtractor(r).list();throw new AssertionError(name+" accepted");}catch(IOException expected){}
  System.out.println("PASS: extraction bytes; existing-file protection; extent bounds; traversal; truncation");
 }
}
