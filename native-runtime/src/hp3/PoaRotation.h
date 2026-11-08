#pragma once
#include "PoaMath.h"
#include "Math/rotator.h"
#include <cmath>
#include <stdexcept>
namespace PoaRotation
{
inline double QuantizedRadians(int32_t value)
{
    return double((uint32_t(value)&65535u)>>2)*(6.283185307179586476925286766559/16384.0);
}
inline quaternion FromRotator(const Rotator& rotator)
{
    const auto p=QuantizedRadians(rotator.Pitch)*0.5;
    const auto y=QuantizedRadians(rotator.Yaw)*-0.5;
    const auto r=QuantizedRadians(rotator.Roll)*0.5;
    const quaternion pitch={0,float(std::sin(p)),0,float(std::cos(p))};
    const quaternion yaw={0,0,float(std::sin(y)),float(std::cos(y))};
    const quaternion roll={float(std::sin(r)),0,0,float(std::cos(r))};
    auto q=PoaCore::Product(PoaCore::Product(roll,pitch),yaw);
    const double norm=std::sqrt(double(q.x)*q.x+double(q.y)*q.y+double(q.z)*q.z+double(q.w)*q.w);
    // Stable unit rotation replaces the original DLL's non-unit half-turn branch.
    const auto factor=float((q.w<0?-1.0:1.0)/norm);
    return {q.x*factor,q.y*factor,q.z*factor,q.w*factor};
}
inline Rotator ToRotator(const quaternion& q)
{
    if(!std::isfinite(q.x)||!std::isfinite(q.y)||!std::isfinite(q.z)||!std::isfinite(q.w))
        throw std::runtime_error("Non-finite PoA quaternion");
    // EU FQuat -> FCoords row basis, retaining non-unit input behavior.
    const float xx=2*q.x*q.x,yy=2*q.y*q.y,zz=2*q.z*q.z;
    const float xy=2*q.x*q.y,xz=2*q.x*q.z,yz=2*q.y*q.z;
    const float wx=2*q.w*q.x,wy=2*q.w*q.y,wz=2*q.w*q.z;
    const vec3 x={1-(yy+zz),xy-wz,xz+wy};
    const vec3 y={xy+wz,1-(xx+zz),yz-wx};
    const vec3 z={xz-wy,yz+wx,1-(xx+yy)};
    for(float v:{x.x,x.y,x.z,y.x,y.y,y.z,z.x,z.y,z.z})
        if(!std::isfinite(v)) throw std::runtime_error("PoA quaternion basis overflow");
    constexpr double units=65536.0/6.283185307179586476925286766559;
    Rotator result;
    result.Yaw=int32_t(std::atan2(double(x.y),double(x.x))*units);
    result.Pitch=int32_t(std::atan2(double(x.z),std::hypot(double(x.x),double(x.y)))*units);
    const auto angle=QuantizedRadians(result.Yaw);
    const auto sine=float(std::sin(angle)),cosine=float(std::cos(angle));
    result.Roll=int32_t(std::atan2(-double(sine)*z.x+double(cosine)*z.y,-double(sine)*y.x+double(cosine)*y.y)*units);
    return result;
}
}
