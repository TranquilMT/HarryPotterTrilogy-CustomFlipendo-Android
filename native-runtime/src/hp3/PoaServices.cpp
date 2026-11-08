#include "Precomp.h"
#include "PoaServices.h"
#include "PoaTextArray.h"
#include "VM/NativeFunc.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/UFunction.h"
#include "Packages/Core/Properties/UProperty.h"
#include "Packages/Core/Properties/PropertyText.h"
#include "Package/PackageManager.h"
#include "Utils/File.h"
#include "Engine.h"
#include <typeinfo>
#include <limits>

namespace PoaServices
{
static fs::path TextPath(const std::string& filename)
{
    if(filename.empty()) Exception::Throw("Empty PoA string-array filename");
    std::string portable=filename;
    for(char& c:portable) if(c=='\\') c='/';
    fs::path path(portable);
    if(path.is_relative()) path=engine->packages->GetSystemFolderPath()/path;
    path=fs::weakly_canonical(File::find_ignoring_case(path));
    const auto root=fs::weakly_canonical(engine->packages->GetRootFolderPath());
    const auto relative=path.lexically_relative(root);
    if(relative.empty()||*relative.begin()=="..") Exception::Throw("PoA string-array path is outside the game folder");
    return path;
}
static UProperty* StringElement(ScriptArray& array)
{
    auto type=array.GetType();
    if(!type||type->ValueType!=ExpressionValueType::ValueString||type->ArrayDimension!=1||type->ElementSize()<sizeof(std::string))
        Exception::Throw("PoA string-array native received a non-string array");
    return type;
}
static void LoadStringArray(ScriptArray& array,const std::string& filename,std::string& result)
{
    result.clear();auto type=StringElement(array);const auto path=TextPath(filename);
    std::vector<std::string> lines;
    if(File::try_open_existing(path.string())) lines=PoaTextArray::Lines(PoaTextArray::Decode(File::read_all_text(path.string())));
    ScriptArray replacement(type);replacement.Resize(lines.size());
    for(size_t i=0;i<lines.size();++i) *static_cast<std::string*>(replacement.GetItem(i))=lines[i];
    array.Swap(replacement);
}
static void SaveStringArray(ScriptArray& array,const std::string& filename,std::string& result)
{
    result.clear();StringElement(array);const auto path=TextPath(filename);
    std::vector<std::string> lines;lines.reserve(array.GetSize());
    for(size_t i=0;i<array.GetSize();++i) lines.push_back(*static_cast<std::string*>(array.GetItem(i)));
    File::write_all_text(path.string(),PoaTextArray::Encode(lines));
}
static void KWDeleteSaveGame(int32_t slot)
{
    // Original EU Core formats exactly <SavePath>\\Save%i.usa and deletes only that file.
    auto path=engine->packages->GetSaveFolderPath()/("Save"+std::to_string(slot)+".usa");
    File::try_delete(File::find_ignoring_case(path).string());
}
static void Copy(UObject* self,UObject* outer,UObject*& result)
{
    result=nullptr;
    if(!self||!self->Class) Exception::Throw("PoA Copy requires an object instance");
    self->LoadNow();
    // The shipped call copies script-only ScriptedAction instances. Resource and
    // actor native state needs a class-specific copier, not a partial clone.
    if(typeid(*self)!=typeid(UObject)) Exception::Throw("PoA Copy of native C++ object state is unsupported");
    auto package=outer&&outer->package?outer->package:engine->packages->GetPackage("Engine");
    static uint64_t serial=0;NameString name;
    while(true)
    {
        if(serial==std::numeric_limits<uint64_t>::max()) Exception::Throw("PoA Copy exhausted object names");
        name=self->Class->Name.ToString()+std::to_string(serial++);bool used=false;
        for(GCObject* item:GC::GetObjects())
            if(auto existing=dynamic_cast<UObject*>(item)) if(existing->Name==name&&existing->Outer()==outer){used=true;break;}
        if(!used) break;
    }
    auto copy=package->NewObject(name,self->Class,ObjectFlags::Public,true);
    for(auto property:self->Class->Properties)
    {
        if(property->Name=="Class"||property->Name=="Name"||property->Name=="Outer"||property->Name=="ObjectFlags") continue;
        if(property->ArrayDimension<=0) Exception::Throw("Invalid PoA Copy array dimension");
        PropertyText::FieldOffset(property->DataOffset.DataOffset,property->ElementPitch(),size_t(property->ArrayDimension-1),self->PropertyData.Size,property->ElementSize());
        PropertyText::FieldOffset(property->DataOffset.DataOffset,property->ElementPitch(),size_t(property->ArrayDimension-1),copy->PropertyData.Size,property->ElementSize());
        property->CopyArray(copy->GetProperty(property),self->GetProperty(property));
    }
    copy->SetObject("Outer",outer);result=copy;
}
static void CommandletMain(UObject* self,const std::string& parameters,int32_t& result)
{
    if(!self||!self->Class) Exception::Throw("Commandlet.Main requires an instance");
    self->LoadNow();
    for(auto cls=self->Class;cls;cls=static_cast<UClass*>(cls->BaseStruct))
    {
        if(auto function=cls->GetFunction("Main"))
        {
            if(AllFlags(function->FuncFlags,FunctionFlags::Native))
                Exception::Throw("Commandlet.Main requires a script override or a native commandlet implementation");
            Array<ExpressionValue> args={ExpressionValue::StringValue(parameters)};
            result=Frame::Call(function,self,ArrayView<ExpressionValue>(args.data(),args.size())).ToInt();
            return;
        }
    }
    Exception::Throw("Commandlet.Main has no implementation");
}
void RegisterNatives()
{
    RegisterVMNativeFunc_2("Commandlet","Main",&CommandletMain,0);
    RegisterVMNativeFunc_3("Object","LoadStringArray",&LoadStringArray,2001);
    RegisterVMNativeFunc_3("Object","SaveStringArray",&SaveStringArray,2002);
    RegisterVMNativeFunc_1("Object","KWDeleteSaveGame",&KWDeleteSaveGame,2004);
    RegisterVMNativeFunc_2("Object","Copy",&Copy,0);
}
}
