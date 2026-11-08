#pragma once
#include "Package/PoaSkeletalMeshData.h"
class UActor;
class UPoaSkeletalMesh;
namespace PoaActorAnimation
{
void RegisterNatives();
void Tick(UActor* actor,float elapsed);
std::vector<PoaAssets::BoneTransform> Pose(UActor* actor,UPoaSkeletalMesh* mesh);
}
