#pragma once
#include <cmath>
#include <algorithm>
namespace PoaGuiImage
{
// Original EU UCanvas::DrawTileStretched (0x103f6e70). Keep draw order:
// horizontal strips, vertical strips, centre, corners. Odd source dimensions
// deliberately retain the retail one-pixel gap rather than inventing a seam fix.
template<class Draw> void Stretched(float x,float y,float width,float height,float sourceWidth,float sourceHeight,Draw draw)
{
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(width)||!std::isfinite(height)||
       !std::isfinite(sourceWidth)||!std::isfinite(sourceHeight)||width<=0||height<=0||sourceWidth<=0||sourceHeight<=0)return;
    const bool wide=width>sourceWidth,tall=height>sourceHeight;
    const float w=wide?std::floor(sourceWidth*.5f):width*.5f;
    const float h=tall?std::floor(sourceHeight*.5f):height*.5f;
    const float right=x+width-w,bottom=y+height-h;
    const float u=sourceWidth-w,v=sourceHeight-h;
    const float middleWidth=width-sourceWidth,middleHeight=height-sourceHeight;
    if(wide){const float stripH=std::min(h,std::floor(sourceHeight*.5f));draw(x+w,y,middleWidth,stripH,w,0,1,stripH);draw(x+w,y+height-stripH,middleWidth,stripH,w,sourceHeight-stripH,1,stripH);}
    if(tall){draw(x,y+h,w,middleHeight,0,h,w,1);draw(right,y+h,w,middleHeight,u,h,w,1);}
    if(wide&&tall)draw(x+w,y+h,middleWidth,middleHeight,w,h,1,1);
    draw(x,y,w,h,0,0,w,h);draw(right,y,w,h,u,0,w,h);
    draw(x,bottom,w,h,0,v,w,h);draw(right,bottom,w,h,u,v,w,h);
}
}
