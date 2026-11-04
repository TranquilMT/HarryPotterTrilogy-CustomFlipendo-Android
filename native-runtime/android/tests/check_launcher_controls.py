"""Data-free launcher/input regressions. Does not build an Android APK.

Compiles production GameFiles/ControlTiming, selected actual Overlay method bodies,
and the native HasGame implementation. Android activity sources are syntax-parsed
with javac; full SDK type checking and device acceptance remain separate.
"""
import os
import pathlib
import re
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
JAVA = ROOT / 'java/io/github/flipendo/spike'


def body(source, signature):
    begin = source.index(signature)
    start = source.index('{', begin)
    depth = 1
    end = start + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[begin:end]


def run(*args, **kwargs):
    subprocess.run(list(map(str, args)), check=True, **kwargs)


def main():
    setup = (JAVA / 'SetupActivity.java').read_text()
    activity = (JAVA / 'FlipendoActivity.java').read_text()
    request_ids = {name: int(value) for name, value in re.findall(r'(REQUEST_PICK_\w+)\s*=\s*(\d+)', setup)}
    assert len(request_ids.values()) == len(set(request_ids.values())), request_ids
    assert 'if ("hp3".equals(gameId))' in setup
    prep = body(setup, 'private void prepareHp3(')
    assert 'missingHp3' in prep and 'validatePackages("hp3"' in prep
    assert all(x not in prep for x in ('repairGameDefaults', 'installGameData', 'applyMod', 'applyVoice'))
    picker = setup[setup.index('if (requestCode == REQUEST_PICK_HP3)'):setup.index('if (requestCode == REQUEST_PICK_HP1 ||')]
    assert 'putString(PREF_GAME_DIR_HP3,' in picker and 'backToStart()' in picker
    assert 'if (action == MotionEvent.ACTION_CANCEL) { releaseAllControls(); return true; }' in activity
    assert 'releaseAllControls()' in body(activity, 'void openSettings()')
    assert 'setInputActive(false)' in body(activity, 'protected void onPause()')
    native = (ROOT / 'jni/android_main.cpp').read_text()
    assert 'explicitGameDir && gameDir.empty()' in native
    assert 'explicitGameDir ? FindGame(argv[1], 0) : FindGameDir()' in native
    print('PASS launcher routing, distinct picker IDs, cancellation and focus integration')

    with tempfile.TemporaryDirectory(prefix='launcher-controls-') as directory:
        tmp = pathlib.Path(directory)
        run('java', 'com.sun.tools.javac.Main', '-d', tmp,
            JAVA / 'GameFiles.java', JAVA / 'ControlTiming.java', ROOT / 'tests/ThreeGameLaunchTests.java')
        run('java', '-cp', tmp, 'io.github.flipendo.spike.ThreeGameLaunchTests')
        parse = tmp / 'ParseJava.java'
        parse.write_text('''import javax.tools.*;import com.sun.source.util.JavacTask;import java.util.*;
public class ParseJava {public static void main(String[] paths)throws Exception{
 DiagnosticCollector<JavaFileObject> d=new DiagnosticCollector<>();JavaCompiler c=ToolProvider.getSystemJavaCompiler();
 try(StandardJavaFileManager m=c.getStandardFileManager(d,null,null)){
 ((JavacTask)c.getTask(null,m,d,Arrays.asList("-proc:none"),null,m.getJavaFileObjects(paths))).parse();
 for(Diagnostic<?> error:d.getDiagnostics())if(error.getKind()==Diagnostic.Kind.ERROR)throw new AssertionError(error.toString());
 System.out.println("PASS Android activity syntax (SDK type checking not performed)");}}}''')
        run('java', 'com.sun.tools.javac.Main', '-d', tmp, parse)
        run('java', '-cp', tmp, 'ParseJava', JAVA / 'SetupActivity.java', JAVA / 'FlipendoActivity.java')

        methods = '\n'.join(body(activity, signature) for signature in (
            'void releaseAllControls()', 'void postInputDelayed(', 'void setInputActive(', 'void key(', 'void setButton('))
        overlay = tmp / 'OverlayChecks.java'
        overlay.write_text('''import java.util.*;
class MotionEvent {static final int ACTION_DOWN=0,ACTION_UP=1;}
class KeyEvent {static final int KEYCODE_SPACE=62,KEYCODE_ENTER=66;}
class SDLActivity {
 static int mask, events; static Set<Integer> held=new HashSet<>();
 static void onNativeMouse(int b,int a,float x,float y,boolean relative){mask=b;++events;}
 static void onNativeKeyUp(int code){held.remove(code);}static void onNativeKeyDown(int code){held.add(code);}
}
class FlipendoActivity {void openHp2GameMenu(){}}
public class OverlayChecks {
 static class Btn {int mouse,key,pointer=-1;boolean down;Btn(int mouse,int key){this.mouse=mouse;this.key=key;}}
 Btn cast=new Btn(1,0),jump=new Btn(2,0),use=new Btn(0,66),menu=new Btn(0,27),save=new Btn(0,0),gear=new Btn(0,0);
 Btn[] buttons={cast,jump,use,menu,save,gear};
 int inputEpoch,mouseButtons,stickPointer,aimPointer,navX,navY,activeSlider,layoutDragPointer;boolean padClicking;
 float aimDx,aimDy,padLx,padLy,padRx,padRy,padHatX,padHatY,padDrawX,padDrawY;
 long aimFrameTime,padFrameTime;boolean inputActive=true,settingsScrolling,noSkip,cutsceneShown,noQuickSave;
 boolean padCastKey,padCastBumper,padCastTrigger,padCasting,padA,padJumpBumper,padJumpTrigger,padJumping,padSkipHeld;
 boolean[] dirDown=new boolean[4];boolean hp3;
 String levelShown="";
 Set<Integer> pressedKeys=new HashSet<>(),twoTapDone=new HashSet<>(),arrowFingers=new HashSet<>();
 Map<Integer,Object> roles=new HashMap<>(),last=new HashMap<>(),start=new HashMap<>(),downAt=new HashMap<>();
 static final int[] DIR_KEYS={19,20,21,22},HP3_KEYS={51,47,29,32};
 List<Runnable> tasks=new ArrayList<>();Runnable poll=()->{},aimTick=()->{},padTick=()->{},navRepeat=()->{},padSkipFire=()->{},menuHeld;
 int saves,clicks;boolean touching;
 boolean isHp3Game(){return hp3;}boolean isHp2Game(){return false;}
 Object getContext(){return new FlipendoActivity();}void invalidate(){}void openSettings(){}
 void nativeRequestSave(){++saves;}void nativeSetTouch(float x,float y,boolean on){touching=on;}
 void post(Runnable r){tasks.add(r);}void postDelayed(Runnable r,long delay){tasks.add(r);}
 void removeCallbacks(Runnable r){tasks.removeIf(t->t==r);}
 void flush(){List<Runnable> old=new ArrayList<>(tasks);tasks.clear();for(Runnable r:old)r.run();}
''' + methods + '''
 static void ok(boolean value){if(!value)throw new AssertionError();}
 public static void main(String[] args){
  OverlayChecks o=new OverlayChecks();
  o.setButton(o.cast,true);o.setButton(o.jump,true);ok(SDLActivity.mask==3);
  o.setButton(o.jump,false);ok(SDLActivity.mask==1);int events=SDLActivity.events;
  o.setButton(o.cast,true);ok(events==SDLActivity.events);o.setButton(o.cast,false);ok(SDLActivity.mask==0);
  o.hp3=true;for(int key:DIR_KEYS)o.key(key,true);ok(SDLActivity.held.equals(new HashSet<>(Arrays.asList(51,47,29,32))));
  o.roles.put(7,new Object());o.roles.put(9,new Object());o.touching=true;
  o.postInputDelayed(()->o.clicks++,60);o.releaseAllControls();o.flush();
  ok(o.roles.isEmpty()&&o.pressedKeys.isEmpty()&&SDLActivity.held.isEmpty()&&!o.touching&&o.clicks==0);
  o.setButton(o.cast,true);o.setButton(o.use,true);o.postInputDelayed(()->o.clicks++,60);
  o.setInputActive(false);o.setInputActive(true);o.flush();ok(o.clicks==0&&SDLActivity.mask==0&&SDLActivity.held.isEmpty());
  o.setButton(o.jump,true);ok(SDLActivity.held.contains(62));o.setButton(o.jump,false);ok(SDLActivity.held.isEmpty());
  o.key(62,true);o.key(62,false);ok(SDLActivity.held.isEmpty());
  o.cutsceneShown=true;o.key(62,true);ok(SDLActivity.held.contains(66));o.key(62,false);ok(SDLActivity.held.isEmpty());
  o.setButton(o.save,true);ok(o.saves==0);
  System.out.println("PASS actual Overlay methods: mouse chords, repeat suppression, PoA keys, multi-input release and cancelled callbacks");
 }}''')
        run('java', 'com.sun.tools.javac.Main', '-d', tmp, overlay)
        run('java', '-cp', tmp, 'OverlayChecks')

        detector = tmp / 'detector.cpp'
        detector.write_text('''#include <string>\n#include <dirent.h>\n#include <strings.h>\n#include <sys/stat.h>\n#include <fstream>\n#include <cassert>\n#include <filesystem>\n'''
            + body(native, 'static bool HasGame(') + '''
int main(int argc,char** argv){assert(argc==2);std::string root=argv[1];std::filesystem::create_directories(root+"/sYsTeM");
 assert(!HasGame(root));std::ofstream(root+"/sYsTeM/HPPOA.EXE")<<"test";assert(HasGame(root));
 std::filesystem::remove(root+"/sYsTeM/HPPOA.EXE");std::ofstream(root+"/sYsTeM/hp.EXE")<<"test";assert(HasGame(root));
 std::filesystem::remove(root+"/sYsTeM/hp.EXE");std::ofstream(root+"/sYsTeM/GAME.exe")<<"test";assert(HasGame(root));
 std::filesystem::remove(root+"/sYsTeM/GAME.exe");std::filesystem::create_directory(root+"/sYsTeM/hppoa.exe");assert(!HasGame(root));
}''')
        binary = tmp / 'detector'
        run('g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined', detector, '-o', binary)
        run(binary, tmp / 'games', env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
        print('PASS actual native game detection: HP1/HP2/HP3, mixed casing and invalid executable directories')
    print('PASS host launcher/control checks; Android build and device tests still required')


if __name__ == '__main__':
    main()
