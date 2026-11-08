"""Production VM adapters and array/reference conversions with value containers."""
import pathlib,subprocess,tempfile,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class ServiceAdapters(unittest.TestCase):
 def test_array_alias_return_slots_and_instance_arguments(self):
  text=(ENGINE/'VM/NativeFunc.h').read_text();wrappers=''
  for sig in ['void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1)','void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3)','void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2)']:
   start=text.rfind('template<',0,text.index(sig));wrappers+=text[start:text.index(sig)]+definition(text,sig)+'\n'
  expression=(ENGINE/'VM/ExpressionValue.h').read_text();types=['ScriptArray&','const std::string&','std::string&','int32_t','UObject*','UObject*&'];specializations='\n'.join(line for line in expression.splitlines() if line.startswith('template<> inline ') and any((' '+t+' ExpressionValue::ToType()')in line for t in types))
  code=r'''#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cassert>
struct UObject{};struct ScriptArray{std::vector<std::string>Values;};enum class ExpressionValueType{ValueString,ValueInt,ValueObject,ValueArray};
struct ExpressionValue{ExpressionValueType Type;union{void*Ptr;std::string*PtrString;int*PtrInt;UObject**PtrObject;};ExpressionValue(ExpressionValueType t,void*p):Type(t),Ptr(p){}void CheckType(ExpressionValueType t){if(Type!=t)throw std::runtime_error("type");}const std::string&ToString()const{if(Type!=ExpressionValueType::ValueString)throw std::runtime_error("string");return *PtrString;}int ToInt()const{return *PtrInt;}UObject*ToObject()const{return *PtrObject;}template<class T>T ToType();};
'''+specializations+r'''
struct NativeFunctions{using Handler=std::function<void(UObject*,ExpressionValue*)>;static std::map<std::string,Handler>ByName;static std::map<int,Handler>ByIndex;static void RegisterHandler(const std::string&,const std::string&name,int index,Handler handler){ByName[name]=handler;if(index)ByIndex[index]=handler;}};std::map<std::string,NativeFunctions::Handler>NativeFunctions::ByName;std::map<int,NativeFunctions::Handler>NativeFunctions::ByIndex;
'''+wrappers+r'''
std::string saved;int deleted=0;UObject*seen=nullptr;
void Load(ScriptArray&a,const std::string&file,std::string&result){a.Values={file};result.clear();}void Save(ScriptArray&a,const std::string&file,std::string&result){saved=file+":"+a.Values.at(0);result.clear();}void Delete(int slot){deleted=slot;}void Copy(UObject*self,UObject*outer,UObject*&result){seen=self;result=outer;}
int main(){RegisterVMNativeFunc_3("Object","LoadStringArray",&Load,2001);RegisterVMNativeFunc_3("Object","SaveStringArray",&Save,2002);RegisterVMNativeFunc_1("Object","KWDeleteSaveGame",&Delete,2004);RegisterVMNativeFunc_2("Object","Copy",&Copy,0);ScriptArray array;std::string filename="credits.txt",result="stale";ExpressionValue args[]={{ExpressionValueType::ValueArray,&array},{ExpressionValueType::ValueString,&filename},{ExpressionValueType::ValueString,&result}};NativeFunctions::ByIndex[2001](nullptr,args);assert(array.Values==std::vector<std::string>{filename}&&result.empty());NativeFunctions::ByIndex[2002](nullptr,args);assert(saved=="credits.txt:credits.txt");int slot=7;ExpressionValue deletion[]={{ExpressionValueType::ValueInt,&slot}};NativeFunctions::ByIndex[2004](nullptr,deletion);assert(deleted==7);UObject self,outer;UObject*outerPtr=&outer,*copy=nullptr;ExpressionValue objectArgs[]={{ExpressionValueType::ValueObject,&outerPtr},{ExpressionValueType::ValueObject,&copy}};NativeFunctions::ByName["Copy"](&self,objectArgs);assert(seen==&self&&copy==&outer);args[0].Type=ExpressionValueType::ValueString;bool rejected=false;try{NativeFunctions::ByIndex[2001](nullptr,args);}catch(...){rejected=true;}assert(rejected);}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-fsanitize=address,undefined',str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_commandlet_dispatch_and_abstract_rejection(self):
  body=definition((ROOT/'src/hp3/PoaServices.cpp').read_text(),'static void CommandletMain(')
  code=r'''#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <cstdint>
struct Exception{static void Throw(const char*s){throw std::runtime_error(s);}};
namespace FunctionFlags{constexpr int Native=1;}bool AllFlags(int a,int b){return (a&b)==b;}
struct UFunction{int FuncFlags=0;};struct UClass{UClass*BaseStruct=nullptr;UFunction*Main=nullptr;UFunction*GetFunction(const char*){return Main;}};struct UObject{UClass*Class=nullptr;void LoadNow(){}};
struct ExpressionValue{std::string Text;int Int=42;static ExpressionValue StringValue(const std::string&s){return {s};}int ToInt(){return Int;}};template<class T>using Array=std::vector<T>;template<class T>struct ArrayView{T*Data;size_t Size;ArrayView(T*p,size_t n):Data(p),Size(n){}};
struct Frame{static std::string Seen;static ExpressionValue Call(UFunction*,UObject*,const ArrayView<ExpressionValue>&args){assert(args.Size==1);Seen=args.Data[0].Text;return {};}};std::string Frame::Seen;
'''+body+r'''
int main(){UFunction native{1},script{0};UClass base{nullptr,&native},derived{&base,&script};UObject object{&derived};int result=0;CommandletMain(&object,"test arguments",result);assert(result==42&&Frame::Seen=="test arguments");derived.Main=nullptr;bool rejected=false;try{CommandletMain(&object,"",result);}catch(...){rejected=true;}assert(rejected);base.Main=&script;CommandletMain(&object,"inherited",result);assert(Frame::Seen=="inherited");}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
