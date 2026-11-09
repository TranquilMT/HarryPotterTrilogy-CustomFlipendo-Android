"""Production actor handlers with object/event/latent adapters; EU ABI audit."""
import pathlib,tempfile,subprocess,unittest,os,re,json
from inspect_packages import inspect
from object_payloads import function_header,native_stub
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine';SOURCE=ROOT/'src/hp3/PoaActorAnimation.cpp'
class ActorAnimationHandlers(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original declarations')
 def test_native_arity_and_indices_match_original_eu(self):
  source=SOURCE.read_text();registrations={name:(int(arity),int(index))for arity,name,index in re.findall(r'RegisterVMNativeFunc_(\d+)\("Actor","([^"]+)",&\w+,(\d+)\)',source)}
  self.assertEqual(len(registrations),22)
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u';m=inspect(p);raw=p.read_bytes();seen=set()
  # Parameter count comes from native bytecode parameter references; bool return
  # uses an additional output slot in this engine's native adapter convention.
  returns={'IsAnimating','HasAnim','GetAnimSequence','GetAnimNumFrames','AnimIsInGroup','GetAnimTime','GetAnimFrame','GetAnimRate','GetRootRotationDelta'}
  for e in m['exports']:
   if e['class']!='Function' or e['name'] not in registrations or e['outer']<=0 or m['exports'][e['outer']-1]['name']!='Actor':continue
   r,h=function_header(raw[e['offset']:e['offset']+e['size']],m['names']);stub=native_stub(r,h);self.assertEqual(registrations[e['name']],(len(stub['parameters'])+(e['name'] in returns),stub['nativeIndex']));seen.add(e['name'])
  self.assertEqual(seen,set(registrations))
 def test_playback_queries_linking_events_and_latent_completion(self):
  body='\n'.join(line for line in SOURCE.read_text().splitlines()if not line.startswith('#include'))
  for signature in ('void Load(UActor* actor,ObjectStream* stream)','void Save(UActor* actor,PackageStreamWriter* stream)'):body=body.replace(definition(body,signature),'')
  code=r'''#include "Package/PoaAnimationChannels.h"
#include <memory>
#include <optional>
#include <cassert>
#include <map>
#include <functional>
using NameString=struct Name{std::string Text;Name(const char*s):Text(s){}Name(const std::string&s):Text(s){}std::string ToString()const{return Text;}};
struct Rotator{int Pitch=0,Yaw=0,Roll=0;};struct Package;struct UObject{virtual~UObject()=default;Package*package=nullptr;void LoadNow(){}template<class T>static T*Cast(UObject*p){auto v=dynamic_cast<T*>(p);if(p&&!v)throw std::runtime_error("cast");return v;}template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}};
struct Package{std::map<int,UObject*>Objects;UObject*GetUObject(int i){return Objects[i];}};struct UMesh:UObject{};
struct UPoaMeshAnimation:UObject{PoaAssets::AnimationData Animation;};struct UPoaSkeletalMesh:UMesh{PoaAssets::SkeletalMeshData Geometry;UPoaMeshAnimation*AnimationResource=nullptr;};
enum class LatentRunState{Continue,FinishAnim};struct StateFrameData{LatentRunState State=LatentRunState::Continue;void SetLatentState(LatentRunState s){State=s;}LatentRunState GetLatentState(){return State;}};
struct UActor:UObject{std::shared_ptr<PoaChannels::State>PoaAnimation;UObject*PoaAnimationObject=nullptr;UMesh*MeshObject=nullptr;UMesh*&Mesh(){return MeshObject;}StateFrameData*StateFrame=nullptr;int Default=0;bool HasProperty(const char*){return true;}int GetInt(const char*){return Default;}bool Deleted=false;bool bDeleteMe(){return Deleted;}};
struct BitfieldBool{bool&Value;BitfieldBool&operator=(bool b){Value=b;return*this;}};
struct Exception{[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct ExpressionValue{int Number=0;UObject*Object=nullptr;static ExpressionValue IntValue(int n){return {n,nullptr};}static ExpressionValue ObjectValue(UObject*p){return {0,p};}};
enum class EventName{AnimEnd};struct Event{UObject*Context;std::string Name;std::vector<ExpressionValue>Args;};std::vector<Event>Events;std::function<void()>EndCallback;
void CallEvent(UObject*p,EventName,std::vector<ExpressionValue>args={}){Events.push_back({p,"AnimEnd",args});if(EndCallback)EndCallback();}
void CallEvent(UObject*p,const NameString&name,std::vector<ExpressionValue>args={}){Events.push_back({p,name.ToString(),args});}
bool FindEventFunction(UObject*,const char*){return true;}
namespace PoaNotifies{bool Dispatch(UObject*notify,UActor*owner){CallEvent(notify,NameString("Notify"),{ExpressionValue::ObjectValue(owner)});return true;}}
template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){}};
template<class F>void RegisterVMNativeFunc_1(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_2(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_3(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_4(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_6(const char*,const char*,F,int){}
'''+body+r'''
int main(){using namespace PoaActorAnimation;UPoaMeshAnimation resource,other;Package package;resource.package=&package;UObject notify;package.Objects[7]=&notify;
 auto&d=resource.Animation;d.Bones={{"root",0,0}};d.Sequences.push_back({0,"Walk",{},0,4,{{.25f,"Step",0},{.5f,"None",7}},4});AnimationMove_unused:;PoaAssets::AnimationMove move;move.Duration=4;PoaAssets::AnimationTrack track;track.Position={{0,0,0},{4,0,0}};track.Time={0,4};move.Tracks={track};d.Moves={move};other.Animation=d;
 UPoaSkeletalMesh mesh;mesh.AnimationResource=&resource;mesh.Geometry.Bones={{"root",0,{0,0,0,1},{0,0,0},0,{0,0,0},0,0}};UActor actor,second;actor.MeshObject=second.MeshObject=&mesh;StateFrameData latent;actor.StateFrame=&latent;
 FinishAnim(&actor,{});assert(latent.State==LatentRunState::Continue);PlayAnim(&actor,"Walk",{}, {},{});Tick(&actor,.5);assert(Events.size()==2&&Events[0].Name=="Step"&&Events[1].Context==&notify&&Events[1].Args[0].Object==&actor);NameString name("None");float frame=0,rate=0;GetAnimParams(&actor,0,name,frame,rate);assert(name.ToString()=="Walk"&&frame==.5&&rate==1);assert(!second.PoaAnimation);
 auto pose=Pose(&actor,&mesh);assert(std::abs(pose[0].Position[0]-2)<1e-5);
 GetAnimSequence(&actor,{},name);assert(name.ToString()=="Walk");float frames=0;GetAnimNumFrames(&actor,{},frames);assert(frames==4);bool group=false;BitfieldBool groupResult{group};AnimIsInGroup(&actor,0,"Missing",groupResult);assert(!group);
 FinishAnim(&actor,{});Tick(&actor,.5);assert(latent.State==LatentRunState::Continue&&Events.back().Name=="AnimEnd"&&Events.back().Args[0].Number==0);bool active=true;IsAnimating(&actor,{},BitfieldBool{active});assert(!active);
 PlayAnim(&actor,"Walk",{}, {},{});FinishAnim(&actor,{});EndCallback=[&](){assert(latent.State==LatentRunState::Continue);PlayAnim(&actor,"Walk",{}, {},{});};Tick(&actor,.75);EndCallback={};assert(actor.PoaAnimation->Waiting==-1&&actor.PoaAnimation->At(0).Playing);
 LoopAnim(&actor,"Walk",2,0,1);AnimBlendParams(&actor,1,.5,0,0,NameString("root"),false);Tick(&actor,.25);assert(actor.PoaAnimation->At(1).Phase==.5);HasAnim(&actor,"Missing",BitfieldBool{active});assert(!active);HasAnim(&actor,"walk",BitfieldBool{active});assert(active);
 LoopAnim(&actor,"Walk",{}, {},{});FinishAnim(&actor,{});assert(latent.State==LatentRunState::FinishAnim);LinkSkelAnim(&actor,&other,{});assert(latent.State==LatentRunState::Continue);assert(actor.PoaAnimationObject==&other&&actor.MeshObject==&mesh&&mesh.AnimationResource==&resource&&second.PoaAnimationObject==nullptr);assert(actor.PoaAnimation->At(1).Name.empty());
 TweenAnim(&actor,"Walk",.5,{});IsAnimating(&actor,{},BitfieldBool{active});assert(active);Tick(&actor,.5);IsAnimating(&actor,{},BitfieldBool{active});assert(!active);
 PlayAnim(&actor,"Walk",{}, {},{});StopAnimating(&actor,true);assert(!actor.PoaAnimation->At(0).Playing);
 AnimBlendParams(&actor,1,1,0,0,NameString("root"),true);assert(actor.PoaAnimation->At(1).GlobalPose);AnimBlendParams(&actor,1,1,.25,3,NameString("root"),false);assert(actor.PoaAnimation->At(1).BlendIn==1&&actor.PoaAnimation->At(1).BlendOut==3&&actor.PoaAnimation->At(1).AlphaTime==0);bool rejected=false;try{SetAnimRate(&actor,NAN,{});}catch(...){rejected=true;}assert(rejected);
 SetAnimTime(&actor,.5,{});GetAnimTime(&actor,{},frame);assert(frame==.5);GetAnimFrame(&actor,{},frame);assert(frame==2);
 SetAnimFrame(&actor,1,{},0);GetAnimTime(&actor,{},frame);assert(std::abs(frame-1.0f/3)<1e-6);GetAnimFrame(&actor,{},frame);assert(std::abs(frame-4.0f/3)<1e-6);
 SetAnimFrame(&actor,.25,{},1);GetAnimTime(&actor,{},frame);assert(frame==.25);SetAnimRate(&actor,2,{});GetAnimRate(&actor,{},rate);assert(rate==2);
 FreezeAnimAt(&actor,.6,{});GetAnimTime(&actor,{},frame);GetAnimRate(&actor,{},rate);assert(std::abs(frame-.6)<1e-6&&rate==0&&!actor.PoaAnimation->At(0).Playing);
 second.Default=2;PlayAnim(&second,"Walk",{}, {},{});GetAnimSequence(&second,{},name);assert(name.ToString()=="Walk"&&second.PoaAnimation->At(0).Name.empty()&&second.PoaAnimation->At(2).Name=="Walk");
 Rotator delta{1,2,3};GetRootRotationDelta(delta);assert(delta.Pitch==0&&delta.Yaw==0&&delta.Roll==0);RegisterNatives();
}
'''
  golden=json.loads((ROOT/'tools/poa/reference-profile/eu-animation-helpers.json').read_text())['cases']
  rows=[]
  for case in golden:
   rows.append('{'+','.join([str(case['frames']),str(case['unit'])]+[str(case[k] if case[k] is not None else 0) for k in ('input','phase','frame','setTime','rate','freeze','afterRate','afterTween')]+['true' if case['phase'] is not None else 'false'])+'}')
  table='struct Original{int Frames,Unit;float Input,Phase,Frame,Time,Rate,Freeze,After,Tween;bool Finite;};Original originals[]={'+','.join(rows)+'};'
  checks=r''' LinkSkelAnim(&actor,&resource,{});size_t corrections=0;
 for(const auto& ref:originals){resource.Animation.Sequences[0].Frames=ref.Frames;resource.Animation.Sequences[0].Rate=30;PlayAnim(&actor,"Walk",1,0,0);SetAnimFrame(&actor,ref.Input,0,ref.Unit);float phase=0,outFrame=0;GetAnimTime(&actor,0,phase);GetAnimFrame(&actor,0,outFrame);if(ref.Finite){assert(std::abs(phase-ref.Phase)<1e-5&&std::abs(outFrame-ref.Frame)<1e-5);}else{assert(std::isfinite(phase)&&std::isfinite(outFrame));++corrections;}SetAnimTime(&actor,ref.Input,0);GetAnimTime(&actor,0,phase);assert(phase==ref.Time);SetAnimRate(&actor,1.25,0);GetAnimRate(&actor,0,rate);assert(std::abs(rate-ref.Rate)<1e-5);FreezeAnimAt(&actor,ref.Freeze,0);GetAnimTime(&actor,0,phase);GetAnimRate(&actor,0,rate);assert(phase==ref.Freeze&&rate==ref.After&&actor.PoaAnimation->At(0).Tween==ref.Tween);}assert(corrections==7);
'''
  code=code.replace(' Rotator delta{1,2,3};GetRootRotationDelta(delta);assert(delta.Pitch==0&&delta.Yaw==0&&delta.Roll==0);RegisterNatives();',table+checks+' Rotator delta{1,2,3};GetRootRotationDelta(delta);assert(delta.Pitch==0&&delta.Yaw==0&&delta.Roll==0);RegisterNatives();')
  code=code.replace('AnimationMove_unused:;','')
  # Native wrappers receive lvalue BitfieldBool outputs.
  code=code.replace('bool active=true;', 'bool active=true;BitfieldBool output{active};').replace('BitfieldBool{active}', 'output')
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
