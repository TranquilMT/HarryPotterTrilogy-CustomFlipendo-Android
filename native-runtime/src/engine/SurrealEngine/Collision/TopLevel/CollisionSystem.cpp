
#include "Precomp.h"
#include "Engine.h"
#include "Package/PoaWorldCollision.h"
#include "KW.h"
#include "Collision/BottomLevel/PoaBoxCollision.h"
#include "Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h"
#include "CollisionSystem.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Math/floating.h"
#include "Math/coords.h"
#include "Collision/BottomLevel/TraceRayModel.h"
#include "Collision/TopLevel/OverlapTest.h"
#include "Collision/TopLevel/TraceTest.h"

bool CollisionSystem::IsOverlapping(UActor* actor1, UActor* actor2)
{
	OverlapTester overlap(this);
	return overlap.IsOverlapping(actor1, actor2);
}

CollisionHitList CollisionSystem::OverlapTest(UActor* actor)
{
	if(engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(actor)){auto e=PoaCollision::ActorExtent(actor);return e?OverlapExtent(actor->Location(),vec3(*e),true,false,false):CollisionHitList{};}
	return OverlapTest(actor->Location(), actor->CollisionHeight(), actor->CollisionRadius(), true, false, false);
}

CollisionHitList CollisionSystem::OverlapTest(const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly)
{
	OverlapTester overlap(this);
	return overlap.TestOverlap(location, height, radius, testActors, testWorld, visibilityOnly);
}

bool CollisionSystem::TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	TraceTester trace(this);
	return trace.TraceAnyHit(from, to, tracingActor, traceActors, traceWorld, visibilityOnly);
}

CollisionHitList CollisionSystem::Trace(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	TraceTester trace(this);
	return trace.Trace(from, to, height, radius, traceActors, traceWorld, visibilityOnly);
}

CollisionHitList CollisionSystem::TraceDecal(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	TraceRayModel trace;
	return trace.Trace(LevelInfo, Level->Model, origin, tmin, dirNormalized, tmax, visibilityOnly);
}

CollisionHit CollisionSystem::TraceFirstHit(const vec3& from, const vec3& to, UActor* tracingActor, const vec3& extents, const TraceFlags& flags)
{
	for (const CollisionHit& hit : (engine->LaunchInfo.IsHarryPotter3()?TraceExtent(from,to,extents,flags.traceActors(),flags.traceWorld(),false):Trace(from, to, extents.z, extents.x, flags.traceActors(), flags.traceWorld(), false)))
	{
		if (hit.Actor != LevelInfo && (!tracingActor || !tracingActor->IsOwnedBy(hit.Actor)))
		{
            if (engine->LaunchInfo.IsHarryPotter3() && PoaCollision::WorldGeometry(hit.Actor))
            {
                if (flags.world) return hit;
                continue;
            }
			if (hit.Actor->IsA("Pawn"))
			{
				if (flags.pawns)
					return hit;
			}
			else if (hit.Actor->IsA("Mover"))
			{
				if (flags.movers)
					return hit;
			}
			else if (hit.Actor->IsA("ZoneInfo"))
			{
				if (flags.zoneChanges)
					return hit;
			}
			else if (flags.others)
			{
				if (!flags.onlyProjectiles || hit.Actor->bProjTarget() || (hit.Actor->bBlockActors() && hit.Actor->bBlockPlayers()))
					return hit;
			}
		}
		else if (flags.world && hit.Actor == LevelInfo)
		{
			return hit;
		}
	}
	return {};
}

Array<UActor*> CollisionSystem::CollidingActors(const vec3& origin, float radius)
{
	OverlapTester overlap(this);
	return overlap.CollidingActors(origin, radius);
}

Array<UActor*> CollisionSystem::CollidingActors(const vec3& origin, float height, float radius)
{
	OverlapTester overlap(this);
	return overlap.CollidingActors(origin, height, radius);
}

