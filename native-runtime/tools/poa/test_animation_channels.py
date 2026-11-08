"""Production channel timing and layered local-space pose composition."""
import pathlib,tempfile,subprocess,unittest,os
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class AnimationChannels(unittest.TestCase):
 def test_timing_events_tween_blend_masks_and_state_isolation(self):
  code=r'''#include "Package/PoaAnimationChannels.h"
#include <cassert>
using namespace PoaAssets;using namespace PoaChannels;
int main(){
 AnimationData data;data.Bones={{"root",0,0},{"child",0,0}};
 for(auto name:{"Walk","Raise"}){AnimationSequence seq;seq.Name=name;seq.Frames=4;seq.Rate=4;seq.Notifies={{.25f,"Step",0},{.5f,"None",7}};data.Sequences.push_back(seq);AnimationMove move;move.Duration=4;AnimationTrack root;root.Position={{0,0,0},{4,0,0}};root.Time={0,4};AnimationTrack child;child.Position={{1,0,0},{1,4,0}};child.Time={0,4};move.Tracks={root,child};data.Moves.push_back(move);}
 State a,b;Play(a,data,"walk",1,0,0,false);auto events=Tick(a,data,.5);assert(a.At(0).Phase==.5&&events.size()==2&&events[0].Function=="Step"&&events[1].ObjectReference==7);assert(b.At(0).Name.empty());
 events=Tick(a,data,.5);assert(!a.At(0).Playing&&a.At(0).Phase==.75&&events.size()==1&&events[0].End&&events[0].Channel==0);
 Play(a,data,"Walk",1,0,0,true);events=Tick(a,data,2.25);assert(a.At(0).Phase==.25&&events.size()==7);size_t ends=0;for(const auto&e:events)ends+=e.End;assert(ends==2);
 a.At(0).Loop=false;events=Tick(a,data,1);assert(!a.At(0).Playing&&a.At(0).Phase==.75);
 Play(a,data,"Raise",1,.5,0,false);Tick(a,data,.25);assert(a.At(0).Phase==0&&std::abs(a.At(0).Tween-.25)<1e-6);Tick(a,data,.5);assert(std::abs(a.At(0).Phase-.25)<1e-6&&a.At(0).Tween==0);
 SkeletalMeshData mesh;mesh.Bones={{"root",0,{0,0,0,1},{0,0,0},0,{0,0,0},1,0},{"child",0,{0,0,0,1},{1,0,0},0,{0,0,0},0,0}};
 State layered;Play(layered,data,"Walk",0,0,0,false);layered.At(0).Phase=.5;Play(layered,data,"Raise",0,0,1,false);layered.At(1).Phase=.75;Blend(layered,1,.5,0,"child");auto pose=Pose(mesh,data,layered);assert(std::abs(pose[0].Position[0]-2)<1e-5);assert(std::abs(pose[1].Position[0]-3)<1e-5&&std::abs(pose[1].Position[1]-2.5)<1e-5);
 layered.At(1).GlobalPose=true;auto global=Pose(mesh,data,layered);assert(std::abs(global[1].Position[0]-3.5)<1e-5&&std::abs(global[1].Position[1]-2.5)<1e-5);layered.At(1).GlobalPose=false;
 Blend(layered,1,0,1,"child");Tick(layered,data,.5);assert(std::abs(layered.At(1).Alpha-.25)<1e-6);Tick(layered,data,.5);assert(layered.At(1).Alpha==0);
 const auto generation=layered.At(1).Generation;Stop(layered,true);assert(layered.At(1).Generation>generation);assert(!layered.At(0).Playing&&layered.At(0).Name=="Walk"&&layered.At(1).Name.empty());
 bool bad=false;try{Play(a,data,"Missing",1,0,0,false);}catch(...){bad=true;}assert(bad);bad=false;try{a.At(16);}catch(...){bad=true;}assert(bad);bad=false;try{Tick(a,data,NAN);}catch(...){bad=true;}assert(bad);bad=false;try{Blend(a,0,2,0,"");}catch(...){bad=true;}assert(bad);
 State interrupted;Play(interrupted,data,"Walk",0,1,0,false,&mesh);interrupted.At(0).Phase=.5;Tick(interrupted,data,.5);auto before=Pose(mesh,data,interrupted);Play(interrupted,data,"Raise",0,1,0,false,&mesh);auto after=Pose(mesh,data,interrupted);assert(std::abs(before[0].Position[0]-after[0].Position[0])<1e-6);Tick(interrupted,data,.5);auto midway=Pose(mesh,data,interrupted);assert(std::abs(midway[0].Position[0]-.5)<1e-5);
 // Frozen tween uses source pose without advancing it, and transitions from bind.
 State tween;Play(tween,data,"Walk",0,1,0,false);tween.At(0).Phase=.5;Tick(tween,data,.5);pose=Pose(mesh,data,tween);assert(std::abs(pose[0].Position[0]-1)<1e-5);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
