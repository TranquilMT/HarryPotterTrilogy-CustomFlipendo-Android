#pragma once
#include <cmath>
#include <optional>
#include "Math/vec.h"
namespace PoaCollision
{
// EU GetCollisionExtent: CT_Cylinder=0, CT_Box=1; zero width uses radius.
inline std::optional<dvec3> Extent(unsigned type,double radius,double width,double height)
{
    if (type>1 || !std::isfinite(radius) || !std::isfinite(height) || radius<0 || height<0) return {};
    if (type==1 && (!std::isfinite(width) || width<0)) return {};
    return dvec3(radius,type==1 && width!=0 ? width : radius,height);
}
}
