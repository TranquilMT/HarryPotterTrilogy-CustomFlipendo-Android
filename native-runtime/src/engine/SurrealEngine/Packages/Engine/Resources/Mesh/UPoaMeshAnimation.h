#pragma once
#include "Packages/Core/UObject.h"
#include "Package/PoaAnimationData.h"
class UPoaMeshAnimation : public UObject
{
public:
    using UObject::UObject;
    void Load(ObjectStream* stream) override;
    void Save(PackageStreamWriter* stream) override;
    PoaAssets::AnimationData Animation;
};
