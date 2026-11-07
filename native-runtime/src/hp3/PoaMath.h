#pragma once
#include "Math/quaternion.h"

namespace PoaCore
{
inline quaternion Product(const quaternion& a, const quaternion& b)
{
    return {a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
            a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
            a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
            a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z};
}
inline vec3 Rotate(const quaternion& q, const vec3& v)
{
    // UE2 quaternion/vector rotation is q * (v,0) * conjugate(q).
    const quaternion p = Product(Product(q, {v.x,v.y,v.z,0}), {-q.x,-q.y,-q.z,q.w});
    return {p.x,p.y,p.z};
}
}
