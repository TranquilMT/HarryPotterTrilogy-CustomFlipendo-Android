"""Round-trip real property text methods with reflected-field test doubles."""
import pathlib,subprocess,tempfile,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
class PropertyTextTests(unittest.TestCase):
 def test_nested_partial_fixed_array_and_quoted_roundtrip(self):
  p=ENGINE/'Packages/Core/Properties'
  bodies='\n'.join(definition((p/f).read_text(),signature) for f,signature in [('UProperty.cpp','std::map<NameString, std::string> UProperty::ParsePropertiesFromString('),('UStructProperty.cpp','std::string UStructProperty::PrintValue('),('UStructProperty.cpp','void UStructProperty::SetValueFromString('),('UStrProperty.cpp','std::string UStrProperty::PrintValue('),('UStrProperty.cpp','void UStrProperty::SetValueFromString('),('UStringProperty.cpp','std::string UStringProperty::PrintValue('),('UStringProperty.cpp','void UStringProperty::SetValueFromString('),('UNameProperty.cpp','std::string UNameProperty::PrintValue('),('UNameProperty.cpp','void UNameProperty::SetValueFromString(')])
  code=r'''#include "Packages/Core/Properties/PropertyText.h"
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <limits>
#include <map>
#include <cctype>
struct NameString{std::string v;NameString()=default;NameString(const std::string&s):v(s){}NameString(const char*s):v(s){}const std::string&ToString()const{return v;}bool operator<(const NameString&b)const{auto a=v,c=b.v;for(char&x:a)x=std::tolower(static_cast<unsigned char>(x));for(char&x:c)x=std::tolower(static_cast<unsigned char>(x));return a<c;}};
struct UProperty{virtual~UProperty()=default;NameString Name;int ArrayDimension=1;size_t size=4,pitch=4;struct{size_t DataOffset=0;}DataOffset;size_t ElementPitch(){return pitch;}size_t ElementSize(){return size;}virtual void SetValueFromString(void*p,const std::string&s){*static_cast<float*>(p)=std::stof(s);}virtual std::string PrintValue(const void*p){return std::to_string(*static_cast<const float*>(p));}static std::map<NameString,std::string>ParsePropertiesFromString(std::string);};
struct UStruct{std::vector<UProperty*>Properties;size_t StructSize;};
struct UStructProperty:UProperty{UStruct*Struct;std::string PrintValue(const void*)override;void SetValueFromString(void*,const std::string&)override;};
struct UStrProperty:UProperty{std::string PrintValue(const void*)override;void SetValueFromString(void*,const std::string&)override;};
struct UStringProperty:UStrProperty{std::string PrintValue(const void*)override;void SetValueFromString(void*,const std::string&)override;};
struct UNameProperty:UProperty{std::string PrintValue(const void*)override;void SetValueFromString(void*,const std::string&)override;};
'''+bodies+r'''
struct Point{float X,Y;};struct Data{float Values[2];Point Origin;std::string Text;};
int main(){
 UStringProperty legacy;UNameProperty names;
 for(const std::string& value:{std::string(),std::string("hello, (world)"),std::string("say \"yes\""),std::string("C:\\Games\\HP3"),std::string("None"),std::string(" leading space ")}){
  std::string restored;legacy.SetValueFromString(&restored,legacy.PrintValue(&value));assert(restored==value);
  NameString name(value),back;names.SetValueFromString(&back,names.PrintValue(&name));assert(back.ToString()==value);
  auto list=PropertyText::List("("+names.PrintValue(&name)+",Other)");assert(list.size()==2);
 }
 NameString normal("Engine.Actor");assert(names.PrintValue(&normal)=="Engine.Actor");std::string raw;legacy.SetValueFromString(&raw,"Unquoted");assert(raw=="Unquoted");
 auto parsed=PropertyText::Parse(R"((Origin=(X=1,Y=2),Label="a,b) c",Ref=Texture'Pkg.A,B',Values(0)=3))");assert(parsed.size()==4&&parsed[0].second=="(X=1,Y=2)");
 for(auto invalid:{"(X=1", "(X=(Y=2)","(X=\"unterminated)","(=1)","(X=1,,Y=2)","(X=1))"}){bool rejected=false;try{PropertyText::Parse(invalid);}catch(const std::exception&){rejected=true;}assert(rejected);}
 assert(PropertyText::Parse("()").empty());assert(PropertyText::Parse(" None ").empty());assert(PropertyText::Parse("(X=1,)").size()==1);
 for(const std::string& s:{std::string(),std::string("text, (x)"),std::string("say \"yes\""),std::string("C:\\Games\\HP3"),std::string("a\\\"b")})assert(PropertyText::Unquote(PropertyText::Quote(s))==s);
 assert(PropertyText::Unquote(R"("C:\Games\HP3")")==R"(C:\Games\HP3)");
 UProperty values,x,y;values.Name="Values";values.ArrayDimension=2;values.DataOffset.DataOffset=offsetof(Data,Values);x.Name="X";x.DataOffset.DataOffset=offsetof(Point,X);y.Name="Y";y.DataOffset.DataOffset=offsetof(Point,Y);
 UStruct point{{&x,&y},sizeof(Point)};UStructProperty origin;origin.Name="Origin";origin.Struct=&point;origin.DataOffset.DataOffset=offsetof(Data,Origin);origin.size=origin.pitch=sizeof(Point);
 UStrProperty text;text.Name="Text";text.DataOffset.DataOffset=offsetof(Data,Text);text.size=text.pitch=sizeof(std::string);UStruct metadata{{&values,&origin,&text},sizeof(Data)};UStructProperty root;root.Struct=&metadata;
 Data d{{3,4},{5,6},"old"};root.SetValueFromString(&d,R"((Values[1]=9,Origin=(X=10),Text="a,b (quoted) \"yes\""))");assert(d.Values[0]==3&&d.Values[1]==9&&d.Origin.X==10&&d.Origin.Y==6&&d.Text=="a,b (quoted) \"yes\"");
 Data restored{{0,0},{0,0},""};const std::string serialized=root.PrintValue(&d);root.SetValueFromString(&restored,serialized);assert(restored.Values[0]==3&&restored.Values[1]==9&&restored.Origin.X==10&&restored.Origin.Y==6&&restored.Text==d.Text);
 root.SetValueFromString(&d,"(Values(0)=7,Origin=(X=1),oRiGiN=(X=2))");assert(d.Values[0]==7&&d.Origin.X==2&&d.Origin.Y==6);
 text.DataOffset.DataOffset=sizeof(Data)+1;bool rejected=false;try{root.SetValueFromString(&d,"(Text=bad)");}catch(const std::exception&){rejected=true;}assert(rejected);
 rejected=false;try{PropertyText::FieldOffset(0,8,std::numeric_limits<size_t>::max(),32,4);}catch(const std::exception&){rejected=true;}assert(rejected);
}
'''
  with tempfile.TemporaryDirectory() as tmp:
   t=pathlib.Path(tmp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_config_export_rejects_non_roundtrippable_members(self):
  body=definition((ENGINE/'Packages/Core/UClass.cpp').read_text(),'static bool CanSavePoaStructConfig(')
  code=r'''#include <vector>
#include <cassert>
struct UProperty{virtual~UProperty()=default;};
struct UByteProperty:UProperty{};struct UIntProperty:UProperty{};struct UFloatProperty:UProperty{};struct UNameProperty:UProperty{};struct UStrProperty:UProperty{};struct UStringProperty:UProperty{};struct UBoolProperty:UProperty{};struct UArrayProperty:UProperty{bool SupportsText()const{return false;}};
struct UStruct{std::vector<UProperty*>Properties;};struct UStructProperty:UProperty{UStruct*Struct=nullptr;};
struct UObject{template<class T>static T*TryCast(UProperty*p){return dynamic_cast<T*>(p);}template<class T>static bool IsType(UProperty*p){return dynamic_cast<T*>(p)!=nullptr;}};
'''+body+r'''
int main(){UFloatProperty value;UStrProperty text;UStruct inner{{&value,&text}};UStructProperty nested;nested.Struct=&inner;UStruct outer{{&nested}};UStructProperty root;root.Struct=&outer;assert(CanSavePoaStructConfig(&root));UArrayProperty unsupported;inner.Properties.push_back(&unsupported);assert(!CanSavePoaStructConfig(&root));inner.Properties={&root};assert(!CanSavePoaStructConfig(&root));}
'''
  with tempfile.TemporaryDirectory() as tmp:
   t=pathlib.Path(tmp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
