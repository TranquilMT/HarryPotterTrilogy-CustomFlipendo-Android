"""EU enum identity and production actor physics dispatch regressions."""
import os,pathlib,tempfile,subprocess,unittest
from inspect_packages import inspect,Reader
from object_payloads import tags,name
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
EXPECTED=['None','Walking','Falling','Swimming','Flying','Rotating','Projectile','Interpolating','MovingBrush','Spider','Trailer','Ladder','RootMotion','Karma','KarmaRagDoll','PushPulled']
class PhysicsModes(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original EU enumeration')
 def test_original_enum_order(self):
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u';m=inspect(p);data=p.read_bytes();e=next(e for e in m['exports']if e['class']=='Enum'and e['name']=='EPhysics');r=Reader(data[e['offset']:e['offset']+e['size']]);self.assertEqual(tags(r,m['names']),[]);r.compact();r.compact();count=r.compact();self.assertEqual([name(r,m['names'])for _ in range(count)],['PHYS_'+n for n in EXPECTED]);self.assertEqual(r.p,len(r.data))
 def test_production_dispatch_and_serialized_identity(self):
  actor=(ENGINE/'Packages/Engine/Actors/UActor.h').read_text();enum=actor[actor.index('enum EPhysics'):actor.index('enum ENetRole')]
  source=(ENGINE/'Packages/Engine/Actors/UActor_Phys.cpp').read_text();body='\n'.join(definition(source,signature)for signature in ('void UActor::TickPhysics(','int UActor::PhysicsMode(','void UActor::SetPhysics('))
  code=r'''#include "Package/PoaPhysicsModes.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>
'''+enum+r'''
struct Exception{[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct Launch{bool Poa=false,KW=false;bool IsHarryPotter3(){return Poa;}bool IsKnowWonder(){return KW;}};
struct Engine{Launch LaunchInfo;} instance;Engine*engine=&instance;
struct vec3{float x,y,z;explicit vec3(float v=0):x(v),y(v),z(v){}};
template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){}};
struct UActor{UActor*Base=nullptr;UActor*&ActorBase(){return Base;}void SetBase(UActor*p,bool){Base=p;}vec3 Speed,Accel;vec3&Velocity(){return Speed;}vec3&Acceleration(){return Accel;}uint8_t Raw=0;int Called=0,Steps=0,Rotations=0,Touches=0;float Time=0;bool Attached=false,Deleted=false,Change=false,LadderMode=true;
 uint8_t&Physics(){return Raw;}bool bDeleteMe(){return Deleted;}int PhysicsMode();void SetPhysics(uint8_t);void TickPhysics(float);
 void Step(int mode,float dt){Called=mode;++Steps;Time+=dt;if(Change)Raw=6;}
 void TickWalking(float dt){Step(1,dt);}void TickFalling(float dt){Step(2,dt);}void TickSwimming(float dt){Step(3,dt);}void TickFlying(float dt){Step(4,dt);}void TickProjectile(float dt){Step(6,dt);}void TickRolling(float dt){Step(7,dt);}void TickInterpolating(float dt){Step(8,dt);}void TickMovingBrush(float dt){Step(9,dt);}void TickSpider(float dt){Step(10,dt);}void TickTrailer(float dt){Step(11,dt);}void TickRotating(float){++Rotations;}void CheckPendingTouch(){++Touches;}
};
namespace PoaBones{bool IsAttached(UActor*a){return a->Attached;}void UpdateAttachment(UActor*a){a->Called=100;}}
namespace KW{bool UseKWPhysics(){return true;}void PhysicsTick(UActor*a,float dt){a->Step(101,dt);}bool IsInterpolationManager(UActor*){return false;}void InterpolationManagerPhysics(UActor*a,float dt){a->Step(102,dt);}}
namespace PoaFloor{bool FindBase(UActor*){return false;}}
'''+body+r'''
int main(){
 int expected[]={PHYS_None,PHYS_Walking,PHYS_Falling,PHYS_Swimming,PHYS_Flying,PHYS_Rotating,PHYS_Projectile,PHYS_Interpolating,PHYS_MovingBrush,PHYS_Spider,PHYS_Trailer};
 instance.LaunchInfo.Poa=true;
 for(int raw=0;raw<=10;++raw){UActor a;a.SetPhysics(uint8_t(raw));assert(a.Raw==raw&&a.LadderMode==(raw==0));assert(a.PhysicsMode()==expected[raw]);a.TickPhysics(.01f);assert(a.Raw==raw&&a.Touches==1);if(raw==0||raw==5)assert(a.Steps==0);else assert(a.Called==expected[raw]&&a.Steps==1);}
 for(int raw=11;raw<256;++raw){UActor a;a.Raw=uint8_t(raw);assert(a.PhysicsMode()>=256);bool threw=false;try{a.TickPhysics(.01f);}catch(const std::runtime_error&){threw=true;}assert(threw&&a.Raw==raw&&a.Steps==0&&a.Rotations==0);}
 UActor changing;changing.Raw=7;changing.Change=true;changing.TickPhysics(.04f);assert(changing.Steps==2&&changing.Called==PHYS_Projectile&&changing.Raw==6);
 UActor attached;attached.Raw=12;attached.Attached=true;attached.TickPhysics(.01f);assert(attached.Called==100&&attached.Raw==12);
 instance.LaunchInfo.Poa=false;
 for(int raw=0;raw<256;++raw){UActor a;a.Raw=uint8_t(raw);assert(a.PhysicsMode()==raw);}UActor legacy;legacy.Raw=7;legacy.TickPhysics(.01f);assert(legacy.Called==PHYS_Rolling&&legacy.Raw==7);
 instance.LaunchInfo.KW=true;UActor hp1;hp1.Raw=7;hp1.TickPhysics(.01f);assert(hp1.Called==101&&hp1.Raw==7);
}
'''
  with tempfile.TemporaryDirectory()as temp:
   p=pathlib.Path(temp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_spider_and_reach_checks_use_semantic_mode(self):
  for name in ['Pawn/UPlayerPawn.cpp','Pawn/UPawn_Tick.cpp','Pawn/UPawn_Reach.cpp']:
   source=(ENGINE/'Packages/Engine/Actors'/name).read_text();self.assertNotIn('Physics() == PHYS_Spider',source);self.assertNotIn('switch (Physics())',source)
if __name__=='__main__':unittest.main()
