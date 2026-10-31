/* SPDX-License-Identifier: GPL-3.0-or-later */
package io.github.flipendo.spike;
import android.app.*;import android.os.*;import android.content.*;import android.net.Uri;import android.widget.*;import java.io.*;
/** Imports the user's own PC disc via the system picker into a new app-specific folder. */
public final class IsoImportActivity extends Activity {
 private TextView status;private ProgressBar bar;private boolean busy;private Thread worker;private static final int PICK=44;
 public void onCreate(Bundle state){super.onCreate(state);LinearLayout col=new LinearLayout(this);col.setOrientation(1);col.setPadding(40,35,40,35);col.setBackgroundColor(0xff101b22);status=new TextView(this);status.setTextColor(0xffecdcb6);status.setTextSize(20);status.setText("Import your Harry Potter 1 PC ISO\n\nChoose the original disc image. Keep this screen open while it copies and extracts.");col.addView(status);bar=new ProgressBar(this,null,android.R.attr.progressBarStyleHorizontal);bar.setMax(1000);col.addView(bar);Button choose=new Button(this);choose.setText("Choose ISO");choose.setOnClickListener(v->{if(!busy){Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);i.setType("*/*");i.addCategory(Intent.CATEGORY_OPENABLE);startActivityForResult(i,PICK);}});col.addView(choose);setContentView(col);}
 protected void onActivityResult(int code,int result,Intent data){super.onActivityResult(code,result,data);if(code!=PICK||result!=RESULT_OK||data==null||data.getData()==null)return;busy=true;getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);Uri uri=data.getData();worker=new Thread(()->importDisc(uri),"HP1 ISO import");worker.start();}
 private void update(String text,int progress){runOnUiThread(()->{if(!isFinishing()){status.setText(text);bar.setProgress(progress);}});}
 private void importDisc(Uri uri){File temp=new File(getCacheDir(),"hp1-import-"+System.currentTimeMillis()+".iso");File target=null;
  try{
   update("Copying ISO for extraction…",0);long total=-1;try(android.content.res.AssetFileDescriptor afd=getContentResolver().openAssetFileDescriptor(uri,"r")){if(afd!=null)total=afd.getLength();}
   long copied=0;try(InputStream in=getContentResolver().openInputStream(uri);OutputStream out=new BufferedOutputStream(new FileOutputStream(temp))){if(in==null)throw new IOException("Cannot open the selected ISO");byte[] b=new byte[262144];int n;while((n=in.read(b))!=-1){if(Thread.currentThread().isInterrupted())throw new IOException("Import cancelled");copied+=n;if(copied>8L*1024*1024*1024)throw new IOException("ISO exceeds 8 GiB limit");out.write(b,0,n);if(copied%(4*1024*1024)<b.length)update("Copying ISO: "+copied/(1024*1024)+" MiB",total>0?(int)Math.min(400,copied*400/total):0);}}
   File base=getExternalFilesDir(null);if(base==null)throw new IOException("App storage is unavailable");target=new File(base,"HP1-"+System.currentTimeMillis());final File destination=target;
   try(RandomAccessFile f=new RandomAccessFile(temp,"r")){new IsoExtractor(f).extract(destination,(file,done,bytes)->update("Extracting "+file+"\n"+(done*100/Math.max(1,bytes))+"%",400+(int)(done*600/Math.max(1,bytes))));}
   GameFiles.repairDefaults(false,destination,(hp2,name)->getAssets().open("defaults/hp1/"+name));
   if(!GameFiles.missing(false,destination).isEmpty())throw new IOException("Missing original files: "+GameFiles.missing(false,destination));
   GameFiles.validatePackages(false,destination);
   getSharedPreferences("setup",MODE_PRIVATE).edit().putString(SetupActivity.PREF_GAME_DIR,destination.getAbsolutePath()).putString("game","hp1").commit();
   update("Import complete. Return to the launcher and press Play.",1000);runOnUiThread(()->new AlertDialog.Builder(this).setMessage("Game imported successfully.").setPositiveButton("Return to launcher",(d,w)->{Intent i=new Intent(this,SetupActivity.class);i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);startActivity(i);finish();}).setCancelable(false).show());
  }catch(Exception e){final File failed=target;if(failed!=null)deleteImport(failed);update("Import failed: "+e.getMessage()+"\nChoose ISO to try again.",0);}
  finally{temp.delete();busy=false;runOnUiThread(()->getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));}
 }
 // Only used for the new unique staging directory created by this activity.
 private static void deleteImport(File f){File[] children=f.listFiles();if(children!=null)for(File c:children)deleteImport(c);f.delete();}
 public void onBackPressed(){if(busy)new AlertDialog.Builder(this).setMessage("Cancel this import?").setPositiveButton("Cancel import",(d,w)->{if(worker!=null)worker.interrupt();}).setNegativeButton("Keep importing",null).show();else super.onBackPressed();}
}
