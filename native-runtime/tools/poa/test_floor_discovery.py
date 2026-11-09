"""Original EU floor-query controller and production cylindrical path."""
import os,pathlib,tempfile,subprocess,unittest,json
from inspect_packages import inspect,Reader
from object_payloads import tags,name
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class FloorDiscovery(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for EU shape enumeration')
 def test_original_shape_enum(self):
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u';m=inspect(p);raw=p.read_bytes();e=next(e for e in m['exports']if e['name']=='ECollideType'and e['class']=='Enum');r=Reader(raw[e['offset']:e['offset']+e['size']]);tags(r,m['names']);r.compact();r.compact();self.assertEqual([name(r,m['names'])for _ in range(r.compact())],['CT_Cylinder','CT_Box'])
 def run_body(self,checks):
  floor='\n'.join(l for l in (ROOT/'src/hp3/PoaFloor.cpp').read_text().splitlines()if not l.startswith('#include'))
  setter=definition((ENGINE/'Packages/Engine/Actors/UActor_Phys.cpp').read_text(),'void UActor::SetPhysics(')
  code=r'''#include "Package/PoaWorldCollision.h"
#include "Package/PoaPhysicsModes.h"
#include <vector>
#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <cstdint>
#include "Math/vec.h"
struct UActor;struct Hit{UActor*Actor;vec3 Normal{0,0,1};};struct Level{std::vector<Hit>Hits;int Calls=0;vec3 Start,End;float Height=0,Radius=0,Width=0;std::vector<Hit>TraceExtent(const vec3&s,const vec3&e,vec3 box,bool actors,bool world,bool visibility){auto hits=Trace(s,e,box.z,box.x,actors,world,visibility);Width=box.y;return hits;}std::vector<Hit>Trace(const vec3&s,const vec3&e,float h,float r,bool actors,bool world,bool visibility){assert(actors&&world&&!visibility);++Calls;Start=s;End=e;Height=h;Radius=r;return Hits;}};struct Scene{struct Level Collision;};
struct UObject{virtual~UObject()=default;template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}};
int Roots=0;template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}};
struct Launch{bool IsHarryPotter3(){return true;}};struct Engine{Launch LaunchInfo;}instance;Engine*engine=&instance;
std::function<void(UActor*)>Hook;
struct UActor:UObject{Scene*ScenePtr=nullptr;UActor*WorldLevel=nullptr,*Base=nullptr;uint8_t Mode=0,Shape=0;bool Deleted=false,HaveShape=true,World=false,CollideWorld=true,Actors=true,Players=true,LadderMode=true;float Radius=2,Height=5,Width=0;float GetFloat(const char*){return Width;}int Sets=0;vec3 Position{10,20,30},Speed{7},Accel{9},FloorNormal{9};
 void SetVector(const char*,vec3 v){FloorNormal=v;}bool bDeleteMe(){return Deleted;}bool HasProperty(const char*s){return std::string(s)=="CollideType"?HaveShape:true;}bool GetBool(const char*){return World;}uint8_t GetByte(const char*){return Shape;}float CollisionRadius(){return Radius;}float CollisionHeight(){return Height;}Scene*XLevel(){return ScenePtr;}UActor*Level(){return WorldLevel;}vec3 Location(){return Position;}bool bCollideWorld(){return CollideWorld;}bool bBlockActors(){return Actors;}bool bBlockPlayers(){return Players;}UActor*&ActorBase(){return Base;}void SetBase(UActor*p,bool notify){assert(notify);assert(Roots>0);Base=p;++Sets;auto cb=Hook;if(cb)cb(this);}uint8_t&Physics(){return Mode;}vec3&Velocity(){return Speed;}vec3&Acceleration(){return Accel;}void SetPhysics(uint8_t);
};
struct UPlayerPawn:UActor{};struct UProjectile:UActor{};constexpr int PHYS_None=0,PHYS_Rotating=5;
'''+ '\n#include "Collision/BottomLevel/PoaBoxCollision.h"\n'+floor+setter+'\nint main(){\n'+checks+'\n}\n'
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_original_controller_and_deferred_box_shapes(self):
  profile=json.loads((ROOT/'tools/poa/reference-profile/eu-floor.json').read_text());self.assertEqual(len(profile['cases']),36)
  rows=','.join('{'+','.join(map(str,[r['scenario'],r['oldBase'],r['hit'],r['newBase'],r['setBaseCalls']]))+'}'for r in profile['cases'])
  self.run_body('struct Case{int Shape,Old,Hit,New,Calls;};Case cases[]={'+rows+r'''};
for(auto r:cases){Scene scene;UActor a,b,c;a.ScenePtr=&scene;a.Shape=r.Shape>=2;a.Width=r.Shape==3?3:0;b.World=c.World=true;UActor*actors[]={nullptr,&b,&c};a.Base=actors[r.Old];if(r.Hit)scene.Collision.Hits={{actors[r.Hit]}};bool queried=PoaFloor::FindBase(&a);
 if(r.Shape>=2){assert(scene.Collision.Width==(r.Shape==3?3:2));}
 assert(queried&&a.Base==actors[r.New]&&a.Sets==r.Calls&&scene.Collision.Calls==1);auto&s=scene.Collision;assert(s.Start.x==10&&s.Start.y==20&&s.Start.z==30&&s.End.x==10&&s.End.y==20&&s.End.z==22&&s.Radius==2&&s.Height==5);assert(a.Position.z==30&&Roots==0);}
''')
 def test_filters_context_and_reentrant_setphysics(self):
  self.run_body(r'''
Scene scene;UActor a,old,floor,ignored,deleted,level;a.ScenePtr=&scene;a.WorldLevel=&level;a.Base=&old;floor.World=true;ignored.Actors=ignored.Players=false;deleted.Deleted=true;
scene.Collision.Hits={{&a},{&deleted},{&ignored},{&floor}};assert(PoaFloor::FindBase(&a)&&a.Base==&floor&&a.Sets==1);
a.CollideWorld=false;assert(PoaFloor::FindBase(&a)&&!a.Base);a.CollideWorld=true;scene.Collision.Hits={{&level}};assert(PoaFloor::FindBase(&a)&&a.Base==&level);
a.Shape=2;auto count=scene.Collision.Calls;assert(!PoaFloor::FindBase(&a)&&scene.Collision.Calls==count&&a.Base==&level);a.Shape=0;a.Radius=NAN;assert(!PoaFloor::FindBase(&a));a.Radius=2;a.ScenePtr=nullptr;assert(!PoaFloor::FindBase(&a));a.ScenePtr=&scene;
a.Base=&old;a.Mode=2;a.Speed=vec3(7);scene.Collision.Hits={{&floor}};Hook=[](UActor*p){Hook={};p->SetPhysics(5);};a.SetPhysics(1);assert(a.Base==&floor&&a.Mode==5&&a.Speed.x==0);
a.Base=&old;a.Mode=2;a.Speed=vec3(7);Hook=[](UActor*p){Hook={};p->Deleted=true;p->Mode=5;};a.SetPhysics(1);assert(a.Deleted&&a.Base==&floor&&a.Speed.x==7);Hook={};assert(Roots==0);
''')
 def test_normal_written_before_callbacks(self):
  self.run_body(r'''
Scene scene;UActor a,floor;a.ScenePtr=&scene;floor.World=true;
scene.Collision.Hits={{&floor,vec3(0,.6f,.8f)}};
Hook=[](UActor*p){assert(p->FloorNormal.y==.6f&&p->FloorNormal.z==.8f);};
assert(PoaFloor::FindBase(&a));Hook={};
a.FloorNormal=vec3(7);assert(PoaFloor::FindBase(&a)&&a.FloorNormal.z==7);
scene.Collision.Hits.clear();Hook=[](UActor*p){assert(p->FloorNormal.x==0&&p->FloorNormal.y==0&&p->FloorNormal.z==0);};
assert(PoaFloor::FindBase(&a)&&!a.Base);Hook={};assert(Roots==0);
''')
if __name__=='__main__':unittest.main()
