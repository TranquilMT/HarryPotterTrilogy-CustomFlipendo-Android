import java.io.*;import io.github.flipendo.spike.IsoExtractor;
public class ExtractDisc {public static void main(String[] a)throws Exception {try(RandomAccessFile r=new RandomAccessFile(a[0],"r")){long n=new IsoExtractor(r).extract(new File(a[1]),(f,d,t)->System.out.println(d+"/"+t+" "+f));System.out.println("Extracted "+n+" bytes");}}}
