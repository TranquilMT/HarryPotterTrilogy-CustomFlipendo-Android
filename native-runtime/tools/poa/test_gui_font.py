"""Production font selection/cache plus the original EU menu font assets."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
from inspect_packages import inspect,Reader
from object_payloads import name,tags
from bytecode import Decoder
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiFont(unittest.TestCase):
 def test_selection_cache_and_missing_assets(self):
  body=definition((ROOT/'src/hp3/PoaGui.cpp').read_text(),'static void GetFont(')
  code=r'''#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <cstdint>
struct UObject{virtual~UObject()=default;bool Fixed=false;bool GetBool(const char*){return Fixed;}template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}};struct UFont:UObject{};
enum class ExpressionValueType{ValueString,ValueInt,ValueObject};struct ScriptArray{ExpressionValueType Type;std::vector<std::string>Names;std::vector<int32_t>Sizes;std::vector<UObject*>Fonts;size_t GetSize(){return Type==ExpressionValueType::ValueString?Names.size():Type==ExpressionValueType::ValueInt?Sizes.size():Fonts.size();}void Resize(size_t n){Fonts.resize(n);}void*GetItem(size_t n){if(Type==ExpressionValueType::ValueString)return &Names.at(n);if(Type==ExpressionValueType::ValueInt)return &Sizes.at(n);return &Fonts.at(n);}};
ScriptArray names{ExpressionValueType::ValueString,{"HGame.Small","HGame.Medium","HGame.Large"},{},{}},sizes{ExpressionValueType::ValueInt,{}, {640,1024},{}},fonts{ExpressionValueType::ValueObject,{},{},{}};
ScriptArray&GuiArray(UObject*,const char*n,ExpressionValueType type){auto&p=std::string(n)=="FontArrayNames"?names:std::string(n)=="FontArrayResolutions"?sizes:fonts;assert(p.Type==type);return p;}
struct Package{UObject Class;UObject*GetClass(const char*){return &Class;}};struct Packages{Package P;Package*GetPackage(const char*){return &P;}};struct Engine{Packages*packages;};Packages packs;Engine instance{&packs};Engine*engine=&instance;
UFont small,medium,large;int Loads=0;struct NObject{static void DynamicLoadObject(const std::string&n,UObject*,bool mayFail,UObject*&result){assert(mayFail);++Loads;result=n=="HGame.Small"?&small:n=="HGame.Medium"?&medium:n=="HGame.Large"?&large:nullptr;}};
'''+body+r'''
int main(){UObject self,*result=&self;GetFont(&self,nullptr,639,result);assert(result==&small&&Loads==1&&fonts.Fonts.size()==1);GetFont(&self,nullptr,639,result);assert(result==&small&&Loads==1);GetFont(&self,nullptr,640,result);assert(result==&medium&&Loads==2);GetFont(&self,nullptr,1024,result);assert(result==&large&&Loads==3);GetFont(&self,nullptr,9000,result);assert(result==&large&&Loads==3);self.Fixed=true;GetFont(&self,nullptr,9000,result);assert(result==&small&&Loads==3);
self.Fixed=false;names.Names={"missing"};fonts.Fonts.clear();GetFont(&self,nullptr,9000,result);assert(!result&&fonts.Fonts.size()==1);names.Names.clear();GetFont(&self,nullptr,640,result);assert(!result);GetFont(nullptr,nullptr,640,result);assert(!result);}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original menu fonts')
 def test_eu_button_and_title_fonts_exist_in_original_package(self):
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/HGame.u';m=inspect(p);data=p.read_bytes();assets={e['name'] for e in m['exports'] if e['class']=='Font'}
  for cls,expected in [('FONT_HP_Button',['HPTinyInkFont','HPSmallInkFont','HPMedInkFont','HPBigInkFont','HPHugeInkFont']),('FONT_HP_PageTitle',['HPHugeInkFont']*5)]:
   e=next(e for e in m['exports']if e['class']=='Class'and e['name']==cls);r=Reader(data[e['offset']:e['offset']+e['size']])
   for _ in range(4):r.compact()
   name(r,m['names']);r.compact();r.u32();r.u32();size=r.u32();Decoder(r,m['names'],len(m['imports']),len(m['exports']),size).decode();r.read(42)
   for _ in range(r.compact()):r.compact();r.read(8)
   for _ in range(r.compact()):r.compact()
   r.compact();name(r,m['names'])
   for _ in range(r.compact()):name(r,m['names'])
   field=next(t for t in tags(r,m['names'])if t['name']=='FontArrayNames');a=Reader(r.data[field['offset']:field['offset']+field['size']]);names=[a.string()for _ in range(a.compact())];self.assertEqual(names,['HGame.'+v for v in expected]);self.assertTrue(set(expected)<=assets)
if __name__=='__main__':unittest.main()
