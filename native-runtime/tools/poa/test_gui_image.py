"""Compare production stretching geometry to 225 original EU native captures."""
import os,json,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiImage(unittest.TestCase):
 def test_original_eu_geometry(self):
  fixture=json.loads((ROOT/'tools/poa/reference-profile/eu-gui-stretch.json').read_text())
  self.assertEqual(len(fixture['cases']),225)
  code='#include <cstdio>\n#include "PoaGuiImage.h"\nint main(){int id;float sw,sh,w,h;while(scanf("%d%f%f%f%f",&id,&sw,&sh,&w,&h)==5){PoaGuiImage::Stretched(10,20,w,h,sw,sh,[&](float x,float y,float a,float b,float u,float v,float ul,float vl){printf("%d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\\n",id,x,y,a,b,u,v,ul,vl);});}}'
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src/hp3'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   rows=''.join(f"{c['id']} {c['sourceWidth']} {c['sourceHeight']} {c['width']} {c['height']}\n"for c in fixture['cases'])
   output=subprocess.run([str(p/'test')],input=rows,text=True,capture_output=True,check=True,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0")).stdout
  actual={c['id']:[]for c in fixture['cases']}
  for line in output.splitlines():
   row=line.split();actual[int(row[0])].append(list(map(float,row[1:])))
  for c in fixture['cases']:
   with self.subTest(case=c['id']):
    self.assertEqual(len(actual[c['id']]),len(c['tiles']))
    for got,tile in zip(actual[c['id']],c['tiles']):
     for a,k in zip(got,('x','y','width','height','u','v','ul','vl')):self.assertAlmostEqual(a,tile[k],places=5)
    self.assertEqual((c['afterX'],c['afterY']),(10,20))
if __name__=='__main__':unittest.main()

class GuiStyleImage(unittest.TestCase):
 def test_production_dispatch_delegate_and_cursor(self):
  from test_core_seven_runtime import definition
  source=(ROOT/'src/hp3/PoaGuiDraw.cpp').read_text()
  source='\n'.join(line for line in source.splitlines()if not line.startswith('#include'))
  source=source.replace(definition(source,'void RegisterNatives()'),'')
  code=r'''#include <string>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
#include <functional>
#include <vector>
#include <array>
#include "PoaGuiImage.h"
struct Color{uint8_t R=0,G=0,B=0,A=255;};
struct NameString{std::string Value;NameString()=default;NameString(const char*s):Value(s){}bool IsNone(){return Value.empty();}};
struct UObject;struct UProperty{virtual~UProperty()=default;int ArrayDimension=5,Id=0;size_t Size=0;size_t ElementSize(){return Size;}void*GetElement(void*p,size_t i){return static_cast<char*>(p)+i*Size;}};
struct UPoaDelegateProperty:UProperty{UObject*Receiver=nullptr;bool Bound=false;void ResolveBinding(UObject*&receiver,NameString&function){if(Bound){receiver=Receiver;function=NameString("CustomDraw");}}};
struct UObject{virtual~UObject()=default;UObject*Images[5]={};uint8_t Styles[5]={},Modes[5]={};Color Colors[5];UProperty ImageProperty,StyleProperty,ColorProperty,ModeProperty;UPoaDelegateProperty Delegate;
UObject(){ImageProperty.Id=1;ImageProperty.Size=sizeof(UObject*);StyleProperty.Id=2;StyleProperty.Size=1;ColorProperty.Id=3;ColorProperty.Size=sizeof(Color);ModeProperty.Id=4;ModeProperty.Size=1;Delegate.Receiver=this;}
UProperty*GetMemberProperty(const char*n){auto s=std::string(n);return s=="Images"?&ImageProperty:s=="RStyles"?&StyleProperty:s=="ImgColors"?&ColorProperty:s=="ImgStyle"?&ModeProperty:&Delegate;}
void*GetProperty(UProperty*p){return p->Id==1?static_cast<void*>(Images):p->Id==2?static_cast<void*>(Styles):p->Id==3?static_cast<void*>(Colors):static_cast<void*>(Modes);}template<class T,class S>static T*TryCast(S*p){return dynamic_cast<T*>(p);}template<class T>static T*Cast(UObject*p){auto t=dynamic_cast<T*>(p);if(!t)throw std::runtime_error("cast");return t;}};
struct UTexture:UObject{int USize(){return 16;}int VSize(){return 8;}};
struct UCanvas:UObject{float X=9,Y=10,Line=11;Color C;uint8_t S=2;float&CurX(){return X;}float&CurY(){return Y;}float&CurYL(){return Line;}Color&DrawColor(){return C;}uint8_t&Style(){return S;}};
namespace Exception{void Throw(const char*s){throw std::runtime_error(s);}}
int Roots=0;template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}T*get(){return Value;}};
struct ExpressionValue{UObject*Object=nullptr;bool Bool=false;int Integer=0;float Float=0;static ExpressionValue ObjectValue(UObject*p){ExpressionValue e;e.Object=p;return e;}static ExpressionValue ByteValue(uint8_t v){ExpressionValue e;e.Integer=v;return e;}static ExpressionValue FloatValue(float v){ExpressionValue e;e.Float=v;return e;}bool ToBool(){return Bool;}};
int DelegateCalls=0;bool Consume=false;std::function<void()>Hook;
ExpressionValue CallEvent(UObject*,NameString n,std::initializer_list<ExpressionValue>args){assert(n.Value=="CustomDraw"&&args.size()==6&&Roots>=3);++DelegateCalls;if(Hook)Hook();ExpressionValue r;r.Bool=Consume;return r;}
struct NCanvas{static inline bool Fail=false;static inline std::vector<std::array<float,8>> Tiles;
static void DrawTile(UCanvas*c,UTexture*,float w,float h,float u,float v,float ul,float vl){Tiles.push_back({c->X,c->Y,w,h,u,v,ul,vl});c->X+=w;c->Line=99;if(Fail)throw std::runtime_error("draw");}};
'''+source+r'''
int main(){using namespace PoaGuiDraw;UCanvas c;UTexture image;UObject style;style.Images[2]=&image;style.Styles[2]=3;style.Colors[2].R=123;
DrawStyle(&style,&c,2,10,20,32,16);assert(NCanvas::Tiles.size()==1&&NCanvas::Tiles[0][2]==16&&NCanvas::Tiles[0][3]==8&&c.X==10&&c.Y==20&&c.Line==11&&c.S==3&&c.C.R==123&&Roots==0);
NCanvas::Tiles.clear();style.Modes[2]=2;DrawStyle(&style,&c,2,10,20,32,16);assert(NCanvas::Tiles.size()==1&&NCanvas::Tiles[0][2]==32&&NCanvas::Tiles[0][3]==16&&NCanvas::Tiles[0][6]==16);
NCanvas::Tiles.clear();style.Modes[2]=1;DrawStyle(&style,&c,2,10,20,32,16);assert(NCanvas::Tiles.size()==9&&c.X==10&&c.Y==20&&c.Line==11);
NCanvas::Tiles.clear();style.Delegate.Bound=true;Consume=true;Hook=[&](){c.C.R=77;};DrawStyle(&style,&c,2,10,20,32,16);assert(NCanvas::Tiles.empty()&&c.C.R==77&&DelegateCalls==1&&Roots==0);
Consume=false;Hook=[&](){style.Modes[2]=2;};DrawStyle(&style,&c,2,10,20,32,16);assert(NCanvas::Tiles.size()==1&&DelegateCalls==2&&c.C.R==123&&Roots==0);
NCanvas::Tiles.clear();DrawStyle(&style,&c,5,10,20,32,16);assert(NCanvas::Tiles.empty()&&DelegateCalls==2);
NCanvas::Fail=true;bool threw=false;try{DrawStyle(&style,&c,2,10,20,32,16);}catch(...){threw=true;}assert(threw&&c.X==10&&c.Y==20&&c.Line==11&&Roots==0);
NCanvas::Fail=false;c.X=7;c.Y=8;NCanvas::Tiles.clear();DrawTileStretched(&c,&image,32,16);assert(NCanvas::Tiles.size()==9&&c.X==7&&c.Y==8&&c.Line==11);
c.S=0;NCanvas::Tiles.clear();DrawTileStretched(&c,&image,32,16);assert(NCanvas::Tiles.empty());
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-function','-fsanitize=address,undefined','-I'+str(ROOT/'src/hp3'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
