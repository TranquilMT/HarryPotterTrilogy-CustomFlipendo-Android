#include "Precomp.h"
#include "PoaPlayer.h"
#include "PoaGui.h"
#include "Packages/Core/Properties/UStrProperty.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/UViewport.h"
#include "VM/ScriptCall.h"
namespace PoaPlayer
{
UActor* ViewTarget(UActor* actor)
{
    if(actor && actor->HasProperty("ViewTarget"))
        if(auto target=UObject::TryCast<UActor>(actor->GetUObject("ViewTarget")))return target;
    return actor;
}
void SetButton(UObject* actor,const std::string& name,bool down)
{
    if(!actor)return;
    auto property=actor->GetMemberProperty(name);
    if(!property)return;
    if(property->ValueType==ExpressionValueType::ValueByte)actor->SetByte(name,down?1:0);
    else if(property->ValueType==ExpressionValueType::ValueBool)actor->SetBool(name,down);
    else Exception::Throw("PoA input button has unsupported property type: "+name);
}
void Login(UObject* game,UViewport* viewport,const std::string& portal,const std::string& options)
{
    if(!game||!viewport)Exception::Throw("Missing PoA player login context");
    auto stringProp=GC::Alloc<UStrProperty>("",nullptr,ObjectFlags::NoFlags);
    GCRoot<UObject> propertyRoot(stringProp);
    std::string error,failcode;
    // Original EU GameInfo.PreLogin: Options, Address, out Error, out FailCode.
    CallEvent(game,"PreLogin",{ExpressionValue::StringValue(options),ExpressionValue::StringValue(""),
        ExpressionValue::Variable(&error,stringProp),ExpressionValue::Variable(&failcode,stringProp)});
    if(!error.empty()||!failcode.empty())Exception::Throw("GameInfo prelogin failed: "+error+" ("+failcode+")");
    // KWGame.Login chooses its controller class and possesses its pawn itself.
    auto result=CallEvent(game,"Login",{ExpressionValue::StringValue(portal),ExpressionValue::StringValue(options),ExpressionValue::Variable(&error,stringProp)}).ToObject();
    auto controller=UObject::TryCast<UActor>(result);
    if(!controller||!controller->IsA("PlayerController")||!error.empty())Exception::Throw("PoA GameInfo login failed: "+error);
    GCRoot<UObject> controllerRoot(controller);
    auto previous=viewport->ViewActor();
    viewport->ViewActor()=controller;controller->SetObject("Player",viewport);
    try {
        PoaGui::Initialize();
        CallEvent(controller,"InitInputSystem");
        CallEvent(game,"PostLogin",{ExpressionValue::ObjectValue(controller)});
    }catch(...){
        if(viewport->ViewActor()==controller)viewport->ViewActor()=previous;
        if(controller->GetUObject("Player")==viewport)controller->SetObject("Player",nullptr);
        throw;
    }
}
}
