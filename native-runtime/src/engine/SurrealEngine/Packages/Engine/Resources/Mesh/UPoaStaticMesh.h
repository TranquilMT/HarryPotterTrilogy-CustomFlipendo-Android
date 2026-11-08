#pragma once
#include "Packages/Engine/Resources/UPrimitive.h"
#include "Package/PoaStaticMeshData.h"
class UPoaStaticMesh : public UPrimitive
{
public:
    using UPrimitive::UPrimitive;
    void Load(ObjectStream* stream) override;
    void Save(PackageStreamWriter* stream) override;
    PoaAssets::StaticMeshData Geometry;
    Array<int> MaterialReferences;
    Array<uint8_t> MaterialCollision;
    Package* SourcePackage=nullptr;
    bool UseSimpleBoxCollision=false;
};
