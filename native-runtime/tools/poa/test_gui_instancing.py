"""Exercise production GUI graph instancing and reference remapping with typed host storage."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiInstancing(unittest.TestCase):
 def test_owned_controls_aliases_delegates_and_failure_atomicity(self):
  source=(ROOT/'src/hp3/PoaGui.cpp').read_text()
  body=definition(source,'static bool InlineComponent(')+'\nusing GuiCopies=std::unordered_map<UObject*,UObject*>;\n'+'\n'.join(definition(source,n)for n in ['static UObject* CloneControl(','static void RemapGuiProperty(','static void InstantiateControls('])
  code=r'''#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstddef>
#include <stdexcept>
#include <cassert>
struct UObject;struct UClass;
struct UProperty{virtual~UProperty()=default;unsigned PropFlags=0;int ArrayDimension=1,Id=0;size_t Pitch=sizeof(UObject*);struct{size_t DataOffset=0;}DataOffset;void*GetElement(void*p,size_t n){return static_cast<char*>(p)+n*Pitch;}};
struct UObjectProperty:UProperty{UClass*ObjectClass=nullptr;};struct UPoaDelegateProperty:UProperty{};
struct UArrayProperty:UProperty{UProperty*Inner=nullptr;};struct UStruct{std::string Name;UStruct*BaseStruct=nullptr;std::vector<UProperty*>Properties;};struct UClass:UStruct{};struct UStructProperty:UProperty{UStruct*Struct=nullptr;};
struct PoaDelegateBinding{UObject*Object=nullptr;int Function=0;};struct Bundle{UObject*Pointer=nullptr;PoaDelegateBinding Callback;};
struct ScriptArray{std::vector<UObject*>Items;size_t GetSize(){return Items.size();}void*GetItem(size_t i){return &Items.at(i);}};
struct UObject{UClass*Class=nullptr;UObject*Owner=nullptr;UObject*Children[2]={};UObject*Alias=nullptr,*Link=nullptr;PoaDelegateBinding Callback;ScriptArray Array;Bundle Nested;int Focus=0;
 UObject*Outer(){return Owner;}template<class T>static T*TryCast(UProperty*p){return dynamic_cast<T*>(p);}void*GetProperty(UProperty*p){switch(p->Id){case 1:return Children;case 2:return &Alias;case 3:return &Link;case 4:return &Callback;case 5:return &Array;case 6:return &Nested;default:throw std::runtime_error("bad property");}}
};
int Roots=0,Copies=0,FailAt=0;template<class T>struct GCRoot{GCRoot(T*){++Roots;}~GCRoot(){--Roots;}};std::vector<std::unique_ptr<UObject>>Storage;
namespace PoaServices{UObject*CopyScriptObject(UObject*source,UObject*owner){if(++Copies==FailAt)throw std::runtime_error("copy failed");auto c=std::make_unique<UObject>(*source);c->Owner=owner;auto p=c.get();Storage.push_back(std::move(c));return p;}}
'''+body+r'''
int main(){UClass component;component.Name="GUIComponent";UObjectProperty child,alias,link,element,nestedPointer;UPoaDelegateProperty callback,nestedCallback;UArrayProperty array;UStructProperty nested;UStruct bundle;
child.Id=1;child.ArrayDimension=2;child.PropFlags=0x80000000u;child.ObjectClass=&component;alias.Id=2;alias.PropFlags=0x80000000u;alias.ObjectClass=&component;link.Id=3;callback.Id=4;callback.Pitch=sizeof(PoaDelegateBinding);array.Id=5;array.Inner=&element;array.Pitch=sizeof(ScriptArray);nested.Id=6;nested.Struct=&bundle;nested.Pitch=sizeof(Bundle);nestedPointer.DataOffset.DataOffset=offsetof(Bundle,Pointer);nestedCallback.Pitch=sizeof(PoaDelegateBinding);nestedCallback.DataOffset.DataOffset=offsetof(Bundle,Callback);bundle.Properties={&nestedPointer,&nestedCallback};
component.Properties={&child,&alias,&link,&callback,&array,&nested};
UObject authored,a,b,c,page,other;a.Class=b.Class=c.Class=page.Class=other.Class=&component;a.Owner=b.Owner=&authored;c.Owner=&a;a.Children[0]=&c;a.Link=&b;a.Callback={&b,42};page.Children[0]=&a;page.Children[1]=&b;page.Alias=&a;page.Array.Items={&b,&a};page.Callback={&a,7};page.Nested.Pointer=&b;page.Nested.Callback={&a,99};other=page;
InstantiateControls(&page);assert(Roots==0);auto ac=page.Children[0],bc=page.Children[1];assert(ac!=&a&&bc!=&b&&ac->Outer()==&page&&bc->Outer()==&page);assert(page.Alias==ac&&ac->Link==bc&&ac->Callback.Object==bc&&ac->Callback.Function==42);assert(ac->Children[0]!=&c&&ac->Children[0]->Outer()==ac);assert(page.Array.Items[0]==bc&&page.Array.Items[1]==ac&&page.Callback.Object==ac&&page.Nested.Pointer==bc&&page.Nested.Callback.Object==ac&&page.Nested.Callback.Function==99);
assert(a.Link==&b&&a.Children[0]==&c&&a.Owner==&authored);ac->Focus=1;InstantiateControls(&other);assert(other.Children[0]!=ac&&other.Children[0]->Focus==0&&a.Focus==0);auto before=Copies;InstantiateControls(&page);assert(Copies==before&&page.Children[0]==ac);
UObject failure;failure.Class=&component;failure.Children[0]=&a;failure.Children[1]=&b;FailAt=Copies+2;bool threw=false;try{InstantiateControls(&failure);}catch(...){threw=true;}assert(threw&&Roots==0&&failure.Children[0]==&a&&failure.Children[1]==&b);
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
