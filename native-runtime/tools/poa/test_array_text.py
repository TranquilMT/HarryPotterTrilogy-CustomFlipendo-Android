"""Production array method bodies, with bounded stream/reflection/storage doubles."""
import pathlib,subprocess,tempfile,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
class ArrayTests(unittest.TestCase):
 def test_text_roundtrip_and_transactional_binary_replacement(self):
  source=(ENGINE/'Packages/Core/Properties/UArrayProperty.cpp').read_text()
  signatures=['static void ReadArray(','void UArrayProperty::LoadValue(','void UArrayProperty::LoadStructMemberValue(','static bool SupportsArrayText(','bool UArrayProperty::SupportsText(','std::string UArrayProperty::PrintValue(','void UArrayProperty::SetValueFromString(']
  bodies='\n'.join(definition(source,s) for s in signatures)
  code=r'''#include "Packages/Core/Properties/PropertyText.h"
#include <cassert>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <cstring>
struct Exception{static void Throw(const char*s){throw std::runtime_error(s);}};
enum class ExpressionValueType{ValueArray,ValueByte,ValueInt,ValueBool,ValueFloat,ValueName,ValueString,ValueObject};
struct ObjectStream{std::vector<int> words;size_t pos=0;uint32_t Tell(){return pos;}size_t Remaining(){return words.size()-pos;}int ReadIndex(){if(!Remaining())throw std::runtime_error("truncated");return words[pos++];}};
struct PropertyHeader{int size,arrayIndex,type=1;};constexpr int UPT_Array=1;
struct UProperty{virtual~UProperty()=default;ExpressionValueType ValueType=ExpressionValueType::ValueString;int ArrayDimension=1;virtual size_t ElementPitch(){return sizeof(std::string);}virtual void Construct(void*p){new(p)std::string;}virtual void Destroy(void*p){static_cast<std::string*>(p)->~basic_string();}virtual std::string PrintValue(const void*p){return PropertyText::Quote(*static_cast<const std::string*>(p));}virtual void SetValueFromString(void*p,const std::string&s){if(s=="bad")throw std::runtime_error("bad element");*static_cast<std::string*>(p)=PropertyText::Unquote(s);}virtual void LoadStructMemberValue(void*p,ObjectStream*s){*static_cast<std::string*>(p)=std::to_string(s->ReadIndex());}};
struct ScriptArray{UProperty*type;void*data=nullptr;size_t count=0;explicit ScriptArray(UProperty*t):type(t){}~ScriptArray(){for(size_t i=0;i<count;++i)type->Destroy(GetItem(i));::operator delete(data);}void Resize(size_t n){assert(!data);if(n)data=::operator new(n*type->ElementPitch());for(;count<n;++count)type->Construct(GetItem(count));}void*GetItem(size_t i){return static_cast<uint8_t*>(data)+i*type->ElementPitch();}const void*GetData()const{return data;}size_t GetSize()const{return count;}void Swap(ScriptArray&b){assert(type==b.type);std::swap(data,b.data);std::swap(count,b.count);}};
struct UStruct{std::vector<UProperty*>Properties;};struct UStructProperty:UProperty{UStruct*Struct=nullptr;};
struct UArrayProperty:UProperty{UProperty*Inner=nullptr;bool SupportsText()const;std::string PrintValue(const void*)override;void SetValueFromString(void*,const std::string&)override;void LoadValue(void*,ObjectStream*,const PropertyHeader&);void LoadStructMemberValue(void*,ObjectStream*)override;void ThrowIfTypeMismatch(const PropertyHeader&h,int t){if(h.type!=t)throw std::runtime_error("type");}size_t ElementPitch()override{return sizeof(ScriptArray);}void Construct(void*p)override{new(p)ScriptArray(Inner);}void Destroy(void*p)override{static_cast<ScriptArray*>(p)->~ScriptArray();}};
struct UObject{template<class T>static T*TryCast(UProperty*p){return dynamic_cast<T*>(p);}};
'''+bodies+r'''
int main(){UProperty text;UArrayProperty a;a.Inner=&text;ScriptArray values(&text);
 a.SetValueFromString(&values,R"(("a,b","say \"yes\"","C:\\Games"))");auto before=a.PrintValue(&values);assert(before==R"(("a,b","say \"yes\"","C:\\Games"))");
 for(auto bad:{"(ok,bad)","(ok,,bad)","(\"unterminated)"}){bool caught=false;try{a.SetValueFromString(&values,bad);}catch(...){caught=true;}assert(caught&&a.PrintValue(&values)==before);}
 UArrayProperty nested;nested.Inner=&a;ScriptArray outer(&a);nested.SetValueFromString(&outer,R"((("x,y"),("z")))");assert(nested.PrintValue(&outer)==R"((("x,y"),("z")))");nested.SetValueFromString(&outer,"()");assert(outer.GetSize()==0);
 UProperty object;object.ValueType=ExpressionValueType::ValueObject;UArrayProperty unsupported;unsupported.Inner=&object;assert(!unsupported.SupportsText());UArrayProperty cycle;cycle.Inner=&cycle;assert(!cycle.SupportsText());
 for(auto words:std::vector<std::vector<int>>{{-1},{99,1},{2,7},{1,8,9},{0,9}}){ObjectStream s{words};bool caught=false;try{a.LoadValue(&values,&s,{int(words.size()),0});}catch(...){caught=true;}assert(caught&&a.PrintValue(&values)==before);}
 ObjectStream success{{2,7,8}};a.LoadValue(&values,&success,{3,0});assert(a.PrintValue(&values)==R"(("7","8"))");
 ObjectStream invalid{{0}};bool caught=false;try{a.LoadValue(&values,&invalid,{1,1});}catch(...){caught=true;}assert(caught&&invalid.pos==0);
 ObjectStream empty{{0}};a.LoadValue(&values,&empty,{1,0});assert(values.GetSize()==0);
 ObjectStream untagged{{2,9,10}};a.LoadStructMemberValue(&values,&untagged);assert(a.PrintValue(&values)==R"(("9","10"))");
}
'''
  with tempfile.TemporaryDirectory() as tmp:
   t=pathlib.Path(tmp);(t/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True)
   subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
