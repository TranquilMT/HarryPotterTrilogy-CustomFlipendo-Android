#pragma once
#include "Package/PoaSkeletalMeshData.h"
class UActor;
class ObjectStream;
class PackageStreamWriter;
class UPoaSkeletalMesh;
namespace PoaActorAnimation
{
void RegisterNatives();
void Load(UActor* actor,ObjectStream* stream);
void Save(UActor* actor,PackageStreamWriter* stream);
void Tick(UActor* actor,float elapsed);
std::vector<PoaAssets::BoneTransform> Pose(UActor* actor,UPoaSkeletalMesh* mesh);
}
