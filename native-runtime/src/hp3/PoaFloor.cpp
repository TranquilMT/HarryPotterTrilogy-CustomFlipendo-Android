#include "Precomp.h"
#include "PoaFloor.h"
#include "Collision/BottomLevel/PoaBoxCollision.h"
#include "Package/PoaWorldCollision.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/UProjectile.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
namespace PoaFloor
{
bool FindBase(UActor* actor)
{
    if (!actor || actor->bDeleteMe() || !actor->XLevel()) return false;
    auto extent=PoaCollision::ActorExtent(actor);
    if(!extent)return false;
    GCRoot<UActor> actorRoot(actor);
    const auto location=actor->Location();
    const auto hits=PoaCollision::BoxActor(actor)?actor->XLevel()->Collision.TraceExtent(location,location+vec3(0,0,-8),vec3(*extent),true,true,false):actor->XLevel()->Collision.Trace(location,location+vec3(0,0,-8),float(extent->z),float(extent->x),true,true,false);
    UActor* floor=nullptr;
    vec3 floorNormal(0.0f);
    const bool playerLike=UObject::TryCast<UPlayerPawn>(actor) || UObject::TryCast<UProjectile>(actor);
    for (const auto& hit:hits)
    {
        auto other=hit.Actor;
        if (!other || other==actor || other->bDeleteMe()) continue;
        bool blocking;
        if (other==actor->Level()) blocking=actor->bCollideWorld();
        else if (auto world=PoaCollision::WorldBlocking(actor,other,true)) blocking=*world;
        else if (playerLike || UObject::TryCast<UPlayerPawn>(other) || UObject::TryCast<UProjectile>(other))
            blocking=actor->bBlockPlayers() && other->bBlockPlayers();
        else blocking=actor->bBlockActors() && other->bBlockActors();
        if (blocking) { floor=other;floorNormal=hit.Normal;break; }
    }
    if (actor->ActorBase()!=floor)
    {
        GCRoot<UActor> floorRoot(floor);
        // Retail APawn::SetBase stores Floor before base-change callbacks.
        if (actor->HasProperty("Floor")) actor->SetVector("Floor",floorNormal);
        actor->SetBase(floor,true);
    }
    return true;
}
}
