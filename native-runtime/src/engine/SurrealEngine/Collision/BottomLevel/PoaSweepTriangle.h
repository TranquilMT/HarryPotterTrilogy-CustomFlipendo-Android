#pragma once
#include "Math/vec.h"
#include <algorithm>
#include <cmath>
// Continuous SAT between a world-axis box and a triangle. Testing world-space
// vertices keeps nonuniform/negative actor scale correct for both rays and boxes.
inline double PoaSweepTriangle(const dvec3& origin,const dvec3& direction,double tmin,double tmax,const dvec3& extent,const dvec3* triangle,dvec3& normal)
{
    const dvec3 edges[3]={triangle[1]-triangle[0],triangle[2]-triangle[1],triangle[0]-triangle[2]};
    const dvec3 face=cross(edges[0],edges[1]);
    if(dot(face,face)<1e-20) return tmax;
    double enter=tmin,leave=tmax;normal=dvec3(0.0);
    auto axisTest=[&](const dvec3& axis)
    {
        double len2=dot(axis,axis);if(len2<1e-20) return true;
        double low=dot(triangle[0],axis),high=low;
        for(int i=1;i<3;++i){double p=dot(triangle[i],axis);low=std::min(low,p);high=std::max(high,p);}
        const double radius=std::abs(axis.x)*extent.x+std::abs(axis.y)*extent.y+std::abs(axis.z)*extent.z;
        const double center=dot(origin,axis),velocity=dot(direction,axis);
        low-=radius;high+=radius;
        if(std::abs(velocity)<1e-12) return center>=low-1e-10&&center<=high+1e-10;
        double a=(low-center)/velocity,b=(high-center)/velocity;
        dvec3 hitNormal=velocity>0?-axis:axis;
        if(a>b) std::swap(a,b);
        if(a>=enter){enter=a;normal=hitNormal/std::sqrt(len2);}
        leave=std::min(leave,b);
        return enter<=leave+1e-10;
    };
    const dvec3 box[3]={dvec3(1,0,0),dvec3(0,1,0),dvec3(0,0,1)};
    if(!axisTest(face)) return tmax;
    for(const auto& axis:box) if(!axisTest(axis)) return tmax;
    for(const auto& edge:edges) for(const auto& axis:box) if(!axisTest(cross(edge,axis))) return tmax;
    if(leave<tmin||enter>=tmax||leave<=tmin+1e-10) return tmax;
    if(dot(normal,normal)==0.0) normal=normalize(dot(face,direction)>0?-face:face);
    return std::max(enter,tmin);
}
