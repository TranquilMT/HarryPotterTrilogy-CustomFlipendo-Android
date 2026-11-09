#include "Precomp.h"
#include "PoaFloor.h"
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
    // Retail GetCollisionExtent has a separate width for CT_Box. The current
    // collision sweep API accepts a cylinder; do not silently replace a box.
    if (actor->HasProperty("CollideType") && actor->GetByte("CollideType") != 0) return false;
    const float radius=actor->CollisionRadius(),height=actor->CollisionHeight();
    if (!std::isfinite(radius) || !std::isfinite(height) || radius<0 || height<0) return false;
    GCRoot<UActor> actorRoot(actor);
    const auto location=actor->Location();
    const auto hits=actor->XLevel()->Collision.Trace(location,location+vec3(0,0,-8),height,radius,true,true,false);
    UActor* floor=nullptr;
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
        if (blocking) { floor=other;break; }
    }
    if (actor->ActorBase()!=floor)
    {
        GCRoot<UActor> floorRoot(floor);
        actor->SetBase(floor,true);
    }
    return true;
}
}
