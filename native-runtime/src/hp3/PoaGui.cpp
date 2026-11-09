#include "Precomp.h"
#include "PoaGui.h"
#include "PoaGuiLayout.h"
#include "Engine.h"
#include "Packages/Core/UObject.h"
#include "Packages/Engine/UCanvas.h"
#include "VM/NativeFunc.h"
namespace PoaGui
{
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
void RegisterNatives()
{
    RegisterVMNativeFunc_1("GUIComponent","ActualWidth",&ActualWidth,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualHeight",&ActualHeight,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualLeft",&ActualLeft,0);
    RegisterVMNativeFunc_1("GUIComponent","ActualTop",&ActualTop,0);
}
}
