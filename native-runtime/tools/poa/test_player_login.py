"""Execute production UE2 login bridge with observable original-script adapters."""
import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class PlayerLogin(unittest.TestCase):
 def test_signatures_owner_links_and_failure_rollback(self):
  source='\n'.join(l for l in (ROOT/'src/hp3/PoaPlayer.cpp').read_text().splitlines()if not l.startswith('#include'))
  code=r'''#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <initializer_list>
enum class ExpressionValueType{ValueByte,ValueBool,Other};struct UProperty{ExpressionValueType ValueType=ExpressionValueType::ValueByte;};
struct UObject{UProperty ButtonProperty;int Button=0;UProperty*GetMemberProperty(const std::string&){return &ButtonProperty;}void SetByte(const std::string&,int v){Button=v;}void SetBool(const std::string&,bool v){Button=v;}virtual~UObject()=default;bool PC=false;UObject*Player=nullptr,*Target=nullptr;bool HasProperty(const char*){return true;}bool IsA(const char*){return PC;}UObject*GetUObject(const char*n){return std::string(n)=="Player"?Player:Target;}void SetObject(const char*,UObject*p){Player=p;}template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}};
struct UActor:UObject{};struct UViewport:UObject{UActor*Actor=nullptr;UActor*&ViewActor(){return Actor;}};
enum class ObjectFlags{NoFlags};struct UStrProperty:UObject{UStrProperty(const char*,std::nullptr_t,ObjectFlags){}};
UStrProperty Property("",nullptr,ObjectFlags::NoFlags);struct GC{template<class T,class...A>static T*Alloc(A...){return &Property;}};
int Roots=0;template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}};
struct ExpressionValue{UObject*Object=nullptr;std::string String;std::string*VariablePointer=nullptr;static ExpressionValue StringValue(std::string s){ExpressionValue e;e.String=s;return e;}static ExpressionValue ObjectValue(UObject*p){ExpressionValue e;e.Object=p;return e;}static ExpressionValue Variable(std::string*p,UStrProperty*){ExpressionValue e;e.VariablePointer=p;return e;}UObject*ToObject(){return Object;}};
namespace Exception{void Throw(const std::string&s){throw std::runtime_error(s);}}
UActor Controller,Old,Replacement;UViewport Viewport;UObject Game;std::vector<std::string>Calls;bool RejectPre=false,RejectLogin=false,ThrowPost=false,ReplaceBeforeThrow=false;
namespace PoaGui{void Initialize(){Calls.push_back("GUI");assert(Viewport.Actor==&Controller&&Controller.Player==&Viewport&&Roots==2);}}
ExpressionValue CallEvent(UObject*o,const char*n,std::initializer_list<ExpressionValue>args={}){Calls.push_back(n);auto a=args.begin();ExpressionValue r;
 if(std::string(n)=="PreLogin"){assert(o==&Game&&args.size()==4&&a[0].String=="?Name=Harry"&&a[1].String.empty()&&a[2].VariablePointer&&a[3].VariablePointer&&Roots==1);if(RejectPre)*a[2].VariablePointer="denied";}
 else if(std::string(n)=="Login"){assert(o==&Game&&args.size()==3&&a[0].String=="portal"&&a[1].String=="?Name=Harry"&&Roots==1);if(RejectLogin)*a[2].VariablePointer="failed";r.Object=&Controller;}
 else if(std::string(n)=="InitInputSystem"){assert(o==&Controller&&args.size()==0&&Viewport.Actor==&Controller&&Roots==2);}
 else if(std::string(n)=="PostLogin"){assert(o==&Game&&args.size()==1&&a[0].Object==&Controller&&Roots==2);if(ReplaceBeforeThrow)Viewport.Actor=&Replacement;if(ThrowPost)throw std::runtime_error("post");}
 else assert(false);return r;}
'''+source+r'''
int main(){PoaPlayer::SetButton(&Controller,"bFire",true);assert(Controller.Button==1);Controller.ButtonProperty.ValueType=ExpressionValueType::ValueBool;PoaPlayer::SetButton(&Controller,"bJump",false);assert(Controller.Button==0);Controller.PC=true;Viewport.Actor=&Old;PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");assert((Calls==std::vector<std::string>{"PreLogin","Login","GUI","InitInputSystem","PostLogin"}));assert(Viewport.Actor==&Controller&&Controller.Player==&Viewport&&Roots==0);
Controller.Target=&Old;assert(PoaPlayer::ViewTarget(&Controller)==&Old);Controller.Target=nullptr;assert(PoaPlayer::ViewTarget(&Controller)==&Controller);assert(!PoaPlayer::ViewTarget(nullptr));
Viewport.Actor=&Old;Controller.Player=nullptr;RejectPre=true;Calls.clear();bool threw=false;try{PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");}catch(...){threw=true;}assert(threw&&Calls.size()==1&&Viewport.Actor==&Old&&Roots==0);RejectPre=false;
RejectLogin=true;threw=false;try{PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");}catch(...){threw=true;}assert(threw&&Viewport.Actor==&Old&&Roots==0);RejectLogin=false;
Controller.PC=false;threw=false;try{PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");}catch(...){threw=true;}assert(threw&&Viewport.Actor==&Old&&Roots==0);Controller.PC=true;
ThrowPost=true;threw=false;try{PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");}catch(...){threw=true;}assert(threw&&Viewport.Actor==&Old&&!Controller.Player&&Roots==0);
ReplaceBeforeThrow=true;threw=false;try{PoaPlayer::Login(&Game,&Viewport,"portal","?Name=Harry");}catch(...){threw=true;}assert(threw&&Viewport.Actor==&Replacement&&!Controller.Player&&Roots==0);
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
