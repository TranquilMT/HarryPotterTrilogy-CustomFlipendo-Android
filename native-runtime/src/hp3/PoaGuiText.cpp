#include "Precomp.h"
#include "PoaGuiText.h"
#include "Engine.h"
#include "Native/NCanvas.h"
#include "Packages/Core/Properties/UProperty.h"
#include "Packages/Engine/UCanvas.h"
#include "Packages/Engine/Resources/UFont.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include <cmath>
namespace PoaGuiText
{
static vec2 JustifiedPosition(uint8_t align,float width,float height,float textWidth,float textHeight)
{
    // Retail WrappedStrLen supplies integers; its half-size division truncates.
    const float w=std::trunc(textWidth),h=std::trunc(textHeight);
    float x=0;
    if(align==1 && w<=width)x=width*.5f-std::trunc(w*.5f);
    else if(align==2)x=width-w;
    return {x,height*.5f-std::trunc(h*.5f)};
}
static void DrawTextJustified(UObject* self,uint8_t align,float left,float top,float right,float bottom,const std::string& text)
{
    auto canvas=UObject::Cast<UCanvas>(self);
    if(!std::isfinite(left)||!std::isfinite(top)||!std::isfinite(right)||!std::isfinite(bottom)||right<=left||bottom<=top)return;
    const float orgX=canvas->OrgX(),orgY=canvas->OrgY(),clipX=canvas->ClipX(),clipY=canvas->ClipY();
    const bool center=canvas->bCenter();
    const float oldX=canvas->CurX(),oldY=canvas->CurY();
    canvas->CurX()=canvas->CurY()=0;canvas->bCenter()=false;
    try {
        float width=0,height=0;NCanvas::StrLen(canvas,text,width,height);
        const auto position=JustifiedPosition(align,right-left,bottom-top,width,height);
        canvas->CurX()=position.x;canvas->CurY()=position.y;
        canvas->OrgX()=left;canvas->OrgY()=top;canvas->ClipX()=right-left;canvas->ClipY()=bottom-top;
        NCanvas::DrawTextClipped(canvas,text,false);
    }catch(...){
        canvas->OrgX()=orgX;canvas->OrgY()=orgY;canvas->ClipX()=clipX;canvas->ClipY()=clipY;
        canvas->CurX()=oldX;canvas->CurY()=oldY;canvas->bCenter()=center;throw;
    }
    // The original helper saves/restores these four values as integers.
    canvas->OrgX()=std::trunc(orgX);canvas->OrgY()=std::trunc(orgY);
    canvas->ClipX()=std::trunc(clipX);canvas->ClipY()=std::trunc(clipY);canvas->bCenter()=center;
}
template<class T> static T* StyleField(UObject* self,const char* name,uint8_t state)
{
    auto property=self->GetMemberProperty(name);
    if(!property || state>=property->ArrayDimension || property->ElementSize()!=sizeof(T))return nullptr;
    return static_cast<T*>(property->GetElement(self->GetProperty(property),state));
}
static bool DrawDelegate(UObject* self,UObject* canvas,uint8_t state,float left,float top,float width,float height,uint8_t align,const std::string& text)
{
    auto property=UObject::TryCast<UPoaDelegateProperty>(self->GetMemberProperty("__OnDrawText__Delegate"));
    if(!property)return false;
    UObject* receiver=self;NameString function;property->ResolveBinding(receiver,function);
    if(function.IsNone() || !receiver)return false;
    GCRoot<UObject> receiverRoot(receiver);
    return CallEvent(receiver,function,{ExpressionValue::ObjectValue(canvas),ExpressionValue::ByteValue(state),
        ExpressionValue::FloatValue(left),ExpressionValue::FloatValue(top),ExpressionValue::FloatValue(width),ExpressionValue::FloatValue(height),
        ExpressionValue::ByteValue(align),ExpressionValue::StringValue(text)}).ToBool();
}
static void DrawStyleText(UObject* self,UObject* object,uint8_t state,float left,float top,float width,float height,uint8_t align,const std::string& text)
{
    if(!self)return;
    auto canvas=UObject::Cast<UCanvas>(object);
    if(!std::isfinite(left)||!std::isfinite(top)||!std::isfinite(width)||!std::isfinite(height))return;
    // GUI.dll rounds all four values with appRound (floor(value + .5)).
    left=float(std::floor(double(left)+.5));top=float(std::floor(double(top)+.5));
    width=float(std::floor(double(width)+.5));height=float(std::floor(double(height)+.5));
    auto slot=StyleField<UObject*>(self,"Fonts",state);if(!slot||!*slot)return;
    GCRoot<UObject> selfRoot(self),canvasRoot(canvas),oldFontRoot(canvas->Font());
    auto oldFont=canvas->Font();const auto oldColor=canvas->DrawColor();const auto oldStyle=canvas->Style();
    if(DrawDelegate(self,canvas,state,left,top,width,height,align,text))return;
    // Read after the delegate: scripts can change the font and style fields.
    slot=StyleField<UObject*>(self,"Fonts",state);
    auto style=StyleField<uint8_t>(self,"RStyles",state);auto color=StyleField<Color>(self,"FontColors",state);
    if(!slot||!*slot||!style||!color)return;
    GCRoot<UObject> fontRoot(*slot);
    try {
        canvas->Style()=*style;canvas->DrawColor()=*color;
        canvas->Font()=UObject::TryCast<UFont>(CallEvent(fontRoot.get(),"GetFont",{ExpressionValue::ObjectValue(canvas),ExpressionValue::IntValue(canvas->SizeX())}).ToObject());
        if(canvas->Font())DrawTextJustified(canvas,align,left,top,left+width,top+height,text);
    }catch(...){canvas->Font()=oldFont;canvas->DrawColor()=oldColor;canvas->Style()=oldStyle;throw;}
    canvas->Font()=oldFont;canvas->DrawColor()=oldColor;canvas->Style()=oldStyle;
}
void RegisterNatives()
{
    RegisterVMNativeFunc_6("Canvas","DrawTextJustified",&DrawTextJustified,0);
    RegisterVMNativeFunc_8("GUIStyles","DrawText",&DrawStyleText,0);
}
}
