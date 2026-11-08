"""Execute production Core bodies through production VM adapter templates.
ExpressionValue containers/registry and UObject storage are test doubles, not a game VM.
"""
import pathlib,re,subprocess,tempfile,unittest,os
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
def definition(text,signature):
 start=text.index(signature);brace=text.index('{',start);depth=1;end=brace+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
class CoreSeven(unittest.TestCase):
 def compile(self,code,args=()):
  with tempfile.TemporaryDirectory() as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ROOT/'src'),'-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test'),*args],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_all_seven_adapter_argument_and_return_slots(self):
  h=(ENGINE/'VM/NativeFunc.h').read_text();wrappers=''
  for sig in ('void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2)', 'void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3)', 'void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1)'):
   template=h.rfind('template<',0,h.index(sig));wrappers+=h[template:h.index(sig)]+definition(h,sig)+'\n'
  body=(ROOT/'src/hp3/PoaCore.cpp').read_text();body='\n'.join(s for s in body.splitlines() if not s.startswith('#include'))
  comparison=definition((ENGINE/'Native/NObject.cpp').read_text(),'void NObject::Greater_StrStr(')
  expression=(ENGINE/'VM/ExpressionValue.h').read_text()
  generic=definition(expression,'template<typename T> T ToType()')
  conversions='\n'.join(definition(expression,sig) for sig in ('inline float ExpressionValue::ToFloat() const','inline const vec3& ExpressionValue::ToVector() const','inline const std::string& ExpressionValue::ToString() const','inline const quaternion& ExpressionValue::ToQuat() const','inline const NameString& ExpressionValue::ToName() const'))
  specializations='\n'.join(line for line in expression.splitlines() if line.startswith('template<> inline ') and any((' '+t+' ExpressionValue::ToType()') in line for t in ('float','float&','const vec3&','vec3&','const std::string&','NameString&','BitfieldBool&','const quaternion&','quaternion&')))
  boolbody=definition((ENGINE/'Packages/Core/UObject.h').read_text(),'struct BitfieldBool')+';'

  code=r'''#include "hp3/PoaMath.h"
#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <cstdint>
struct NameString:std::string{using std::string::string;NameString(const std::string&s):std::string(s){}const std::string&ToString()const{return *this;}};
'''+boolbody+r'''
struct UObject{std::string last;NameString GetName(const char* key){assert(std::string(key)=="LastStateName");return NameString(last);}};
struct Exception{[[noreturn]]static void Throw(const char*s){throw std::runtime_error(s);}};
enum class ExpressionValueType{Nothing,ValueFloat,ValueInt,ValueByte,ValueStruct,ValueQuat,ValueVector,ValueString,ValueName,ValueBool};
struct StructValue{void*Ptr=nullptr;};
struct ExpressionValue{
 ExpressionValueType Type;void*VariableProperty=nullptr;StructValue sv;
 union{void*Ptr;float*PtrFloat;int32_t*PtrInt;uint8_t*PtrByte;quaternion*PtrQuat;vec3*PtrVector;std::string*PtrString;NameString*PtrName;};BitfieldBool BoolInfo{nullptr,1};
 ExpressionValue(float*p):Type(ExpressionValueType::ValueFloat),Ptr(p){}ExpressionValue(quaternion*p):Type(ExpressionValueType::ValueQuat),Ptr(p){}ExpressionValue(vec3*p):Type(ExpressionValueType::ValueVector),Ptr(p){}ExpressionValue(std::string*p):Type(ExpressionValueType::ValueString),Ptr(p){}ExpressionValue(NameString*p):Type(ExpressionValueType::ValueName),Ptr(p){}ExpressionValue(BitfieldBool*p):Type(ExpressionValueType::ValueBool),Ptr(nullptr),BoolInfo(*p){}
 void CheckType(ExpressionValueType t){if(Type!=t)throw std::runtime_error("type mismatch");}StructValue*GetStructValue(){return &sv;}
 float ToFloat()const;const vec3&ToVector()const;const std::string&ToString()const;const quaternion&ToQuat()const;const NameString&ToName()const;
'''+generic+r'''
};
'''+conversions+'\n'+specializations+r'''

struct NativeFunctions{
 static inline std::map<std::string,std::function<void(UObject*,ExpressionValue*)>> handlers;
 static inline std::map<std::string,int> indices;
 template<class F>static void RegisterHandler(const std::string&c,const std::string&f,int index,F fn){assert(c=="Object");assert(!handlers.count(f));handlers[f]=fn;indices[f]=index;}
};
struct NObject{static void Greater_StrStr(const std::string&,const std::string&,BitfieldBool&);};
'''+comparison+'\n'+wrappers+'\n'+body+r'''
bool near(float a,float b){return std::abs(a-b)<0.0001f;}
int main(){
 PoaCore::RegisterNatives();assert(NativeFunctions::handlers.size()==7);assert(NativeFunctions::indices["Greater_StrStr"]==116);assert(NativeFunctions::indices["GetLastStateName"]==285);
 UObject self;
 for(auto f:{"Acos","Asin"})for(float x:{-1.f,0.f,0.5f,1.f}){float result=-99.f;ExpressionValue args[]={{&x},{&result}};NativeFunctions::handlers[f](&self,args);assert(near(result,std::string(f)=="Acos"?std::acos(x):std::asin(x)));assert(x>=-1.f&&x<=1.f);}
 std::string a="b",b="a";uint32_t bits=0xa0;BitfieldBool result{&bits,4};ExpressionValue cmp[]={{&a},{&b},{&result}};NativeFunctions::handlers["Greater_StrStr"](&self,cmp);assert(result.Get()&&bits==0xa4);a="a";NativeFunctions::handlers["Greater_StrStr"](&self,cmp);assert(!result.Get()&&bits==0xa0);a="";NativeFunctions::handlers["Greater_StrStr"](&self,cmp);assert(!result.Get());
 NameString state="sentinel";ExpressionValue names[]={{&state}};self.last="Combat";NativeFunctions::handlers["GetLastStateName"](&self,names);assert(state=="Combat");self.last="";NativeFunctions::handlers["GetLastStateName"](&self,names);assert(state.empty());
 float pi=std::acos(-1.f);vec3 axis{0,0,2};float angle=pi/2;quaternion q;ExpressionValue aa[]={{&axis},{&angle},{&q}};NativeFunctions::handlers["QuatFromAxisAndAngle"](&self,aa);assert(near(q.z,std::sqrt(.5f))&&near(q.w,std::sqrt(.5f)));assert(axis.z==2);
 vec3 v{1,0,0},rotated;ExpressionValue rv[]={{&q},{&v},{&rotated}};NativeFunctions::handlers["QuatRotateVector"](&self,rv);assert(near(rotated.x,0)&&near(rotated.y,1)&&near(rotated.z,0));assert(v.x==1);
 quaternion q2;ExpressionValue prod[]={{&q},{&q},{&q2}};NativeFunctions::handlers["QuatProduct"](&self,prod);ExpressionValue twice[]={{&q2},{&v},{&rotated}};NativeFunctions::handlers["QuatRotateVector"](&self,twice);assert(near(rotated.x,-1)&&near(rotated.y,0));
 quaternion x{1,0,0,0},y{0,1,0,0},xy,yx;ExpressionValue p1[]={{&x},{&y},{&xy}},p2[]={{&y},{&x},{&yx}};NativeFunctions::handlers["QuatProduct"](&self,p1);NativeFunctions::handlers["QuatProduct"](&self,p2);assert(near(xy.z,1)&&near(yx.z,-1));
 axis={0,0,0};NativeFunctions::handlers["QuatFromAxisAndAngle"](&self,aa);assert(q.x==0&&q.y==0&&q.z==0&&q.w==1);
}
'''
  self.compile(code)
 def test_production_previous_state_tracking(self):
  body=definition((ENGINE/'Packages/Core/UObject.cpp').read_text(),'void UObject::GotoState(')
  code=r'''#include <string>
#include <map>
#include <memory>
#include <functional>
#include <set>
#include <cstdint>
#include <cassert>
struct NameString:std::string{using std::string::string;NameString()=default;bool IsNone()const{return empty()||*this=="None";}};
enum class ObjectFlags:uint32_t{InEndState=1,StateChanged=2,HasStack=4};
ObjectFlags operator|(ObjectFlags a,ObjectFlags b){return ObjectFlags(uint32_t(a)|uint32_t(b));}ObjectFlags&operator|=(ObjectFlags&a,ObjectFlags b){return a=a|b;}bool AllFlags(ObjectFlags a,ObjectFlags b){return (uint32_t(a)&uint32_t(b))==uint32_t(b);}
enum class ScriptStateFlags{None=0,Auto=1};ScriptStateFlags operator&(ScriptStateFlags a,ScriptStateFlags b){return ScriptStateFlags(int(a)&int(b));}
struct UState{NameString Name;ScriptStateFlags StateFlags=ScriptStateFlags::None;};
struct UClass{UClass*BaseStruct=nullptr;std::map<NameString,UState*>States;UState*GetState(const NameString&n){auto it=States.find(n);return it==States.end()?nullptr:it->second;}};
struct Frame{UState*state=nullptr;Frame(void*,void*,int,bool){}UState*GetState(){return state;}void SetState(UState*s){state=s;}void GotoLabel(const NameString&){} };
struct Launch{bool hp3=true;bool IsHarryPotter3(){return hp3;}};struct Engine{Launch LaunchInfo;};Engine eng;Engine*engine=&eng;
enum class EventName{EndState,BeginState};
struct UObject{UClass*Class;std::shared_ptr<Frame>StateFrame;ObjectFlags Flags=ObjectFlags(0);std::set<int>DisabledEvents;NameString last;std::function<void(EventName)>event;void SetName(const char*,NameString n){last=n;}void GotoState(NameString,const NameString&);};
void CallEvent(UObject*p,EventName e){if(p->event)p->event(e);}
'''+body+r'''
int main(){UState a{"A"},b{"B"},c{"C"};UClass cls;cls.States={{"A",&a},{"B",&b},{"C",&c}};UObject o{&cls,{},ObjectFlags(0),{},"",{}};
 o.GotoState("A","");assert(o.last.IsNone());o.GotoState("B","");assert(o.last=="A");o.GotoState("B","");assert(o.last=="A");o.GotoState("None","");assert(o.last=="B"&&o.StateFrame->GetState()==nullptr);
 o.GotoState("A","");o.event=[&](EventName e){if(e==EventName::EndState)o.GotoState("C","");};o.GotoState("B","");assert(o.StateFrame->GetState()==&c&&o.last=="A");o.event={};
 engine->LaunchInfo.hp3=false;o.last="unchanged";o.GotoState("B","");assert(o.last=="unchanged");
}
'''
  self.compile(code)
 def test_optional_config_constructor_preserves_path_and_existing_file_errors(self):
  text=(ENGINE/'Package/IniFile.cpp').read_text();ctor=definition(text,'IniFile::IniFile(const std::string& filename, bool allowMissing)');reader=definition(text,'bool IniFile::ReadLine(')
  code=r'''#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cassert>
#include <stdexcept>
namespace fs=std::filesystem;
struct File{static std::string read_all_text(const std::string&p){if(!fs::is_regular_file(p))throw std::runtime_error("not a file");std::ifstream f(p);if(!f)throw std::runtime_error("read failed");return {std::istreambuf_iterator<char>(f),{}};}};
std::string DecodeIniText(const std::string&s){return s;}
struct IniSection{std::map<std::string,std::string> values;void SetValue(const std::string&n,const std::string&v,int,bool){values[n]=v;}};
struct IniFile{std::string ini_file_path;std::map<std::string,IniSection> sections;IniFile(const std::string&,bool=false);bool ReadLine(const std::string&,size_t&,std::string&);IniSection&AddUniqueSection(const std::string&s){return sections[s];}};
'''+ctor+'\n'+reader+r'''
int main(int,char**argv){auto dir=fs::path(argv[0]).parent_path()/std::string("poa-optional-ini-test");fs::remove_all(dir);fs::create_directory(dir);auto p=dir/"Hardware2Options.ini";
 bool rejected=false;try{IniFile strict(p.string());}catch(const std::exception&){rejected=true;}assert(rejected);
 IniFile missing(p.string(),true);assert(missing.sections.empty()&&missing.ini_file_path==p.string());assert(!fs::exists(p));
 std::ofstream(p)<<"[UDebugMenu.UGraphHardware2Page]\nValue=42\n";IniFile existing(p.string(),true);assert(existing.sections.begin()->second.values["Value"]=="42");
 fs::remove(p);fs::create_directory(p);rejected=false;try{IniFile invalid(p.string(),true);}catch(const std::exception&){rejected=true;}assert(rejected);fs::remove_all(dir);
}
'''
  self.compile(code)
if __name__=='__main__':unittest.main()
