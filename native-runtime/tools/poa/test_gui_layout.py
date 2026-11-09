"""Production menu geometry and original EU layout declarations."""
import os,pathlib,subprocess,tempfile,unittest
from inspect_packages import inspect
from object_payloads import function_header,native_stub
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiLayout(unittest.TestCase):
 def test_parent_and_viewport_geometry(self):
  code=r'''#include "hp3/PoaGuiLayout.h"
#include <map>
#include <string>
#include <cassert>
#include <cmath>
struct Object{Object*Controller=this,*Owner=nullptr;std::map<std::string,float>Numbers{{"WinWidth",.5f},{"WinHeight",.25f},{"WinLeft",.1f},{"WinTop",.2f}};bool Bound=false,Scale=false;bool HasProperty(const char*){return true;}Object*GetUObject(const char*s){return std::string(s)=="Controller"?Controller:Owner;}float GetFloat(const char*s){return Numbers.at(s);}bool GetBool(const char*s){return std::string(s)=="bBoundToParent"?Bound:Scale;}};
int main(){Object root,child;child.Owner=&root;
 assert(PoaGuiLayout::Resolve(&root,0,1000,500)==500);assert(PoaGuiLayout::Resolve(&root,1,1000,500)==125);
 child.Scale=true;child.Bound=true;
 assert(PoaGuiLayout::Resolve(&child,0,1000,500)==250);assert(PoaGuiLayout::Resolve(&child,1,1000,500)==31.25f);
 assert(PoaGuiLayout::Resolve(&child,2,1000,500)==150);assert(PoaGuiLayout::Resolve(&child,3,1000,500)==125);
 child.Numbers["WinLeft"]=20;child.Numbers["WinTop"]=30;assert(PoaGuiLayout::Resolve(&child,2,1000,500)==120);assert(PoaGuiLayout::Resolve(&child,3,1000,500)==130);
 child.Numbers["WinWidth"]=1;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==500);child.Numbers["WinWidth"]=1.01f;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==1.01f);
 child.Numbers["WinWidth"]=-.5f;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==-250);
 child.Scale=false;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==-500);
 child.Numbers["WinWidth"]=NAN;assert(std::isnan(PoaGuiLayout::Resolve(&child,0,1000,500)));
 child.Controller=nullptr;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==0);child.Controller=&root;
 child.Owner=&child;child.Scale=true;child.Numbers["WinWidth"]=.5f;assert(PoaGuiLayout::Resolve(&child,0,1000,500)==0);
 for(float width:{480.f,1280.f,1920.f,2560.f})for(float height:{480.f,720.f,1080.f}){child.Owner=&root;child.Scale=true;child.Numbers["WinWidth"]=.5f;assert(PoaGuiLayout::Resolve(&child,0,width,height)==width*.25f);}
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original GUI declaration checks')
 def test_original_layout_native_signatures(self):
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/GUI.u';m=inspect(p);data=p.read_bytes()
  for name in ('ActualWidth','ActualHeight','ActualLeft','ActualTop'):
   i,e=next((i,e)for i,e in enumerate(m['exports'],1)if e['name']==name and e['class']=='Function');r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);n=native_stub(r,h);self.assertTrue(n['native']);self.assertEqual(n['nativeIndex'],0);self.assertEqual(n['parameters'],[]);fields=[x for x in m['exports']if x['outer']==i];self.assertEqual([(x['name'],x['class'])for x in fields],[('ReturnValue','FloatProperty')])
if __name__=='__main__':unittest.main()