Array<UActor*> CollisionSystem::EncroachingActors(UActor* actor)
{
	OverlapTester overlap(this);
	return overlap.EncroachingActors(actor);
}

Array<UActor*> CollisionSystem::ActorsInBox(const vec3& boxMin, const vec3& boxMax)
{
	Array<UActor*> result;
	vec3 center = (boxMin + boxMax) * 0.5f;
	vec3 extents = (boxMax - boxMin) * 0.5f;
	ivec3 start = GetStartExtents(center, extents);
	ivec3 end = GetEndExtents(center, extents);
	if (end.x - start.x >= 100 || end.y - start.y >= 100 || end.z - start.z >= 100)
		return result;
	for (int z = start.z; z < end.z; z++)
	{
		for (int y = start.y; y < end.y; y++)
		{
			for (int x = start.x; x < end.x; x++)
			{
				auto it = CollisionActors.find(GetBucketId(x, y, z));
				if (it == CollisionActors.end())
					continue;
				for (UActor* actor : it->second)
				{
					if (std::find(result.begin(), result.end(), actor) == result.end())
						result.push_back(actor);
				}
			}
		}
	}
	return result;
}

void CollisionSystem::SetLevel(ULevelInfo* levelInfo, ULevel* level)
{
	LevelInfo = levelInfo;
	Level = level;
}

void CollisionSystem::AddToCollision(UActor* actor)
{
	if (actor->bCollideActors() || (engine->LaunchInfo.IsHarryPotter3() && PoaCollision::WorldGeometry(actor) && GetPoaStaticMesh(actor)))
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		vec3 extents = { radius, radius, height };
        if(engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(actor)){auto e=PoaCollision::ActorExtent(actor);if(!e)return;extents=vec3(*e);}

		if (UMover* mover = UObject::TryCast<UMover>(actor))
		{
			UModel* brush = mover->Brush();
			if (brush)
			{
				mat4 objectToWorld = mat4::translate(actor->Location()) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-actor->PrePivot());
				BBox bbox = brush->BoundingBox.transform(objectToWorld);
				location = bbox.center();
				extents = bbox.extents() + 0.1f; // 0.1 for numerical stability
			}
		}
		else if (engine->LaunchInfo.IsHarryPotter3() && GetPoaStaticMesh(actor))
        {
            BBox bbox = GetPoaStaticMesh(actor)->BoundingBox.transform(PoaStaticMeshToWorld(actor));
            location = bbox.center(); extents = bbox.extents() + 0.1f;
        }
        else if (engine->LaunchInfo.IsKnowWonder()) // KnowWonder hashes an actor by its primitive's world box (CT_Box, CollisionWidth offset)
		{
			BBox bbox = KW::ActorWorldCollisionBox(actor);
			location = bbox.center();
			extents = bbox.extents() + 0.1f;
		}

		actor->Collision.Inserted = true;
		actor->Collision.Location = location;
		actor->Collision.Extents = extents;

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					CollisionActors[GetBucketId(x, y, z)].push_back(actor);
				}
			}
		}
	}
}

void CollisionSystem::RemoveFromCollision(UActor* actor)
{
	if (actor->Collision.Inserted)
	{
		vec3 location = actor->Collision.Location;
		vec3 extents = actor->Collision.Extents;

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = CollisionActors.find(GetBucketId(x, y, z));
					if (it != CollisionActors.end())
					{
						it->second.remove(actor);
						if (it->second.empty())
							CollisionActors.erase(it);
					}
				}
			}
		}

		actor->Collision.Inserted = false;
	}
}

CollisionHitList CollisionSystem::TraceExtent(const vec3& from,const vec3& to,const vec3& extent,bool actors,bool world,bool visibility)
{ TraceTester test(this);return test.TraceExtent(from,to,extent,actors,world,visibility); }
CollisionHitList CollisionSystem::OverlapExtent(const vec3& location,const vec3& extent,bool actors,bool world,bool visibility)
{ OverlapTester test(this);return test.TestOverlapExtent(location,extent,actors,world,visibility); }
