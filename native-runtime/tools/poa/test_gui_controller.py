"""Production controller initialization and resource lookup bodies through host adapters."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/hp3/PoaGui.cpp'
class GuiController(unittest.TestCase):
 def run_cpp(self,code):
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_controller_lifecycle_and_reentrant_initialization(self):
  s=SOURCE.read_text();body='\n'.join(definition(s,n)for n in ['UObject* Controller()','void Shutdown()','void Initialize()'])
  self.run_cpp(r'''#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include <stdexcept>
int Roots=0,Creates=0,Calls=0;struct UObject{bool Valid=true;bool IsA(const char*){return Valid;}};
template<class T>struct GCRoot{T*Value;GCRoot(T*v):Value(v){++Roots;}~GCRoot(){--Roots;}T*get(){return Value;}};
namespace Exception{void Throw(std::string s){throw std::runtime_error(s);}}
namespace ObjectFlags{constexpr int Transient=1;}
struct Package{UObject Class,Instance;UObject*GetClass(std::string name){assert(name=="HPGUIController");return &Class;}UObject*NewObject(const char*name,UObject*cls,int flags){assert(std::string(name)=="poa_gui_controller"&&cls==&Class&&flags==1);++Creates;return &Instance;}};
struct Packages{std::string Name="HGame.HPGUIController";Package P;std::string GetIniValue(const char*,const char*,const char*){return Name;}Package*GetPackage(std::string name){assert(name=="HGame");return &P;}Package*GetTransientPackage(){return &P;}};
struct Engine{struct{bool Poa=true;bool IsHarryPotter3(){return Poa;}}LaunchInfo;Packages*packages;void*viewport=nullptr,*canvas=nullptr;};Engine*engine=nullptr;
std::function<void()>Hook;void CallEvent(UObject*,const char*name){assert(Roots==1&&std::string(name)=="InitializeController");++Calls;if(Hook)Hook();}
namespace PoaGui{static std::unique_ptr<GCRoot<UObject>>controller;static bool initializing=false;
'''+body+r'''}
int main(){using namespace PoaGui;Initialize();assert(!Controller());Packages p;Engine e;e.packages=&p;engine=&e;Initialize();assert(Creates==0);e.viewport=&e;e.canvas=&e;e.LaunchInfo.Poa=false;Initialize();assert(Creates==0);e.LaunchInfo.Poa=true;
p.Name="Invalid";bool threw=false;try{Initialize();}catch(...){threw=true;}assert(threw&&!Controller()&&Roots==0);p.Name="HGame.HPGUIController";
p.P.Instance.Valid=false;threw=false;try{Initialize();}catch(...){threw=true;}assert(threw&&!Controller()&&Roots==0);p.P.Instance.Valid=true;
Hook=[](){Initialize();};Initialize();assert(Controller()==&p.P.Instance&&Calls==1&&Roots==1);Initialize();assert(Calls==1);Shutdown();assert(!Controller()&&Roots==0);
Hook=[](){throw std::runtime_error("script initialization failed");};threw=false;try{Initialize();}catch(...){threw=true;}assert(threw&&!Controller()&&Roots==0);Hook={};Initialize();assert(Controller()&&Roots==1);Shutdown();assert(Roots==0);
}
''')
 def test_production_style_and_font_stack_lookup(self):
  s=SOURCE.read_text();body='\n'.join(definition(s,n)for n in ['static UObject* Lookup(','static void GetStyle(','static void GetMenuFont('])
  self.run_cpp(r'''#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <cctype>
struct UObject;
enum class ExpressionValueType{ValueObject,Other};struct Inner{ExpressionValueType ValueType=ExpressionValueType::ValueObject;size_t ElementSize(){return sizeof(UObject*);}};
struct UArrayProperty{struct Inner*Inner;};struct ScriptArray{std::vector<UObject*>Items;size_t GetSize(){return Items.size();}void*GetItem(size_t n){return &Items.at(n);}};
struct UObject{std::string Key;ScriptArray Styles,Fonts;bool Has=true;struct Inner Type;UArrayProperty Prop{&Type};bool HasProperty(const char*){return Has;}template<class T>static T*TryCast(UArrayProperty*p){return static_cast<T*>(p);}UArrayProperty*GetMemberProperty(const char*){return &Prop;}void*GetProperty(UArrayProperty*){return Current;}ScriptArray*Current=nullptr;const std::string&GetString(const char*){return Key;}};
namespace Exception{void Throw(std::string s){throw std::runtime_error(s);}}
namespace StrTools{bool equals_ignore_case(std::string a,std::string b){for(char&v:a)v=std::tolower((unsigned char)v);for(char&v:b)v=std::tolower((unsigned char)v);return a==b;}}
'''+body+r'''
int main(){UObject self,a,b,other;self.Current=&self.Styles;a.Key="MenuStyle";b.Key="MENUSTYLE";other.Key="Other";self.Styles.Items={nullptr,&a,&b,&other};UObject*result=&other;GetStyle(&self,"menustyle",result);assert(result==&a);GetStyle(&self,"missing",result);assert(!result);self.Current=&self.Fonts;self.Fonts.Items={&other};GetMenuFont(&self,"other",result);assert(result==&other);GetMenuFont(nullptr,"other",result);assert(!result);self.Type.ValueType=ExpressionValueType::Other;bool threw=false;try{GetStyle(&self,"menu",result);}catch(...){threw=true;}assert(threw);}
''')
 def test_production_control_collection_order_and_existing_lists(self):
  body=definition(SOURCE.read_text(),'static void InitializeControls(')
  self.run_cpp(r'''#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <stdexcept>
#include <cstdint>
struct UObject;struct UField{virtual~UField()=default;UField*Next=nullptr;};
enum class ExpressionValueType{ValueObject,Other};struct UProperty:UField{uint32_t PropFlags=0;};struct Inner{ExpressionValueType ValueType=ExpressionValueType::ValueObject;};struct UArrayProperty:UProperty{struct Inner*Inner;};
struct UStruct{std::string Name;UStruct*BaseStruct=nullptr;UField*Children=nullptr;};struct UClass:UStruct{};
struct UObjectProperty:UProperty{UClass*ObjectClass=nullptr;};
struct ScriptArray{std::vector<UObject*>Items;size_t GetSize(){return Items.size();}void Resize(size_t n){Items.resize(n);}void*GetItem(size_t n){return &Items.at(n);}};
int Roots=0;template<class T>struct GCRoot{GCRoot(T*){++Roots;}~GCRoot(){--Roots;}};
struct UObject{UClass*Class=nullptr;float Weight=0;struct Inner Type;UArrayProperty Array;ScriptArray Controls;std::map<UProperty*,UObject*>Values;UObject(){Array.Inner=&Type;}bool HasProperty(const char*){return true;}template<class T>static T*TryCast(UField*f){return dynamic_cast<T*>(f);}UField*GetMemberProperty(const char*){return &Array;}void*GetProperty(UProperty*p){return p==&Array?static_cast<void*>(&Controls):static_cast<void*>(&Values[p]);}float GetFloat(const char*){return Weight;}};
void InstantiateControls(UObject*){}
namespace Exception{void Throw(std::string s){throw std::runtime_error(s);}}
'''+body+r'''
int main(){UClass component,derived,base,page;component.Name="GUIComponent";derived.BaseStruct=&component;page.BaseStruct=&base;UObject self,a,b,c,ignored;self.Class=&page;a.Weight=4;b.Weight=2;c.Weight=2;
UObjectProperty pa,pb,pc,skip;pa.PropFlags=pb.PropFlags=pc.PropFlags=0x80000000u;pa.ObjectClass=pb.ObjectClass=pc.ObjectClass=skip.ObjectClass=&derived;pa.Next=&skip;skip.Next=&pb;page.Children=&pa;base.Children=&pc;
self.Values[&pa]=&a;self.Values[&pb]=&b;self.Values[&pc]=&c;self.Values[&skip]=&ignored;InitializeControls(&self);assert(Roots==0);assert((self.Controls.Items==std::vector<UObject*>{&c,&b,&a}));
a.Weight=-10;InitializeControls(&self);assert(self.Controls.Items[0]==&c);self.Controls.Items.clear();self.Values[&pb]=nullptr;InitializeControls(&self);assert((self.Controls.Items==std::vector<UObject*>{&a,&c}));InitializeControls(nullptr);assert(Roots==0);
}
''')
 def test_scaled_drawing_preserves_canvas_cursor_even_on_error(self):
  body=definition(SOURCE.read_text(),'static void DrawTileScaled(')
  self.run_cpp(r'''#include <stdexcept>
#include <cassert>
struct UObject{virtual~UObject()=default;template<class T>static T*Cast(UObject*v){auto p=dynamic_cast<T*>(v);if(!p)throw std::runtime_error("bad canvas");return p;}template<class T>static T*TryCast(UObject*v){return dynamic_cast<T*>(v);}};
struct UCanvas:UObject{float X=10,Y=20,Line=30;int Mode=2;float&CurX(){return X;}float&CurY(){return Y;}float&CurYL(){return Line;}int Style(){return Mode;}};struct UTexture:UObject{int USize(){return 40;}int VSize(){return 20;}};
namespace Exception{void Throw(const char*s){throw std::runtime_error(s);}}
struct NCanvas{static inline bool Fail=false;static inline int Calls=0;static void DrawTile(UCanvas*c,UTexture*,float w,float h,float u,float v,float ul,float vl){assert(w==80&&h==60&&u==0&&v==0&&ul==40&&vl==20);++Calls;c->X=90;c->Y=70;c->Line=100;if(Fail)throw std::runtime_error("draw failure");}};
'''+body+r'''
int main(){UCanvas c;UTexture t;DrawTileScaled(&c,&t,2,3);assert(NCanvas::Calls==1&&c.X==10&&c.Y==20&&c.Line==30);NCanvas::Fail=true;bool threw=false;try{DrawTileScaled(&c,&t,2,3);}catch(...){threw=true;}assert(threw&&c.X==10&&c.Y==20&&c.Line==30);c.Mode=0;DrawTileScaled(&c,&t,2,3);assert(NCanvas::Calls==2);DrawTileScaled(&c,nullptr,2,3);assert(NCanvas::Calls==2);}
''')
 def test_original_menu_class_and_resolution_accessors(self):
  body='\n'.join(definition(SOURCE.read_text(),n)for n in ['static void GetMainMenuClass(','static void GetCurrentRes('])
  self.run_cpp(r'''#include <string>
#include <cassert>
struct UObject{};struct Game{bool Have=true;std::string Main="HGame.OriginalMenu";bool HasProperty(const char*){return Have;}std::string GetString(const char*){return Main;}};struct Viewport{int Width=1920,Height=1080;int ViewportWidth(){return Width;}int ViewportHeight(){return Height;}};struct Engine{Game*gameengine=nullptr;Viewport*viewport=nullptr;};Engine*engine=nullptr;
'''+body+r'''
int main(){std::string value="stale";GetMainMenuClass(nullptr,value);assert(value.empty());GetCurrentRes(nullptr,value);assert(value.empty());Game game;Viewport viewport;Engine e;e.gameengine=&game;e.viewport=&viewport;engine=&e;GetMainMenuClass(nullptr,value);assert(value==game.Main);GetCurrentRes(nullptr,value);assert(value=="1920x1080");viewport.Width=2400;GetCurrentRes(nullptr,value);assert(value=="2400x1080");game.Have=false;GetMainMenuClass(nullptr,value);assert(value.empty());}
''')
if __name__=='__main__':unittest.main()
