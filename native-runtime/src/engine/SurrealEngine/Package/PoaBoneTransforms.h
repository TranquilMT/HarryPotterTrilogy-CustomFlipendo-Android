#pragma once
#include "PoaSkeletalMeshData.h"
#include "Math/mat.h"
#include "Math/coords.h"
namespace PoaBones
{
inline mat4 BoneMatrix(const PoaAssets::BoneTransform& bone)
{
    auto result=mat4::identity();
    for(size_t axis=0;axis<3;++axis)
    {
        PoaAssets::Vector unit={0,0,0};unit[axis]=1;auto direction=PoaAssets::RotateBoneVector(bone.Rotation,unit);
        for(size_t row=0;row<3;++row)result[axis*4+row]=direction[row];
    }
    for(size_t row=0;row<3;++row)result[12+row]=bone.Position[row];return result;
}
inline Coords QueryCoords(const mat4& world)
{
    Coords result;result.Origin=(world*vec4(0,0,0,1)).xyz();
    result.XAxis=(world*vec4(1,0,0,0)).xyz();result.YAxis=(world*vec4(0,1,0,0)).xyz();result.ZAxis=(world*vec4(0,0,1,0)).xyz();return result;
}
inline mat4 AttachmentMatrix(const mat4& bone,const vec3& relativeLocation,const Rotator& relativeRotation)
{
    return bone*mat4::translate(relativeLocation)*Coords::Rotation(relativeRotation).ToMatrix();
}
// Build a right-handed rotation after removing affine scale/shear. Location
// and query axes retain the full world affine transform separately.
inline mat4 RotationMatrix(const mat4& transform)
{
    auto x=(transform*vec4(1,0,0,0)).xyz(),y=(transform*vec4(0,1,0,0)).xyz();
    if(length(x)<1e-8f)throw std::runtime_error("Degenerate PoA bone frame");x=normalize(x);y-=x*dot(x,y);
    if(length(y)<1e-8f)throw std::runtime_error("Degenerate PoA bone frame");y=normalize(y);auto z=cross(x,y);
    auto rotation=mat4::identity();for(size_t row=0;row<3;++row){rotation[row]=x[row];rotation[4+row]=y[row];rotation[8+row]=z[row];}return rotation;
}
}
