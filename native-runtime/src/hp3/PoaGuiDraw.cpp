#include "Precomp.h"
#include "PoaGuiDraw.h"
#include "PoaGuiImage.h"
#include "Native/NCanvas.h"
#include "Packages/Core/Properties/UProperty.h"
#include "Packages/Engine/UCanvas.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
namespace PoaGuiDraw
{
template<class T> static T* StyleField(UObject* self,const char* name,uint8_t state)
{
    auto property=self->GetMemberProperty(name);
    if(!property || state>=property->ArrayDimension || property->ElementSize()!=sizeof(T))return nullptr;
    return static_cast<T*>(property->GetElement(self->GetProperty(property),state));
}
static void Tile(UCanvas* canvas,UTexture* texture,float left,float top,float width,float height,float u,float v,float ul,float vl)
{
    const float line=canvas->CurYL();
    canvas->CurX()=left;canvas->CurY()=top;
    try {NCanvas::DrawTile(canvas,texture,width,height,u,v,ul,vl);}
    catch(...){canvas->CurX()=left;canvas->CurY()=top;canvas->CurYL()=line;throw;}
    canvas->CurX()=left;canvas->CurY()=top;canvas->CurYL()=line;
}
static UTexture* Texture(UObject* material)
{
    if(!material)return nullptr;
    auto texture=UObject::TryCast<UTexture>(material);
    if(!texture)Exception::Throw("PoA GUI image drawing requires texture material support");
    return texture;
}
static void Stretched(UCanvas* canvas,UTexture* texture,float left,float top,float width,float height)
{
    const float line=canvas->CurYL();
    try {
        PoaGuiImage::Stretched(left,top,width,height,float(texture->USize()),float(texture->VSize()),
            [&](float x,float y,float w,float h,float u,float v,float ul,float vl){Tile(canvas,texture,x,y,w,h,u,v,ul,vl);});
    }catch(...){canvas->CurX()=left;canvas->CurY()=top;canvas->CurYL()=line;throw;}
    canvas->CurX()=left;canvas->CurY()=top;canvas->CurYL()=line;
}
static void DrawTileStretched(UObject* self,UObject* material,float width,float height)
{
    auto canvas=UObject::Cast<UCanvas>(self);
    if(!material||canvas->Style()==0)return;
    auto texture=Texture(material);Stretched(canvas,texture,canvas->CurX(),canvas->CurY(),width,height);
}
static bool DrawDelegate(UObject* self,UObject* canvas,uint8_t state,float left,float top,float width,float height)
{
    auto property=UObject::TryCast<UPoaDelegateProperty>(self->GetMemberProperty("__OnDraw__Delegate"));
    if(!property)return false;
    UObject* receiver=self;NameString function;property->ResolveBinding(receiver,function);
    if(function.IsNone()||!receiver)return false;
    GCRoot<UObject> receiverRoot(receiver);
    return CallEvent(receiver,function,{ExpressionValue::ObjectValue(canvas),ExpressionValue::ByteValue(state),
        ExpressionValue::FloatValue(left),ExpressionValue::FloatValue(top),ExpressionValue::FloatValue(width),ExpressionValue::FloatValue(height)}).ToBool();
}
static void DrawStyle(UObject* self,UObject* object,uint8_t state,float left,float top,float width,float height)
{
    if(!self||!std::isfinite(left)||!std::isfinite(top)||!std::isfinite(width)||!std::isfinite(height))return;
    auto canvas=UObject::Cast<UCanvas>(object);
    // Five fixed state slots; reject the retail helper's unsafe sixth slot.
    if(!StyleField<UObject*>(self,"Images",state))return;
    GCRoot<UObject> selfRoot(self),canvasRoot(canvas);
    if(DrawDelegate(self,canvas,state,left,top,width,height))return;
    auto slot=StyleField<UObject*>(self,"Images",state);auto style=StyleField<uint8_t>(self,"RStyles",state);
    auto color=StyleField<Color>(self,"ImgColors",state);auto mode=StyleField<uint8_t>(self,"ImgStyle",state);
    if(!slot||!*slot||!style||!color||!mode)return;
    GCRoot<UObject> imageRoot(*slot);auto texture=Texture(imageRoot.get());
    canvas->Style()=*style;canvas->DrawColor()=*color;
    if(*mode==0)Tile(canvas,texture,left,top,float(texture->USize()),float(texture->VSize()),0,0,float(texture->USize()),float(texture->VSize()));
    else if(*mode==1)Stretched(canvas,texture,left,top,width,height);
    else if(*mode==2)Tile(canvas,texture,left,top,width,height,0,0,float(texture->USize()),float(texture->VSize()));
}
void RegisterNatives()
{
    RegisterVMNativeFunc_3("Canvas","DrawTileStretched",&DrawTileStretched,0);
    RegisterVMNativeFunc_6("GUIStyles","Draw",&DrawStyle,0);
}
}
