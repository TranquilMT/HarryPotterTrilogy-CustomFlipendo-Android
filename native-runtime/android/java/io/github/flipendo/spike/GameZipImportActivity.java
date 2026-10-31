package io.github.flipendo.spike;
import android.app.*;import android.os.*;import android.content.*;import android.net.Uri;import android.widget.*;import java.io.*;import java.util.*;

/** Imports original extracted game data to a new app-owned folder. */
public final class GameZipImportActivity extends Activity {
 private TextView status;private ProgressBar bar;private boolean busy;private Thread worker;private static final int PICK=64;
 public void onCreate(Bundle state){super.onCreate(state);LinearLayout col=new LinearLayout(this);col.setOrientation(1);col.setPadding(32,28,32,28);col.setBackgroundColor(0xff101b22);status=new TextView(this);status.setTextColor(0xffecdcb6);status.setTextSize(18);status.setText("Import original HP1 or HP2 game ZIP\n\nChoose the extracted Original Game Data ZIP, or the outer Chamber All Packs ZIP. HD texture packs alone do not contain the game. Chamber of Secrets is already bundled in this APK.");col.addView(status);bar=new ProgressBar(this);col.addView(bar);bar.setVisibility(8);Button choose=new Button(this);choose.setText("Choose game ZIP");choose.setOnClickListener(v->{if(!busy){Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);i.setType("*/*");i.addCategory(Intent.CATEGORY_OPENABLE);startActivityForResult(i,PICK);}});col.addView(choose);setContentView(col);}
 protected void onActivityResult(int code,int result,Intent data){super.onActivityResult(code,result,data);if(code!=PICK||result!=RESULT_OK||data==null||data.getData()==null)return;busy=true;bar.setVisibility(0);getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);Uri uri=data.getData();worker=new Thread(()->importZip(uri),"Original game ZIP import");worker.start();}
 private void update(String text){runOnUiThread(()->{if(!isFinishing())status.setText(text);});}
 private void importZip(Uri uri){File target=null;boolean committed=false;
  try{
   File base=getExternalFilesDir(null);if(base==null)base=getFilesDir();target=new File(base,"game-import-"+System.currentTimeMillis());if(target.exists())throw new IOException("Import directory already exists");
   update("Reading original game archive…");
   try(InputStream in=getContentResolver().openInputStream(uri)){
    if(in==null)throw new IOException("Cannot open selected ZIP");new GameZipImporter().extract(in,target,(file,bytes)->update("Importing "+file+"\n"+bytes/(1024*1024)+" MiB extracted"));
   }
   File one=GameFiles.find(false,target,2),two=GameFiles.find(true,target,2);
   if(one!=null&&two!=null)throw new IOException("Import one game's data at a time; HP1 and HP2 System files must remain separate");
   boolean hp2=two!=null;File game=hp2?two:one;if(game==null)throw new IOException("No supported game found. Choose an original game-data ZIP with System/HP.exe or System/Game.exe. Prisoner of Azkaban is not supported yet.");
   GameFiles.repairDefaults(hp2,game,(second,name)->getAssets().open("defaults/"+(second?"hp2":"hp1")+"/"+name));
   List<String> missing=GameFiles.missing(hp2,game);if(!missing.isEmpty())throw new IOException("Incomplete game archive. Missing: "+android.text.TextUtils.join(", ",missing));GameFiles.validatePackages(hp2,game);
   if(!getSharedPreferences("setup",MODE_PRIVATE).edit().putString(hp2?SetupActivity.PREF_GAME_DIR_HP2:SetupActivity.PREF_GAME_DIR,game.getAbsolutePath()).putString(SetupActivity.PREF_GAME,hp2?"hp2":"hp1").commit())throw new IOException("Cannot save the imported game location");
   committed=true;final String id=hp2?"hp2":"hp1";update("Import complete. Startup configuration checked.");
   runOnUiThread(()->new AlertDialog.Builder(this).setMessage("Game imported. Your other game folders and saves were kept.").setPositiveButton("Return to launcher",(d,w)->{startActivity(new Intent(this,SetupActivity.class).putExtra("game",id).addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP));finish();}).setCancelable(false).show());
  }catch(Exception error){update("Import failed: "+error.getMessage()+"\nChoose ZIP to try again.");}
  finally{if(!committed&&target!=null)deleteStaging(target);busy=false;runOnUiThread(()->{bar.setVisibility(8);getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);});}
 }
 private static void deleteStaging(File file){File[] children=file.listFiles();if(children!=null)for(File child:children)deleteStaging(child);file.delete();}
 public void onBackPressed(){if(busy)new AlertDialog.Builder(this).setMessage("Cancel this import?").setPositiveButton("Cancel import",(d,w)->{if(worker!=null)worker.interrupt();}).setNegativeButton("Keep importing",null).show();else super.onBackPressed();}
}
