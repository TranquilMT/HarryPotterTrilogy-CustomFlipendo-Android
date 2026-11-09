"""Retail reference and exhaustive production SetPhysics side effects."""
import os,pathlib,tempfile,subprocess,unittest,json
from inspect_packages import inspect
from object_payloads import function_header,native_stub
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class PhysicsTransitions(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for EU SetPhysics ABI')
 def test_original_setphysics_abi(self):
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u';m=inspect(p);raw=p.read_bytes();e=next(e for e in m['exports']if e['name']=='SetPhysics'and e['class']=='Function');r,h=function_header(raw[e['offset']:e['offset']+e['size']],m['names']);stub=native_stub(r,h);self.assertEqual((len(stub['parameters']),stub['nativeIndex']),(1,3970))
 def test_repeated_mode_stopping_and_original_reference(self):
  body=definition((ENGINE/'Packages/Engine/Actors/UActor_Phys.cpp').read_text(),'void UActor::SetPhysics(')
  profile=json.loads((ROOT/'tools/poa/reference-profile/eu-physics-setter.json').read_text());self.assertEqual(len(profile['cases']),144)
  rows=[]
  for r in profile['cases']:rows.append('{'+','.join(map(str,[r['old'],r['new']]+r['velocity']+r['acceleration']))+'}')
  code=r'''#include "Package/PoaPhysicsModes.h"
#include <cstdint>
#include <cassert>
struct vec3{float x,y,z;explicit vec3(float v=0):x(v),y(v),z(v){}};
constexpr int PHYS_None=0,PHYS_Rotating=5;
struct Launch{bool Poa=true;bool IsHarryPotter3(){return Poa;}};struct Engine{Launch LaunchInfo;}instance;Engine*engine=&instance;
template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){}};
struct UActor{UActor*Base=nullptr;UActor*&ActorBase(){return Base;}void SetBase(UActor*p,bool){Base=p;}bool bDeleteMe(){return false;}uint8_t Mode=0;bool LadderMode=true;vec3 Speed{7},Accel{9};uint8_t&Physics(){return Mode;}vec3&Velocity(){return Speed;}vec3&Acceleration(){return Accel;}void SetPhysics(uint8_t);};
namespace PoaFloor{bool FindBase(UActor*){return false;}}
'''+body+r'''
struct Original{int Old,New;float X,Y,Z,AX,AY,AZ;};
Original reference[]={'''+','.join(rows)+r'''};
int main(){for(const auto&r:reference){UActor a;a.Mode=uint8_t(r.Old);a.Speed.x=7;a.Speed.y=8;a.Speed.z=9;a.Accel.x=9;a.Accel.y=10;a.Accel.z=11;a.SetPhysics(uint8_t(r.New));assert(a.Mode==r.New&&a.Speed.x==r.X&&a.Speed.y==r.Y&&a.Speed.z==r.Z&&a.Accel.x==r.AX&&a.Accel.y==r.AY&&a.Accel.z==r.AZ);}
 for(int old=0;old<256;++old)for(int target=0;target<256;++target){UActor a;a.Mode=uint8_t(old);a.SetPhysics(uint8_t(target));assert(a.Mode==target&&a.LadderMode==(old==target));bool stop=old!=target&&(target==0||target==5);assert(a.Speed.x==(stop?0:7)&&a.Speed.y==(stop?0:7)&&a.Speed.z==(stop?0:7));assert(a.Accel.x==(stop?0:9)&&a.Accel.y==(stop?0:9)&&a.Accel.z==(stop?0:9));}
 instance.LaunchInfo.Poa=false;for(int target=0;target<256;++target){UActor a;a.Mode=uint8_t(target);a.SetPhysics(uint8_t(target));assert(!a.LadderMode&&a.Speed.x==7&&a.Accel.z==9&&a.Mode==target);}
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-I'+str(ENGINE),'-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
