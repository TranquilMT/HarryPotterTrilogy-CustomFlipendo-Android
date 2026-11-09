"""Production SetPhysics and SetBase callback/list integration with EU reference cases."""
import json,os,pathlib,tempfile,subprocess,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class PhysicsBaseTransitions(unittest.TestCase):
 def run_body(self,checks):
  root=ENGINE/'Packages/Engine/Actors'
  source=(root/'UActor_Base.cpp').read_text()
  base='\n'.join(definition(source,'void UActor::'+n+'(')for n in ['AddBasedActor','RemoveBasedActor','SetBase'])+definition(source,'bool UActor::IsBasedOn(')
  setter=definition((root/'UActor_Phys.cpp').read_text(),'void UActor::SetPhysics(')
  code=r'''#include "Package/PoaPhysicsModes.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdint>
struct vec3{float x,y,z;explicit vec3(float v=0):x(v),y(v),z(v){}};
constexpr int PHYS_None=0,PHYS_Rotating=5;
struct UObject{virtual~UObject()=default;};int Roots=0;
template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}};
struct Launch{bool Poa=true;bool IsHarryPotter3(){return Poa;}};struct Engine{Launch LaunchInfo;}instance;Engine*engine=&instance;
struct UActor:UObject{
 uint8_t Mode=0,Count=0;bool LadderMode=true,Deleted=false;UActor*Base=nullptr;std::vector<UActor*>BasedActors,Attached;vec3 Speed{7},Accel{9};
 uint8_t&Physics(){return Mode;}uint8_t&StandingCount(){return Count;}bool HasProperty(const char*){return !instance.LaunchInfo.Poa;}bool bDeleteMe(){return Deleted;}vec3&Velocity(){return Speed;}vec3&Acceleration(){return Accel;}
 UActor*&ActorBase(){return Base;}UActor*Level(){return nullptr;}void AddBasedActor(UActor*);void RemoveBasedActor(UActor*);void SetBase(UActor*,bool);bool IsBasedOn(UActor*);void SetPhysics(uint8_t);
};
namespace PoaBones{void SyncAttached(UActor*p,UActor*c,bool add){auto&i=p->Attached;auto at=std::find(i.begin(),i.end(),c);if(add&&at==i.end())i.push_back(c);else if(!add&&at!=i.end())i.erase(at);}}
enum class EventName{Attach,Detach,BaseChange};
struct ExpressionValue{UObject*Object;static ExpressionValue ObjectValue(UObject*p){return{p};}};
int Detaches=0,Changes=0;std::function<void(UObject*,EventName)>Hook;
void CallEvent(UObject*p,EventName e,std::vector<ExpressionValue> = {}){if(e==EventName::Detach)++Detaches;if(e==EventName::BaseChange)++Changes;auto callback=Hook;if(callback)callback(p,e);}
namespace PoaFloor{bool FindBase(UActor*){return false;}}
'''+base+setter+'\nint main(){\n'+checks+'\n}\n'
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_original_reference_and_attachment_lists(self):
  profile=json.loads((ROOT/'tools/poa/reference-profile/eu-physics-base.json').read_text());self.assertEqual(len(profile['cases']),338)
  rows=','.join('{'+','.join(map(str,[r['old'],r['new'],int(r['hadBase']),int(r['hasBase']),r['setBaseCalls'],r['notify']]))+'}'for r in profile['cases'])
  self.run_body('struct Case{int Old,New,Had,Has,Calls,Notify;};Case cases[]={'+rows+r'''};
for(const auto&r:cases){UActor parent,child;if(r.Had)child.SetBase(&parent,false);child.Mode=uint8_t(r.Old);Detaches=Changes=0;child.SetPhysics(uint8_t(r.New));assert(child.Mode==r.New);assert((child.Base!=nullptr)==bool(r.Has));assert(Detaches==r.Calls&&Changes==r.Notify);assert(parent.BasedActors.size()==size_t(r.Has)&&parent.Attached.size()==size_t(r.Has));assert(Roots==0);}
''')
 def test_callbacks_reentrancy_deletion_and_legacy(self):
  self.run_body(r'''
UActor parent,other,child;child.SetBase(&parent,false);child.Mode=1;
Hook=[&](UObject*p,EventName e){assert(Roots>0);if(p==&child&&e==EventName::BaseChange){Hook={};child.SetPhysics(5);}};
child.SetPhysics(2);assert(!child.Base&&child.Mode==5&&child.Speed.x==0&&child.Accel.z==0&&parent.BasedActors.empty());
child.SetBase(&parent,false);child.Mode=1;child.Speed=vec3(7);
Hook=[&](UObject*p,EventName e){if(p==&child&&e==EventName::BaseChange){Hook={};child.Mode=5;child.Deleted=true;}};
child.SetPhysics(2);assert(child.Deleted&&child.Speed.x==7&&!child.Base);
child.Deleted=false;child.SetBase(&parent,false);child.Mode=1;
Hook=[&](UObject*p,EventName e){if(p==&child&&e==EventName::BaseChange){Hook={};child.SetBase(&other,false);}};
child.SetPhysics(4);assert(child.Base==&other&&parent.BasedActors.empty()&&other.BasedActors.size()==1&&other.Attached.size()==1);
Hook={};instance.LaunchInfo.Poa=false;child.Mode=1;child.Speed=vec3(7);child.SetPhysics(2);assert(child.Base==&other&&child.Speed.x==7);assert(Roots==0);
''')
if __name__=='__main__':unittest.main()
