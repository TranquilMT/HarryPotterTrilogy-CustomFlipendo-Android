#pragma once
#include "PoaCollisionExtent.h"
#include <algorithm>
namespace PoaCollision
{
template<class Actor> inline bool BoxActor(Actor* a) { return a->HasProperty("CollideType") && a->GetByte("CollideType")==1; }
template<class Actor> inline std::optional<dvec3> ActorExtent(Actor* a)
{
    return Extent(a->HasProperty("CollideType")?a->GetByte("CollideType"):0,a->CollisionRadius(),a->HasProperty("CollisionWidth")?a->GetFloat("CollisionWidth"):0,a->CollisionHeight());
}
inline bool BoxOverlap(dvec3 p,dvec3 e) { return std::abs(p.x)<e.x && std::abs(p.y)<e.y && std::abs(p.z)<e.z; }
inline bool BoxCylinderOverlap(dvec3 p,dvec3 box,double radius,double height)
{
    if(std::abs(p.z)>=box.z+height) return false;
    double x=std::max(std::abs(p.x)-box.x,0.0),y=std::max(std::abs(p.y)-box.y,0.0);
    return radius==0 ? std::abs(p.x)<box.x&&std::abs(p.y)<box.y : x*x+y*y<radius*radius;
}
struct Interval { double enter,exit; dvec3 normal{0.0}; };
inline bool Slab(Interval& i,double p,double d,double e,int axis)
{
    if(d==0) return std::abs(p)<=e;
    double a=(-e-p)/d,b=(e-p)/d;dvec3 n(0.0);n[axis]=d>0?-1:1;
    if(a>b)std::swap(a,b);
    if(a>i.enter){i.enter=a;i.normal=n;}i.exit=std::min(i.exit,b);return i.enter<=i.exit;
}
inline double SweepBox(dvec3 p,dvec3 d,double lo,double hi,dvec3 e,dvec3& normal)
{
    Interval i{lo,hi};for(int k=0;k<3;++k)if(!Slab(i,p[k],d[k],e[k],k))return hi;
    if(i.enter<hi){normal=i.normal;return i.enter;}return hi;
}
// XY Minkowski sum of a rectangle and disk, intersected with the vertical slab.
inline double SweepBoxCylinder(dvec3 p,dvec3 d,double lo,double hi,dvec3 box,double radius,double height,dvec3& normal)
{
    Interval z{lo,hi};if(!Slab(z,p.z,d.z,box.z+height,2))return hi;
    double best=hi;dvec3 bestNormal(0.0);
    auto accept=[&](Interval i){if(i.enter<=i.exit && i.enter<best){best=i.enter;bestNormal=i.normal;}};
    for(int k=0;k<2;++k){Interval i=z;dvec3 e=box;e[k]+=radius;if(Slab(i,p.x,d.x,e.x,0)&&Slab(i,p.y,d.y,e.y,1))accept(i);}
    double a=d.x*d.x+d.y*d.y;
    for(int x:{-1,1})for(int y:{-1,1}){
        double px=p.x-x*box.x,py=p.y-y*box.y,c=px*px+py*py-radius*radius;Interval i=z;
        if(a==0){if(c>0)continue;}else{double b=px*d.x+py*d.y,disc=b*b-a*c;if(disc<0)continue;double root=std::sqrt(disc),entry=(-b-root)/a;i.exit=std::min(i.exit,(-b+root)/a);if(entry>i.enter){i.enter=entry;double nx=px+d.x*entry,ny=py+d.y*entry;double len=std::sqrt(nx*nx+ny*ny);i.normal=len>0?dvec3(nx/len,ny/len,0):dvec3(0.0);}}
        accept(i);
    }
    normal=bestNormal;return best;
}
}
