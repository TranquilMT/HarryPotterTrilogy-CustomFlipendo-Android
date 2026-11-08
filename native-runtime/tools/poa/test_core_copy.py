"""Production Copy adapter/property-array loop with reflected storage doubles."""
import pathlib,tempfile,subprocess,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class CopyNative(unittest.TestCase):
 def test_script_action_copy_identity_and_owned_values(self):
  adapter=definition((ROOT/'src/hp3/PoaServices.cpp').read_text(),'static void Copy(')
  copy=definition((ENGINE/'Packages/Core/Properties/UProperty.cpp').read_text(),'void UProperty::CopyArray(')
  code=r'''#include "Packages/Core/Properties/PropertyText.h"
#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include <limits>
#include <cassert>
#include <cstddef>
struct Exception{static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct NameString{std::string Text;NameString()=default;NameString(const std::string&s):Text(s){}NameString(const char*s):Text(s){}const std::string&ToString()const{return Text;}bool operator==(const NameString&n)const{return Text==n.Text;}};
struct GCObject{virtual~GCObject()=default;};struct UObject;struct UClass;struct Package;
struct Payload{int Count=0;std::string Labels[2];std::vector<std::string>Lines;UObject*Target=nullptr;};
struct UProperty{NameString Name;int ArrayDimension=1;struct{size_t DataOffset=0;}DataOffset;enum Kind{Identity,Integer,String,List,Reference}Type;size_t Size;size_t ElementSize(){return Size;}size_t ElementPitch(){return Size;}void*GetElement(void*p,int i){return static_cast<char*>(p)+i*Size;}const void*GetElement(const void*p,int i){return static_cast<const char*>(p)+i*Size;}void CopyArray(void*,const void*);void CopyElement(void*d,const void*s){switch(Type){case Identity:throw std::runtime_error("identity copied");case Integer:*static_cast<int*>(d)=*static_cast<const int*>(s);break;case String:*static_cast<std::string*>(d)=*static_cast<const std::string*>(s);break;case List:*static_cast<std::vector<std::string>*>(d)=*static_cast<const std::vector<std::string>*>(s);break;case Reference:*static_cast<UObject**>(d)=*static_cast<UObject*const*>(s);break;}}};
struct UClass{NameString Name="Action";std::vector<UProperty*>Properties;};
enum class ObjectFlags{Public=4};
struct UObject:GCObject{UClass*Class=nullptr;Package*package=nullptr;NameString Name;ObjectFlags Flags=ObjectFlags::Public;Payload Data;struct{size_t Size=sizeof(Payload);}PropertyData;UObject*Parent=nullptr;void LoadNow(){}UObject*&Outer(){return Parent;}void SetObject(const char*,UObject*p){Parent=p;}void*GetProperty(UProperty*p){return reinterpret_cast<char*>(&Data)+p->DataOffset.DataOffset;}};
struct NativeObject:UObject{};
struct GC{static std::vector<GCObject*>Objects;static auto&GetObjects(){return Objects;}};std::vector<GCObject*>GC::Objects;
struct Package{std::vector<std::unique_ptr<UObject>>Owned;UObject*NewObject(const NameString&name,UClass*cls,ObjectFlags flags,bool){auto object=std::make_unique<UObject>();object->Name=name;object->Class=cls;object->Flags=flags;object->package=this;auto result=object.get();Owned.push_back(std::move(object));GC::Objects.push_back(result);return result;}};
struct Packages{Package Default;Package*GetPackage(const char*){return &Default;}}packages;struct Engine{Packages*packages;}instance{&packages};Engine*engine=&instance;
'''+copy+adapter+r'''
int main(){UProperty identity;identity.Name="Name";identity.Type=UProperty::Identity;identity.Size=sizeof(int);UProperty count;count.Name="Count";count.Type=UProperty::Integer;count.Size=sizeof(int);count.DataOffset.DataOffset=offsetof(Payload,Count);UProperty labels;labels.Name="Labels";labels.Type=UProperty::String;labels.Size=sizeof(std::string);labels.ArrayDimension=2;labels.DataOffset.DataOffset=offsetof(Payload,Labels);UProperty lines;lines.Name="Lines";lines.Type=UProperty::List;lines.Size=sizeof(std::vector<std::string>);lines.DataOffset.DataOffset=offsetof(Payload,Lines);UProperty reference;reference.Name="Target";reference.Type=UProperty::Reference;reference.Size=sizeof(UObject*);reference.DataOffset.DataOffset=offsetof(Payload,Target);
 UClass cls;cls.Properties={&identity,&count,&labels,&lines,&reference};Package package;UObject outer;outer.package=&package;UObject self;self.Class=&cls;self.Name="Original";self.Data.Count=7;self.Data.Labels[0]="Harry";self.Data.Labels[1]="Ron";self.Data.Lines={"one","two"};self.Data.Target=&outer;UObject*result=nullptr;Copy(&self,&outer,result);assert(result&&result!=&self&&result->Class==self.Class&&result->Parent==&outer&&result->Name!=self.Name);assert(result->Data.Count==7&&result->Data.Labels[1]=="Ron"&&result->Data.Lines==self.Data.Lines&&result->Data.Target==&outer);result->Data.Labels[0]="Hermione";result->Data.Lines[0]="changed";assert(self.Data.Labels[0]=="Harry"&&self.Data.Lines[0]=="one");
 auto first=result;Copy(&self,&outer,result);assert(result->Name!=first->Name);NativeObject native;native.Class=&cls;bool rejected=false;try{Copy(&native,&outer,result);}catch(...){rejected=true;}assert(rejected&&result==nullptr);
 labels.DataOffset.DataOffset=sizeof(Payload);rejected=false;try{Copy(&self,&outer,result);}catch(...){rejected=true;}assert(rejected&&result==nullptr);
}
'''
  code=code.replace('bool operator==(const NameString&n)const{return Text==n.Text;}','bool operator==(const NameString&n)const{return Text==n.Text;}bool operator!=(const NameString&n)const{return Text!=n.Text;}')
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
