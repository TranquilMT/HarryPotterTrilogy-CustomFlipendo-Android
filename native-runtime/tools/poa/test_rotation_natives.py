"""Production conversions against isolated original EU math reference outputs."""
import pathlib,json,struct,tempfile,subprocess,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class RotationNatives(unittest.TestCase):
 def test_original_quaternion_corpus_and_stable_half_turns(self):
  source=(ROOT/'src/hp3/PoaCompat.cpp').read_text();bodies='\n'.join(definition(source,s)for s in ['static void QuatFromRotator(','static void QuatToRotator('])
  code=r'''#include "hp3/PoaRotation.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
struct Record{int32_t input[3];float q[4];int32_t output[3];float forward[3];};
'''+bodies+r'''
int Difference(int a,int b){unsigned d=(unsigned(a)-unsigned(b))&65535;return int(d>32768?65536-d:d);}
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);Record r;int cases=0,valid=0,corrected=0,regular=0,gimbal=0;
 while(f.read(reinterpret_cast<char*>(&r),sizeof(r))){
  quaternion q;QuatFromRotator({r.input[0],r.input[1],r.input[2]},q);double norm=double(q.x)*q.x+double(q.y)*q.y+double(q.z)*q.z+double(q.w)*q.w;assert(std::abs(norm-1)<1e-5);
  double oldNorm=0,dotProduct=0;for(int i=0;i<4;++i){oldNorm+=double(r.q[i])*r.q[i];dotProduct+=double((&q.x)[i])*r.q[i];}
  const bool usable=std::abs(oldNorm-1)<.0001;
  if(usable){++valid;assert(std::abs(dotProduct)>1-2e-5);}else ++corrected;
  Rotator rotation;QuatToRotator({r.q[0],r.q[1],r.q[2],r.q[3]},rotation);
  if(std::hypot(r.forward[0],r.forward[1])>=1e-5){++regular;assert(Difference(rotation.Pitch,r.output[0])<=8);assert(Difference(rotation.Yaw,r.output[1])<=8);assert(Difference(rotation.Roll,r.output[2])<=8);}
  else{++gimbal;if(usable){auto roundtrip=PoaRotation::FromRotator(rotation);double aligned=double(roundtrip.x)*r.q[0]+double(roundtrip.y)*r.q[1]+double(roundtrip.z)*r.q[2]+double(roundtrip.w)*r.q[3];assert(std::abs(aligned)>1-2e-5);}}
  ++cases;
 }
 assert(cases==1331&&valid==1081&&corrected==250);quaternion a=PoaRotation::FromRotator({0,0,32768});assert(std::abs(a.x)>.9999&&std::abs(a.w)<1e-6);
 bool rejected=false;try{PoaRotation::ToRotator({NAN,0,0,1});}catch(...){rejected=true;}assert(rejected);rejected=false;try{PoaRotation::ToRotator({std::numeric_limits<float>::max(),0,0,1});}catch(...){rejected=true;}assert(rejected);
 std::cout<<"PASS "<<cases<<" EU rotation cases; "<<corrected<<" original non-unit outputs corrected; "<<regular<<" regular and "<<gimbal<<" singular extraction cases\n";
}
'''
  rows=json.loads((ROOT/'tools/poa/reference-profile/eu-quaternion-math.json').read_text())['cases']
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);(t/'cases.bin').write_bytes(b''.join(struct.pack('<3i4f3i3f',*r)for r in rows));subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src'),'-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test'),str(t/'cases.bin')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
