#pragma once
#include "USkeletalMesh.h"
#include "Package/PoaSkeletalMeshData.h"
class UPoaMeshAnimation;
class UPoaSkeletalMesh : public USkeletalMesh
{
public:
    using USkeletalMesh::USkeletalMesh;
    void Load(ObjectStream* stream) override;
    void Save(PackageStreamWriter* stream) override;
    PoaAssets::SkeletalMeshData Geometry;
    Package* SourcePackage=nullptr;
    UPoaMeshAnimation* AnimationResource=nullptr;
};
