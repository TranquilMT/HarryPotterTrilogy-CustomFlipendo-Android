#pragma once
#include <unordered_set>
namespace PoaGuiLayout
{
// EU GUI.dll ActualWidth/Height (0x1001a730/0x1001a8a0) and
// ActualLeft/Top (0x1001aa10/0x1001abc0). One normalized unit fills the extent.
template<class Object> float Resolve(Object* object,int axis,float width,float height,std::unordered_set<Object*>& active)
{
    if(!object || !object->HasProperty("Controller") || !object->GetUObject("Controller")) return 0;
    if(!active.insert(object).second) return 0; // Reject corrupt cyclic owner chains.
    const char* names[]={"WinWidth","WinHeight","WinLeft","WinTop"};
    if(!object->HasProperty(names[axis])){active.erase(object);return 0;}
    float value=object->GetFloat(names[axis]);
    auto owner=object->HasProperty("MenuOwner")?object->GetUObject("MenuOwner"):nullptr;
    bool position=axis>=2,vertical=(axis&1)!=0;
    const char* flag=position?"bBoundToParent":"bScaleToParent";
    bool parent=owner && object->HasProperty(flag) && object->GetBool(flag);
    float result=value;
    if(value<=1.0f) result=value*(parent?Resolve(owner,vertical?1:0,width,height,active):(vertical?height:width));
    if(position && parent)result+=Resolve(owner,axis,width,height,active);
    active.erase(object);return result;
}
template<class Object> float Resolve(Object* object,int axis,float width,float height)
{ std::unordered_set<Object*> active;return Resolve(object,axis,width,height,active); }
}
