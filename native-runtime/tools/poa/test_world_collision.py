"""Original world-blocking reference plus production trace-filter and flag routing."""
import pathlib,tempfile,subprocess,unittest,os,json
from inspect_packages import inspect
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class WorldCollision(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for EU flag declarations')
 def test_original_bool_declarations(self):
  m=inspect(pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u')
  for n in ['bWorldGeometry','bCollideWorld','bCollideActors']:
   e=next(e for e in m['exports']if e['name']==n and e['outer']>0 and m['exports'][e['outer']-1]['name']=='Actor');self.assertEqual(e['class'],'BoolProperty')
 def test_original_reference_and_production_trace_filter(self):
  profile=json.loads((ROOT/'tools/poa/reference-profile/eu-world-collision.json').read_text());self.assertEqual(len(profile['cases']),512)
  rows=','.join('{'+','.join([str(r['selfMask']),str(r['otherMask']),str(int(r['blocked']))])+'}'for r in profile['cases'])
  body=definition((ENGINE/'Collision/TopLevel/CollisionSystem.cpp').read_text(),'CollisionHit CollisionSystem::TraceFirstHit(')
  code=r'''#include "Package/PoaWorldCollision.h"
#include <vector>
#include <string>
#include <cassert>
struct vec3{};
struct UActor{bool World=false,Collide=true,Actors=true,Players=true,ProjectileTarget=false,HasWorld=true;int Reads=0;std::string Type="Actor";UActor*Owner=nullptr;bool HasProperty(const char*){return HasWorld;}bool GetBool(const char*){++Reads;return World;}bool bCollideWorld(){return Collide;}bool bCollideActors(){return Actors;}bool bBlockActors(){return Actors;}bool bBlockPlayers(){return Players;}bool bProjTarget(){return ProjectileTarget;}bool IsA(const char*s){return Type==s;}bool IsOwnedBy(UActor*a){return this==a||Owner==a;}};
struct Launch{bool Poa=true;bool IsHarryPotter3(){return Poa;}};struct Engine{Launch LaunchInfo;}instance;Engine*engine=&instance;
struct TraceFlags{bool pawns=false,movers=false,others=false,world=false,zoneChanges=false,onlyProjectiles=false;bool traceActors()const{return pawns||movers||others||zoneChanges||onlyProjectiles;}bool traceWorld()const{return world;}};
struct CollisionHit{UActor*Actor=nullptr;};
struct CollisionSystem{UActor*LevelInfo=nullptr;std::vector<CollisionHit>Hits;std::vector<CollisionHit>Trace(const vec3&,const vec3&,float,float,bool,bool,bool){return Hits;}CollisionHit TraceFirstHit(const vec3&,const vec3&,UActor*,const vec3&,const TraceFlags&);};
'''
  code=code.replace('struct vec3{};','struct vec3{float x=0,y=0,z=0;};')+body+r'''
struct Original{int Self,Other,Blocked;};Original cases[]={'''+rows+r'''};
int main(){UActor mover,geometry;geometry.World=true;
for(auto r:cases){mover.Collide=bool(r.Self&8);mover.Actors=bool(r.Self&16);mover.Players=bool(r.Self&32);geometry.Actors=bool(r.Other&16);geometry.Players=bool(r.Other&32);auto result=PoaCollision::WorldBlocking(&mover,&geometry,true);assert(result.has_value()&&*result==bool(r.Blocked));}
geometry.Reads=0;assert(!PoaCollision::WorldBlocking(&mover,&geometry,false).has_value()&&geometry.Reads==0);
mover.Collide=false;auto ignored=PoaCollision::WorldBlocking(&mover,&geometry,true);assert(ignored.has_value()&&!*ignored);geometry.World=false;assert(!PoaCollision::WorldBlocking(&mover,&geometry,true).has_value());
for(int world=0;world<2;++world)for(int actors=0;actors<2;++actors)for(int ta=0;ta<2;++ta)for(int tw=0;tw<2;++tw){geometry.World=world;geometry.Actors=actors;assert(PoaCollision::TraceStaticMesh(&geometry,ta,tw)==bool(world?tw:(ta&&actors)));}
geometry.HasWorld=false;geometry.World=true;assert(!PoaCollision::WorldGeometry(&geometry));geometry.HasWorld=true;
UActor level,ordinary;ordinary.Actors=false;geometry.Actors=false;geometry.World=true;mover.Owner=nullptr;CollisionSystem collision;collision.LevelInfo=&level;collision.Hits={{&geometry}};TraceFlags f;f.world=true;vec3 v;assert(collision.TraceFirstHit(v,v,&mover,v,f).Actor==&geometry);
f.world=false;f.others=true;assert(!collision.TraceFirstHit(v,v,&mover,v,f).Actor);geometry.World=false;assert(collision.TraceFirstHit(v,v,&mover,v,f).Actor==&geometry);
geometry.World=true;f.world=true;mover.Owner=&geometry;assert(!collision.TraceFirstHit(v,v,&mover,v,f).Actor);mover.Owner=nullptr;
instance.LaunchInfo.Poa=false;f.others=false;assert(!collision.TraceFirstHit(v,v,&mover,v,f).Actor);f.others=true;assert(collision.TraceFirstHit(v,v,&mover,v,f).Actor==&geometry);
collision.Hits={{&level}};f.world=false;assert(!collision.TraceFirstHit(v,v,&mover,v,f).Actor);f.world=true;assert(collision.TraceFirstHit(v,v,&mover,v,f).Actor==&level);
}
'''
  with tempfile.TemporaryDirectory()as temp:
   p=pathlib.Path(temp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_movement_and_mesh_paths_use_classification(self):
  actor=(ENGINE/'Packages/Engine/Actors/UActor_Phys.cpp').read_text();move=definition(actor,'CollisionHit UActor::TryMove(');self.assertEqual(move.count('PoaCollision::WorldBlocking('),4)
  trace=(ENGINE/'Collision/TopLevel/TraceTest.cpp').read_text();self.assertIn('PoaCollision::TraceStaticMesh(actor,traceActors,traceWorld)',definition(trace,'void TraceTester::TraceActor('));self.assertIn('PoaCollision::WorldGeometry(actor)',trace)
if __name__=='__main__':unittest.main()
