#pragma once
class UActor;
namespace PoaFloor
{
// Returns false when the query shape/context is not supported; leaves its base intact.
bool FindBase(UActor* actor);
}
