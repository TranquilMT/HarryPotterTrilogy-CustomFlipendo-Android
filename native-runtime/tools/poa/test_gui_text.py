"""Execute production menu text drawing with observable Canvas, VM and font adapters."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiText(unittest.TestCase):
 def test_alignment_delegates_and_canvas_state(self):
  source=(ROOT/'src/hp3/PoaGuiText.cpp').read_text();source='\n'.join(l for l in source.splitlines()if not l.startswith('#include'));source=source.replace(definition(source,'void RegisterNatives()'),'')
  code=r'''#include <string>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
#include <functional>
struct vec2{float x,y;};struct Color{uint8_t R=0,G=0,B=0,A=255;bool operator==(const Color&)const=default;};
struct NameString{std::string Value;NameString()=default;NameString(const char*s):Value(s){}bool IsNone(){return Value.empty();}};
struct UObject;struct UProperty{virtual~UProperty()=default;int ArrayDimension=5,Id=0;size_t Size=0;size_t ElementSize(){return Size;}void*GetElement(void*p,size_t i){return static_cast<char*>(p)+i*Size;}};
struct UPoaDelegateProperty:UProperty{UObject*Receiver=nullptr;bool Bound=false;void ResolveBinding(UObject*&receiver,NameString&function){if(Bound){receiver=Receiver;function=NameString("CustomDraw");}}};
struct UObject{virtual~UObject()=default;UObject*Fonts[5]={};uint8_t Styles[5]={};Color Colors[5];UProperty FontProperty,StyleProperty,ColorProperty;UPoaDelegateProperty Delegate;
UObject(){FontProperty.Id=1;FontProperty.Size=sizeof(UObject*);StyleProperty.Id=2;StyleProperty.Size=1;ColorProperty.Id=3;ColorProperty.Size=sizeof(Color);Delegate.Receiver=this;}
UProperty*GetMemberProperty(const char*n){auto s=std::string(n);return s=="Fonts"?&FontProperty:s=="RStyles"?&StyleProperty:s=="FontColors"?&ColorProperty:&Delegate;}void*GetProperty(UProperty*p){return p->Id==1?static_cast<void*>(Fonts):p->Id==2?static_cast<void*>(Styles):static_cast<void*>(Colors);}template<class T,class S>static T*TryCast(S*p){return dynamic_cast<T*>(p);}template<class T>static T*Cast(UObject*p){auto t=dynamic_cast<T*>(p);if(!t)throw std::runtime_error("cast");return t;}};
struct UFont:UObject{};
struct UCanvas:UObject{float X=9,Y=10,OX=2.75f,OY=-3.75f,CX=101.75f,CY=81.25f;bool Center=true;UFont*F=nullptr;Color C{9,8,7,255};uint8_t S=2;float&CurX(){return X;}float&CurY(){return Y;}float&OrgX(){return OX;}float&OrgY(){return OY;}float&ClipX(){return CX;}float&ClipY(){return CY;}bool&bCenter(){return Center;}UFont*&Font(){return F;}Color&DrawColor(){return C;}uint8_t&Style(){return S;}int SizeX(){return 1000;}};
int Roots=0;template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}T*get(){return Value;}};
struct ExpressionValue{UObject*Object=nullptr;bool Bool=false;int Integer=0;float Float=0;std::string String;static ExpressionValue ObjectValue(UObject*p){ExpressionValue e;e.Object=p;return e;}static ExpressionValue ByteValue(uint8_t v){ExpressionValue e;e.Integer=v;return e;}static ExpressionValue IntValue(int v){ExpressionValue e;e.Integer=v;return e;}static ExpressionValue FloatValue(float v){ExpressionValue e;e.Float=v;return e;}static ExpressionValue StringValue(std::string v){ExpressionValue e;e.String=v;return e;}bool ToBool(){return Bool;}UObject*ToObject(){return Object;}};
UFont Loaded;int FontCalls=0,DelegateCalls=0;bool Consume=false;std::function<void()>Hook;
ExpressionValue CallEvent(UObject*,NameString n,std::initializer_list<ExpressionValue>args){ExpressionValue r;if(n.Value=="GetFont"){assert(args.size()==2&&Roots>=4);assert((args.begin()+1)->Integer==1000);++FontCalls;r.Object=&Loaded;}else{assert(n.Value=="CustomDraw"&&args.size()==8&&Roots>=3);++DelegateCalls;assert((args.begin()+2)->Float==0);assert((args.begin()+3)->Float==3);assert((args.begin()+4)->Float==100);assert((args.begin()+5)->Float==41);assert((args.begin()+7)->String=="100% magic");if(Hook)Hook();r.Bool=Consume;}return r;}
struct NCanvas{static inline bool Fail=false;static inline int Draws=0;static inline float Width=20,Height=11;static inline float SeenX=0,SeenY=0,SeenOX=0,SeenOY=0;static inline uint8_t SeenStyle=0,SeenRed=0;static inline UFont*SeenFont=nullptr;
static void StrLen(UCanvas*c,const std::string&,float&w,float&h){assert(!c->Center&&c->X==0&&c->Y==0);w=Width;h=Height;}static void DrawTextClipped(UCanvas*c,const std::string&,bool hotkey){assert(!hotkey&&!c->Center);++Draws;SeenX=c->X;SeenY=c->Y;SeenOX=c->OX;SeenOY=c->OY;SeenStyle=c->S;SeenRed=c->C.R;SeenFont=c->F;if(Fail)throw std::runtime_error("draw failure");}};
'''+source+r'''
int main(){using namespace PoaGuiText;assert(JustifiedPosition(0,100,40,20,11).x==0);assert(JustifiedPosition(1,100,40,21,11).x==40);assert(JustifiedPosition(1,10,40,21,11).x==0);assert(JustifiedPosition(2,10,40,21,11).x==-11);assert(JustifiedPosition(1,100,40,21,11).y==15);
UCanvas c;UFont old;c.F=&old;DrawTextJustified(&c,1,10,20,110,60,"text");assert(c.X==40&&c.Y==15&&c.OX==2&&c.OY==-3&&c.CX==101&&c.CY==81&&c.Center);assert(NCanvas::SeenOX==10&&NCanvas::SeenOY==20);
c.OX=2.75f;c.OY=-3.75f;c.CX=101.75f;c.CY=81.25f;c.X=9;c.Y=10;NCanvas::Fail=true;bool threw=false;try{DrawTextJustified(&c,1,10,20,110,60,"text");}catch(...){threw=true;}assert(threw&&c.OX==2.75f&&c.CX==101.75f&&c.X==9&&c.Y==10&&c.Center);NCanvas::Fail=false;
UObject style,fontObject;style.Fonts[2]=&fontObject;style.Styles[2]=3;style.Colors[2]={100,20,30,255};style.Delegate.Bound=true;auto oldColor=c.C;int before=NCanvas::Draws;
DrawStyleText(&style,&c,5,0,0,100,40,0,"text");assert(NCanvas::Draws==before&&DelegateCalls==0);
DrawStyleText(&style,&c,2,-.5f,2.5f,100.49f,40.5f,1,"100% magic");assert(FontCalls==1&&DelegateCalls==1&&NCanvas::SeenFont==&Loaded&&NCanvas::SeenStyle==3&&NCanvas::SeenRed==100&&NCanvas::SeenOX==0&&NCanvas::SeenOY==3&&NCanvas::SeenY==15.5f);assert(c.F==&old&&c.S==2&&c.C==oldColor&&Roots==0);
Consume=true;Hook=[&](){c.C.R=77;};before=NCanvas::Draws;DrawStyleText(&style,&c,2,-.5f,2.5f,100.49f,40.5f,1,"100% magic");assert(NCanvas::Draws==before&&c.C.R==77&&FontCalls==1&&Roots==0);
Consume=false;Hook=[&](){style.Styles[2]=4;style.Colors[2].R=88;c.C.R=66;c.S=7;c.F=nullptr;};NCanvas::Fail=true;auto saved=c.C;threw=false;try{DrawStyleText(&style,&c,2,-.5f,2.5f,100.49f,40.5f,1,"100% magic");}catch(...){threw=true;}assert(threw&&NCanvas::SeenStyle==4&&NCanvas::SeenRed==88&&c.F==&old&&c.S==2&&c.C==saved&&Roots==0);
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-function','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
