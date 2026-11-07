"""Exercise actual optional-offset initializers and PoA quaternion operations."""
from pathlib import Path
import re,subprocess,tempfile,unittest
class ProductionStartup(unittest.TestCase):
 def test_optional_network_offsets(self):
  root=Path(__file__).resolve().parents[2];text=(root/'src/engine/SurrealEngine/PropertyOffsets.cpp').read_text()
  source='#include <cstring>\n#include <cassert>\nstruct UClass{int GetPropertyDataOffset(const char*){return 23;}}; struct UObject{template<class T>static T* TryCast(UClass*p){return p;}};\nstruct Package{UClass cls;UClass*GetUObject(const char*,const char*){return &cls;}};\nstruct PackageManager{bool available=false;int loads=0;Package p;bool HasPackage(const char*){return available;}Package*GetPackage(const char*){++loads;return &p;}};\n'
  names=['InternetLink','UdpLink','TcpLink']
  for name in names:
   start=text.index('static void InitPropertyOffsets_'+name+'(');end=text.index('\n}\n',start)+3;body=text[start:end];fields=re.findall('PropOffsets_'+name+r'\.([A-Za-z0-9_]+)',body)
   source+='struct PropertyOffsets_'+name+'{'+''.join('int '+f+'=0;' for f in fields)+'}; PropertyOffsets_'+name+' PropOffsets_'+name+';\n'+body
  source+='int main(){PackageManager p;'
  for name in names:
   source+='InitPropertyOffsets_'+name+'(&p); for(unsigned char c: *reinterpret_cast<unsigned char(*)[sizeof(PropOffsets_'+name+')] >(&PropOffsets_'+name+'))assert(c==255);'
  source+='assert(p.loads==0);p.available=true;'
  for name in names:source+='InitPropertyOffsets_'+name+'(&p);'
  source+='assert(p.loads==3);}\n'
  self.compile_run(source,root)
 def test_quaternion_rotation_and_composition(self):
  root=Path(__file__).resolve().parents[2]
  self.compile_run(r'''#include "hp3/PoaMath.h"
#include <cassert>
#include <cmath>
bool near(float a,float b){return std::abs(a-b)<0.0001f;}
int main(){float h=std::sqrt(0.5f);quaternion q{0,0,h,h};auto v=PoaCore::Rotate(q,{1,0,0});assert(near(v.x,0)&&near(v.y,1)&&near(v.z,0));auto q2=PoaCore::Product(q,q);v=PoaCore::Rotate(q2,{1,0,0});assert(near(v.x,-1)&&near(v.y,0));auto identity=PoaCore::Product(q,{-q.x,-q.y,-q.z,q.w});assert(near(identity.x,0)&&near(identity.y,0)&&near(identity.z,0)&&near(identity.w,1));v=PoaCore::Rotate({}, {2,-3,4});assert(near(v.x,2)&&near(v.y,-3)&&near(v.z,4));}''',root)
 def compile_run(self,source,root):
  with tempfile.TemporaryDirectory() as td:
   p=Path(td);(p/'test.cpp').write_text(source)
   subprocess.run(['g++','-std=c++20','-I'+str(root/'src'),'-I'+str(root/'src/engine/SurrealEngine'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
