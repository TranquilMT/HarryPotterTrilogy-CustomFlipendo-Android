#include "Precomp.h"
#include "UPoaSkeletalMesh.h"
#include "UPoaMeshAnimation.h"
#include "Package/Package.h"
#include "Math/coords.h"
void UPoaSkeletalMesh::Load(ObjectStream* stream)
{
    UPrimitive::Load(stream);
    auto geometry=PoaAssets::ReadSkeletalMesh(*stream,stream->GetVersion());
    PoaAssets::SkeletalPose(geometry); // validate hierarchy before replacing live data
    auto sourcePackage=stream->GetPackage();
    UPoaMeshAnimation* animationResource=nullptr;
    if(auto object=sourcePackage->GetUObject(geometry.AnimationReference))
    {
        object->LoadNow();animationResource=UObject::TryCast<UPoaMeshAnimation>(object);
    }
    Geometry=std::move(geometry);SourcePackage=sourcePackage;AnimationResource=animationResource;
    Scale={Geometry.Scale[0],Geometry.Scale[1],Geometry.Scale[2]};Origin={Geometry.Origin[0],Geometry.Origin[1],Geometry.Origin[2]};
    RotOrigin.Pitch=Geometry.Rotation[0];RotOrigin.Yaw=Geometry.Rotation[1];RotOrigin.Roll=Geometry.Rotation[2];
    meshToObject=Coords::Rotation(RotOrigin).ToMatrix()*mat4::scale(Scale)*mat4::translate(-Origin);
}
void UPoaSkeletalMesh::Save(PackageStreamWriter*)
{
    Exception::Throw("PoA skeletal mesh package rewriting is not implemented");
}
