#include "Precomp.h"
#include "PoaNotifies.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Core/UFunction.h"
#include "Native/NActor.h"
#include "VM/ScriptCall.h"
#include "VM/ExpressionValue.h"
namespace PoaNotifies
{
bool Dispatch(UObject* notify,UActor* owner)
{
    if(!notify||!owner||owner->bDeleteMe())return true;
    GCRoot<UObject> notifyRoot(notify);GCRoot<UActor> ownerRoot(owner);notify->LoadNow();
    if(FindEventFunction(notify,"Notify"))
    {
        CallEvent(notify,NameString("Notify"),{ExpressionValue::ObjectValue(owner)});return true;
    }
    if(notify->IsA("AnimNotify_Script"))
    {
        const auto name=notify->GetName("NotifyName");
        if(name.IsNone())return true;
        auto function=FindEventFunction(owner,name);if(!function)return true;
        if((uint32_t(function->FuncFlags)&0x100000u)!=0)Exception::Throw("PoA delegated script notify requires delegate dispatch");
        if(function->NumParms!=0)Exception::Throw("PoA named script notify requires a zero-argument function");
        CallEvent(owner,name);return true;
    }
    if(notify->IsA("AnimNotify_Sound"))
    {
        auto sound=notify->GetUObject("Sound");if(!sound)return true;sound->LoadNow();
        const float volume=notify->GetFloat("Volume");const int32_t radius=int32_t(notify->GetInt("Radius"));
        if(!std::isfinite(volume))Exception::Throw("Non-finite PoA sound notify volume");
        // Retail Notify uses slot None, pitch 1, its stored Volume/Radius,
        // and Core.GAudioDefaultRadius (initialized to 80) when Radius is zero.
        NActor::PlaySound(owner,sound,uint8_t(SLOT_None),volume,false,radius?float(radius):80.0f,1.0f);return true;
    }
    return false;
}
}
