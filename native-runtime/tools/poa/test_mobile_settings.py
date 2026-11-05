import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
def body(s,signature):
 start=s.index(signature);brace=s.index('{',start);end=brace+1;depth=1
 while depth:
  depth+=(s[end]=='{')-(s[end]=='}');end+=1
 return s[start:end]
class MobileSettings(unittest.TestCase):
 def test_actual_frame_limit_policy(self):
  source=(ROOT/'src/hp1/mods/FrameLimit.cpp').read_text()
  f=body(source,'int FrameLimit()')
  program="""#include <atomic>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cassert>
struct Info{bool ue2;bool IsUE2(){return ue2;}};struct Engine{Info LaunchInfo;};Engine instance;Engine*engine=&instance;
struct Cmd{std::string GetArg(const char*,const char*,const char*){return "";}};Cmd*commandline=nullptr;bool mod=true;bool IsOn(const char*){return mod;}std::atomic<int> runtimeLimit{-1};
"""+f+"""
int main(){instance.LaunchInfo.ue2=true;mod=false;for(int n:{-1,0,15,25,30,45,60,90,120}){runtimeLimit=n;assert(FrameLimit()==(n<0?60:std::clamp(n,25,60)));}instance.LaunchInfo.ue2=false;runtimeLimit=90;assert(FrameLimit()==0);mod=true;assert(FrameLimit()==90);}
"""
  with tempfile.TemporaryDirectory() as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(program);subprocess.run(['g++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
 def test_runtime_settings_wiring(self):
  engine=(ROOT/'src/engine/SurrealEngine/Engine.cpp').read_text();java=ROOT/'android/java/io/github/flipendo/spike'
  self.assertIn('if (step == 0 && LaunchInfo.IsUE2()) HP1::Mods::TickFrameLimit();',engine)
  self.assertIn('else if (LaunchInfo.IsUE2())',engine)
  activity=(java/'FlipendoActivity.java').read_text();self.assertIn('(isHp3Game() ? 60 : FPS_LIMIT_MAX)',activity);self.assertIn('if (!isHp3Game()) drawSwitchRow(c, autoJumpBtn',activity)
if __name__=='__main__':unittest.main()
