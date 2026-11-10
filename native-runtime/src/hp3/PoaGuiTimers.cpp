#include "Precomp.h"
#include "PoaGuiTimers.h"
#include "Packages/Core/Properties/UArrayProperty.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include <cmath>
#include <unordered_set>
namespace PoaGuiTimers
{
static ScriptArray& Timers(UObject* page)
{
    auto p=UObject::TryCast<UArrayProperty>(page->GetMemberProperty("Timers"));
    if(!p||!p->Inner||p->Inner->ValueType!=ExpressionValueType::ValueObject||p->Inner->ElementSize()!=sizeof(UObject*))
        Exception::Throw("Invalid PoA GUI page timer array");
    return *static_cast<ScriptArray*>(page->GetProperty(p));
}
static UObject* Page(UObject* self)
{
    std::unordered_set<UObject*> visited;
    while(self){
        if(!visited.insert(self).second)Exception::Throw("Cyclic PoA GUI timer owner chain");
        auto owner=self->HasProperty("MenuOwner")?self->GetUObject("MenuOwner"):nullptr;
        if(!owner)break;
        self=owner;
    }
    if(!self||!self->IsA("GUIPage"))Exception::Throw("PoA GUI timer has no owning page");
    return self;
}
static void KillTimer(UObject* self)
{
    self->SetFloat("TimerCountdown",0);self->SetFloat("TimerInterval",0);self->SetBool("bTimerRepeat",false);
}
static void SetTimer(UObject* self,float interval,bool repeat)
{
    // GUI.dll 0x10022f40: owning page retains the component until its next update.
    auto page=Page(self);auto& timers=Timers(page);
    if(!std::isfinite(interval))Exception::Throw("Non-finite PoA GUI timer interval");
    if(interval>0){
        size_t index=timers.GetSize();
        for(size_t i=0;i<timers.GetSize();++i)if(*static_cast<UObject**>(timers.GetItem(i))==self){index=i;break;}
        if(index==timers.GetSize()){timers.Resize(index+1);*static_cast<UObject**>(timers.GetItem(index))=self;}
        self->SetInt("TimerIndex",uint32_t(index));
    }
    self->SetFloat("TimerCountdown",interval);self->SetFloat("TimerInterval",interval);
    self->SetBool("bTimerRepeat",interval>0&&repeat);
}
void Update(UObject* page,float elapsed)
{
    if(!page||!std::isfinite(elapsed)||elapsed<0)return;
    GCRoot<UObject> pageRoot(page);
    auto& timers=Timers(page);
    // Original 0x10023420 captures the initial count. New timers wait a frame.
    size_t remaining=timers.GetSize(),i=0;
    while(remaining-- && i<timers.GetSize()){
        auto component=*static_cast<UObject**>(timers.GetItem(i));
        if(!component){timers.Remove(i,1);continue;}
        GCRoot<UObject> componentRoot(component);
        component->SetInt("TimerIndex",uint32_t(i));
        float countdown=component->GetFloat("TimerCountdown")-elapsed;
        component->SetFloat("TimerCountdown",countdown);
        if(countdown<0 && component->GetFloat("TimerInterval")>0){
            CallEvent(component,"Timer");
            // Callbacks may kill, rearm or append timers; reread fields afterwards.
            if(component->GetBool("bTimerRepeat")){
                const float interval=component->GetFloat("TimerInterval");
                countdown=component->GetFloat("TimerCountdown")+interval;
                component->SetFloat("TimerCountdown",countdown<0?interval:countdown);
            }else{
                component->SetFloat("TimerInterval",0);component->SetFloat("TimerCountdown",0);
            }
        }
        if(component->GetFloat("TimerInterval")<=0){
            component->SetInt("TimerIndex",uint32_t(-1));timers.Remove(i,1);
        }else ++i;
    }
}
void RegisterNatives()
{
    RegisterVMNativeFunc_2("GUIComponent","SetTimer",&SetTimer,0);
    RegisterVMNativeFunc_0("GUIComponent","KillTimer",&KillTimer,0);
}
}
