#pragma once
#include "UPoaStaticMesh.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Math/coords.h"
inline UPoaStaticMesh* GetPoaStaticMesh(UActor* actor)
{
    if(!actor->HasProperty("StaticMesh")) return nullptr;
    auto object=actor->GetUObject("StaticMesh");
    if(object) object->LoadNow();
    return UObject::TryCast<UPoaStaticMesh>(object);
}
inline mat4 PoaStaticMeshToWorld(UActor* actor)
{
    auto scale=actor->HasProperty("DrawScale3D")?actor->GetVector("DrawScale3D"):vec3(1.0f);
    return mat4::translate(actor->Location())*Coords::Rotation(actor->Rotation()).ToMatrix()*mat4::scale(scale*actor->DrawScale())*mat4::translate(-actor->PrePivot());
}
