#pragma once
#include <optional>
namespace PoaCollision
{
template<class Actor> bool WorldGeometry(Actor* actor)
{
    return actor && actor->HasProperty("bWorldGeometry") && actor->GetBool("bWorldGeometry");
}
// Retail IsBlockedBy checks world geometry before actor/player blocking flags.
template<class Actor> std::optional<bool> WorldBlocking(Actor* self,Actor* other,bool poa)
{
    if (!poa || !self || !WorldGeometry(other)) return std::nullopt;
    return bool(self->bCollideWorld());
}
template<class Actor> bool TraceStaticMesh(Actor* actor,bool traceActors,bool traceWorld)
{
    return WorldGeometry(actor) ? traceWorld : (traceActors && actor && bool(actor->bCollideActors()));
}
}
