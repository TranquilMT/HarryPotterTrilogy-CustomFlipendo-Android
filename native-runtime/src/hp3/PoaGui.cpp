#include "Precomp.h"
#include "PoaGui.h"
#include "PoaGuiText.h"
#include "PoaGuiDraw.h"
#include "PoaGuiLayout.h"
#include "PoaServices.h"
#include "Engine.h"
#include "Packages/Core/UObject.h"
#include "Packages/Engine/UCanvas.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Subsystems/UGameEngine.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "Package/PackageManager.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UArrayProperty.h"
#include "Packages/Core/Properties/UObjectProperty.h"
#include "Packages/Core/Properties/UStructProperty.h"
#include "Utils/StrTools.h"
#include "Native/NCanvas.h"
#include "Native/NObject.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
namespace PoaGui
{
static std::unique_ptr<GCRoot<UObject>> controller;
static bool initializing=false;
UObject* Controller(){return controller?controller->get():nullptr;}
void Shutdown()
{
    if(auto current=Controller()){
        if(engine && engine->viewport && engine->viewport->HasProperty("GUIController") && engine->viewport->GetUObject("GUIController")==current)
            engine->viewport->SetObject("GUIController",nullptr);
        if(current->HasProperty("ViewportOwner"))current->SetObject("ViewportOwner",nullptr);
    }
    controller.reset();initializing=false;
}
void Initialize()
{
    if(Controller() || initializing || !engine || !engine->LaunchInfo.IsHarryPotter3() || !engine->viewport || !engine->canvas)return;
    const auto name=engine->packages->GetIniValue("System","Engine.Engine","GUIController");
    const auto dot=name.find('.');
    if(dot==std::string::npos || dot==0 || dot+1==name.size())Exception::Throw("Invalid PoA GUIController class: "+name);
    auto cls=engine->packages->GetPackage(name.substr(0,dot))->GetClass(name.substr(dot+1));
    if(!cls)Exception::Throw("Could not resolve PoA GUIController: "+name);
    auto root=std::make_unique<GCRoot<UObject>>(engine->packages->GetTransientPackage()->NewObject("poa_gui_controller",cls,ObjectFlags::Transient));
    if(!root->get()->IsA("BaseGUIController"))Exception::Throw("Configured PoA GUIController does not derive from BaseGUIController");
    controller=std::move(root);initializing=true;
    try {
        Controller()->SetObject("ViewportOwner",engine->viewport);
        engine->viewport->SetObject("GUIController",Controller());
        CallEvent(Controller(),"InitializeController");initializing=false;
    }
    catch(...){Shutdown();throw;}
}
static UObject* Lookup(UObject* self,const char* stack,const std::string& key)
{
    if(!self || !self->HasProperty(stack))return nullptr;
    auto property=UObject::TryCast<UArrayProperty>(self->GetMemberProperty(stack));
    if(!property || !property->Inner || property->Inner->ValueType!=ExpressionValueType::ValueObject || property->Inner->ElementSize()!=sizeof(UObject*))
        Exception::Throw(std::string("Invalid PoA GUI object stack: ")+stack);
    auto& items=*static_cast<ScriptArray*>(self->GetProperty(property));
    for(size_t i=0;i<items.GetSize();++i){
        auto item=*static_cast<UObject**>(items.GetItem(i));
        if(item && item->HasProperty("KeyName") && StrTools::equals_ignore_case(item->GetString("KeyName"),key))return item;
    }
    return nullptr;
}
static void GetStyle(UObject* self,const std::string& key,UObject*& result){result=Lookup(self,"StyleStack",key);}
static void GetMenuFont(UObject* self,const std::string& key,UObject*& result){result=Lookup(self,"FontStack",key);}
static void GetMainMenuClass(UObject*,std::string& result)
{
    result=engine && engine->gameengine && engine->gameengine->HasProperty("MainMenuClass")?engine->gameengine->GetString("MainMenuClass"):std::string();
}
static void GetCurrentRes(UObject*,std::string& result)
{
    result.clear();if(!engine || !engine->viewport)return;
    result=std::to_string(engine->viewport->ViewportWidth())+"x"+std::to_string(engine->viewport->ViewportHeight());
}
static float Coordinate(UObject* self,int axis)
{
    if(!engine || !engine->viewport || !engine->canvas)return 0;
    // Android uses one live viewport; Canvas Size is its full render size, not
    // the temporarily clipped drawing region used by an individual control.
    return PoaGuiLayout::Resolve(self,axis,float(engine->canvas->SizeX()),float(engine->canvas->SizeY()));
}
static void ActualWidth(UObject* self,float& result){result=Coordinate(self,0);}
static void ActualHeight(UObject* self,float& result){result=Coordinate(self,1);}
static void ActualLeft(UObject* self,float& result){result=Coordinate(self,2);}
static void ActualTop(UObject* self,float& result){result=Coordinate(self,3);}
static bool InlineComponent(UProperty* field)
{
    auto object=UObject::TryCast<UObjectProperty>(field);
    if(!object || !(uint32_t(object->PropFlags)&0x80000000u))return false;
    for(auto cls=static_cast<UStruct*>(object->ObjectClass);cls;cls=cls->BaseStruct)
        if(cls->Name=="GUIComponent")return true;
    return false;
}
using GuiCopies=std::unordered_map<UObject*,UObject*>;
static UObject* CloneControl(UObject* source,UObject* owner,GuiCopies& copies,std::vector<std::unique_ptr<GCRoot<UObject>>>& roots)
{
    if(!source)return nullptr;
    if(auto found=copies.find(source);found!=copies.end())return found->second;
    if(source->Outer()==owner)return source;
    auto copy=PoaServices::CopyScriptObject(source,owner);
    roots.push_back(std::make_unique<GCRoot<UObject>>(copy));copies[source]=copy;
    for(auto property:copy->Class->Properties)if(InlineComponent(property))
        for(int i=0;i<property->ArrayDimension;++i){
            auto slot=static_cast<UObject**>(property->GetElement(copy->GetProperty(property),i));
            *slot=CloneControl(*slot,copy,copies,roots);
        }
    return copy;
}
static void RemapGuiProperty(UProperty* property,void* data,const GuiCopies& copies)
{
    for(int i=0;i<property->ArrayDimension;++i){
        auto value=property->GetElement(data,i);
        auto remap=[&](UObject*& object){if(auto found=copies.find(object);found!=copies.end())object=found->second;};
        if(UObject::TryCast<UObjectProperty>(property))remap(*static_cast<UObject**>(value));
        else if(UObject::TryCast<UPoaDelegateProperty>(property))remap(static_cast<PoaDelegateBinding*>(value)->Object);
        else if(auto array=UObject::TryCast<UArrayProperty>(property)){
            auto items=static_cast<ScriptArray*>(value);
            for(size_t n=0;n<items->GetSize();++n)RemapGuiProperty(array->Inner,items->GetItem(n),copies);
        }else if(auto structure=UObject::TryCast<UStructProperty>(property)){
            for(auto member:structure->Struct->Properties)RemapGuiProperty(member,static_cast<char*>(value)+member->DataOffset.DataOffset,copies);
        }
    }
}
static void InstantiateControls(UObject* self)
{
    GCRoot<UObject> selfRoot(self);GuiCopies copies;
    std::vector<std::unique_ptr<GCRoot<UObject>>> roots;
    std::vector<std::pair<UObject**,UObject*>> staged;
    // Keep authored defaults intact. NewObject copies object references shallowly.
    for(auto property:self->Class->Properties)if(InlineComponent(property))
        for(int i=0;i<property->ArrayDimension;++i){
            auto slot=static_cast<UObject**>(property->GetElement(self->GetProperty(property),i));
            staged.emplace_back(slot,CloneControl(*slot,self,copies,roots));
        }
    if(copies.empty())return;
    for(auto [source,copy]:copies){(void)source;for(auto property:copy->Class->Properties)RemapGuiProperty(property,copy->GetProperty(property),copies);}
    for(auto [slot,value]:staged)*slot=value;
    for(auto property:self->Class->Properties)RemapGuiProperty(property,self->GetProperty(property),copies);
}
static void InitializeControls(UObject* self)
{
    if(!self || !self->Class || !self->HasProperty("Controls"))return;
    auto arrayProperty=UObject::TryCast<UArrayProperty>(self->GetMemberProperty("Controls"));
    if(!arrayProperty || !arrayProperty->Inner || arrayProperty->Inner->ValueType!=ExpressionValueType::ValueObject)
        Exception::Throw("Invalid PoA GUI Controls array");
    InstantiateControls(self);
    auto& controls=*static_cast<ScriptArray*>(self->GetProperty(arrayProperty));
    if(controls.GetSize())return; // Preserve explicitly supplied control lists.
    GCRoot<UObject> selfRoot(self);
    std::vector<UObject*> found;
    for(auto type=static_cast<UStruct*>(self->Class);type;type=type->BaseStruct)
        for(auto field=type->Children;field;field=field->Next)
        {
            auto property=UObject::TryCast<UObjectProperty>(field);
            if(!property || !(uint32_t(property->PropFlags)&0x80000000u))continue;
            bool component=false;
            for(auto cls=static_cast<UStruct*>(property->ObjectClass);cls;cls=cls->BaseStruct)
                if(cls->Name=="GUIComponent"){component=true;break;}
            if(!component)continue;
            auto object=*static_cast<UObject**>(self->GetProperty(property));
            if(!object)continue;
            const float weight=object->GetFloat("RenderWeight");
            auto at=found.begin();
            while(at!=found.end() && !(weight<=(*at)->GetFloat("RenderWeight")))++at;
            found.insert(at,object);
        }
    controls.Resize(found.size());
    for(size_t i=0;i<found.size();++i)*static_cast<UObject**>(controls.GetItem(i))=found[i];
}
static ScriptArray& GuiArray(UObject* self,const char* name,ExpressionValueType type)
{
    auto property=UObject::TryCast<UArrayProperty>(self->GetMemberProperty(name));
    if(!property || !property->Inner || property->Inner->ValueType!=type)
        Exception::Throw(std::string("Invalid PoA GUI array: ")+name);
    return *static_cast<ScriptArray*>(self->GetProperty(property));
}
static void GetFont(UObject* self,UObject*,int32_t resolution,UObject*& result)
{
    result=nullptr;if(!self)return;
    auto& names=GuiArray(self,"FontArrayNames",ExpressionValueType::ValueString);
    auto& sizes=GuiArray(self,"FontArrayResolutions",ExpressionValueType::ValueInt);
    auto& fonts=GuiArray(self,"FontArrayFonts",ExpressionValueType::ValueObject);
    if(!names.GetSize())return;
    size_t selected=0;
    if(!self->GetBool("bFixedSize"))
        while(selected<sizes.GetSize() && resolution>=*static_cast<int32_t*>(sizes.GetItem(selected)))++selected;
    selected=std::min(selected,names.GetSize()-1);
    if(fonts.GetSize()<=selected)fonts.Resize(selected+1);
    auto& cached=*static_cast<UObject**>(fonts.GetItem(selected));
    if(!cached){
        const auto& name=*static_cast<std::string*>(names.GetItem(selected));
        if(!name.empty()){
            auto fontClass=engine->packages->GetPackage("Engine")->GetClass("Font");
            UObject* loaded=nullptr;NObject::DynamicLoadObject(name,fontClass,true,loaded);
            cached=UObject::TryCast<UFont>(loaded);
        }
    }
    result=cached;
}
static void DrawTileScaled(UObject* self,UObject* material,float xScale,float yScale)
{
    auto canvas=UObject::Cast<UCanvas>(self);
    if(!material || canvas->Style()==0)return;
    auto texture=UObject::TryCast<UTexture>(material);
    if(!texture)Exception::Throw("PoA Canvas.DrawTileScaled requires texture material support");
    // Unlike DrawTile, the retail scaled helper leaves cursor and line state unchanged.
    const float x=canvas->CurX(),y=canvas->CurY(),line=canvas->CurYL();
    try {NCanvas::DrawTile(canvas,texture,float(texture->USize())*xScale,float(texture->VSize())*yScale,0,0,float(texture->USize()),float(texture->VSize()));}
    catch(...){canvas->CurX()=x;canvas->CurY()=y;canvas->CurYL()=line;throw;}
    canvas->CurX()=x;canvas->CurY()=y;canvas->CurYL()=line;
}
void RegisterNatives()
{
    PoaGuiText::RegisterNatives();
    PoaGuiDraw::RegisterNatives();
    RegisterVMNativeFunc_3("GUIFont","GetFont",&GetFont,0);
    RegisterVMNativeFunc_1("GUIController","GetMainMenuClass",&GetMainMenuClass,0);
    RegisterVMNativeFunc_1("GUIController","GetCurrentRes",&GetCurrentRes,0);
    RegisterVMNativeFunc_0("GUIMultiComponent","InitializeControls",&InitializeControls,0);
    RegisterVMNativeFunc_3("Canvas","DrawTileScaled",&DrawTileScaled,0);
    RegisterVMNativeFunc_2("GUIController","GetStyle",&GetStyle,0);
    RegisterVMNativeFunc_2("GUIController","GetMenuFont",&GetMenuFont,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualWidth",&ActualWidth,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualHeight",&ActualHeight,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualLeft",&ActualLeft,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualTop",&ActualTop,0);
}
}
