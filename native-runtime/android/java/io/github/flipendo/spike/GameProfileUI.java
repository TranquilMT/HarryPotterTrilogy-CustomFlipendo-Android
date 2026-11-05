package io.github.flipendo.spike;
import android.app.*;import android.content.*;import android.widget.*;
/** Port settings available before native startup; profiles are independent per game. */
final class GameProfileUI {
 static void show(Activity a,String game,Runnable applied){
  if(!GameFiles.supported(game))throw new IllegalArgumentException("Unknown game");
  SharedPreferences p=a.getSharedPreferences("performance."+game,Context.MODE_PRIVATE);
  boolean hp3="hp3".equals(game);
  new AlertDialog.Builder(a).setTitle(GameFiles.title(game)+" — Settings")
   .setItems(new String[]{"Display","Controls","Diagnostics","Performance presets"},(d,w)->{
    if(w==0)display(a,game,p,applied);else if(w==1)controls(a,game,p,applied);else if(w==2)diagnostics(a,game,p,applied);else presets(a,game,p,applied);
   }).setNegativeButton("Done",null).show();
 }
 static void pick(Activity a,String game,String title,String[] labels,int current,java.util.function.IntConsumer save,Runnable applied){
  new AlertDialog.Builder(a).setTitle(title).setSingleChoiceItems(labels,Math.max(0,Math.min(labels.length-1,current)),(d,w)->{save.accept(w);d.dismiss();if(applied!=null)applied.run();show(a,game,applied);}).setNegativeButton("Back",(d,w)->show(a,game,applied)).show();
 }
 static int nearest(float[] values,float value){int best=0;for(int i=1;i<values.length;i++)if(Math.abs(values[i]-value)<Math.abs(values[best]-value))best=i;return best;}
 static void display(Activity a,String game,SharedPreferences p,Runnable cb){
  new AlertDialog.Builder(a).setTitle("Display — "+GameFiles.title(game)).setItems(new String[]{"Render resolution scale","Frame rate limit","Field of view offset","FPS counter"},(d,w)->{
   if(w==0){float[] v={.25f,.35f,.5f,.65f,.75f,1f};pick(a,game,"Render scale",new String[]{"25%","35% — cooler","50% — balanced","65%","75% — quality","100% — native"},nearest(v,p.getFloat("renderScale",.5f)),i->p.edit().putBoolean("initialized",true).putFloat("renderScale",v[i]).apply(),cb);}
   else if(w==1){float[] v="hp3".equals(game)?new float[]{25,30,40,45,60}:new float[]{25,30,40,45,60,90,120};String[] labels=new String[v.length];for(int i=0;i<v.length;i++)labels[i]=(int)v[i]+" FPS";pick(a,game,"Frame rate limit",labels,nearest(v,p.getInt("fpsLimit",60)),i->p.edit().putBoolean("initialized",true).putInt("fpsLimit",(int)v[i]).apply(),cb);}
   else if(w==2){float[] v={-20,-10,0,10,20,30,40};pick(a,game,"Field of view",new String[]{"-20°","-10°","Default","+10°","+20°","+30°","+40°"},nearest(v,p.getFloat("fovOffset",0)),i->p.edit().putBoolean("initialized",true).putFloat("fovOffset",v[i]).apply(),cb);}
   else toggle(a,game,p,"showFps","FPS counter",cb);
  }).setNegativeButton("Back",(d,w)->show(a,game,cb)).show();
 }
 static void controls(Activity a,String game,SharedPreferences p,Runnable cb){
  new AlertDialog.Builder(a).setTitle("Controls — "+GameFiles.title(game)).setItems(new String[]{"Movement: stick / D-pad","Invert vertical aim","Aim sensitivity","Disable cutscene skip"},(d,w)->{
   if(w==0)toggle(a,game,p,"dpad","Use D-pad",cb);else if(w==1)toggle(a,game,p,"invertWand","Invert vertical aim",cb);else if(w==3)toggle(a,game,p,"noSkip","Disable cutscene skip",cb);
   else {float[] v={.25f,.5f,.75f,1,1.5f,2,3};pick(a,game,"Aim sensitivity",new String[]{"25%","50%","75%","100%","150%","200%","300%"},nearest(v,p.getFloat("wandSens",1)),i->p.edit().putBoolean("initialized",true).putFloat("wandSens",v[i]).apply(),cb);}
  }).setNegativeButton("Back",(d,w)->show(a,game,cb)).show();
 }
 static void toggle(Activity a,String game,SharedPreferences p,String key,String label,Runnable cb){pick(a,game,label,new String[]{"Off","On"},p.getBoolean(key,false)?1:0,i->p.edit().putBoolean("initialized",true).putBoolean(key,i==1).apply(),cb);}
 static void diagnostics(Activity a,String game,SharedPreferences p,Runnable cb){new AlertDialog.Builder(a).setTitle("Diagnostics").setItems(new String[]{"Performance timing log","Export launcher report"},(d,w)->{if(w==0)toggle(a,game,p,"perfLog","Performance timing log",cb);else LogReport.share(a);}).setNegativeButton("Back",(d,w)->show(a,game,cb)).show();}
 static void presets(Activity a,String game,SharedPreferences p,Runnable cb){new AlertDialog.Builder(a).setTitle("Performance preset").setItems(new String[]{"Battery: 35% resolution · 30 FPS","Balanced: 50% resolution · 60 FPS","Quality: 75% resolution · 60 FPS"},(d,w)->{p.edit().putBoolean("initialized",true).putFloat("renderScale",w==0?.35f:w==1?.5f:.75f).putInt("fpsLimit",w==0?30:60).apply();if(cb!=null)cb.run();show(a,game,cb);}).setNegativeButton("Back",(d,w)->show(a,game,cb)).show();}
 private GameProfileUI(){}
}
