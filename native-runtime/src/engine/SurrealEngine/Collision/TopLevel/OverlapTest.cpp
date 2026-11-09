
#include "Precomp.h"
#include "Engine.h"
#include "KW.h"
#include "Math/coords.h"
#include "Collision/BottomLevel/PoaBoxCollision.h"
#include "Package/PoaWorldCollision.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "OverlapTest.h"
#include "PoaStaticMeshTrace.h"
#include "Collision/BottomLevel/OverlapAABBModel.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Brush/UMover.h"

CollisionHitList OverlapTester::TestOverlap(const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly)
{
	if (!testActors && !testWorld)
		return {};

	vec3 extents = { radius, radius, height };

	CollisionHitList hits;

	if ((testActors && !visibilityOnly) || (engine->LaunchInfo.IsHarryPotter3() && testWorld))
	{
		double dradius = radius;
		double dheight = height;
		dvec3 dlocation = to_dvec3(location);

		int checkCounter = NextCheckCounter();
		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
		{
			for (int z = start.z; z < end.z; z++)
			{
				for (int y = start.y; y < end.y; y++)
				{
					for (int x = start.x; x < end.x; x++)
					{
						for (UActor* actor : GetActors(x, y, z))
						{
							if (actor->Collision.CheckCounter != checkCounter)
							{
								actor->Collision.CheckCounter = checkCounter;
                                if (engine->LaunchInfo.IsHarryPotter3())
                                {
                                    if (auto mesh = GetPoaStaticMesh(actor))
                                    {
                                        if (testWorld) TracePoaStaticMesh(actor, mesh, dlocation, 0.0, dvec3(0.0), 1.0, dheight, dradius, hits);
                                        continue;
                                    }
                                }
                                if (!testActors || visibilityOnly) continue;

								if (CylinderActorOverlap(dlocation, dheight, dradius, actor))
								{
									vec3 normal(0.0f); // To do: do we need the normal for contact tests?
									hits.push_back({ 0.0f, normal, actor, nullptr, nullptr });
								}
							}
						}
					}
				}
			}
		}
	}

	if (testWorld)
	{
		if (extents != vec3(0.0f))
		{
			OverlapAABBModel overlapmodel;
			CollisionHitList worldHits = overlapmodel.TestOverlap(GetLevelInfo(), GetLevel()->Model, location, extents, visibilityOnly);
			hits.push_back(worldHits);
		}
	}

	// Only include each actor once

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Actor < b.Actor; });

	UActor* prevActor = nullptr;
	CollisionHitList uniqueHits;
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			if (hit.Actor != prevActor)
			{
				prevActor = hit.Actor;
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

CollisionHitList OverlapTester::TestOverlapExtent(const vec3& location, const vec3& extents, bool testActors, bool testWorld, bool visibilityOnly)
{
	if (!testActors && !testWorld)
		return {};

	if(!std::isfinite(extents.x)||!std::isfinite(extents.y)||!std::isfinite(extents.z)||extents.x<0||extents.y<0||extents.z<0)return {};

	CollisionHitList hits;

	if ((testActors && !visibilityOnly) || (engine->LaunchInfo.IsHarryPotter3() && testWorld))
	{


		dvec3 dlocation = to_dvec3(location);

		int checkCounter = NextCheckCounter();
		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
		{
			for (int z = start.z; z < end.z; z++)
			{
				for (int y = start.y; y < end.y; y++)
				{
					for (int x = start.x; x < end.x; x++)
					{
						for (UActor* actor : GetActors(x, y, z))
						{
							if (actor->Collision.CheckCounter != checkCounter)
							{
								actor->Collision.CheckCounter = checkCounter;
                                if (engine->LaunchInfo.IsHarryPotter3())
                                {
                                    if (auto mesh = GetPoaStaticMesh(actor))
                                    {
                                        if (PoaCollision::TraceStaticMesh(actor,testActors&&!visibilityOnly,testWorld)) TracePoaStaticMeshExtent(actor, mesh, dlocation, 0.0, dvec3(0.0), 1.0, to_dvec3(extents), hits);
                                        continue;
                                    }
                                }
                                if(auto mover=UObject::TryCast<UMover>(actor)){if(testWorld && BoxActorOverlap(dlocation,to_dvec3(extents),mover))hits.push_back({0.0f,vec3(0.0f),mover,nullptr,nullptr});continue;}
                                if (!testActors || visibilityOnly || !actor->bCollideActors()) continue;

								if (BoxActorOverlap(dlocation, to_dvec3(extents), actor))
								{
									vec3 normal(0.0f); // To do: do we need the normal for contact tests?
									hits.push_back({ 0.0f, normal, actor, nullptr, nullptr });
								}
							}
						}
					}
				}
			}
		}
	}

	if (testWorld && GetLevel() && GetLevel()->Model && !GetLevel()->Model->Nodes.empty())
	{
		if (extents != vec3(0.0f))
		{
			OverlapAABBModel overlapmodel;
			CollisionHitList worldHits = overlapmodel.TestOverlap(GetLevelInfo(), GetLevel()->Model, location, extents, visibilityOnly);
			hits.push_back(worldHits);
		}
	}

	// Only include each actor once

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Actor < b.Actor; });

	UActor* prevActor = nullptr;
	CollisionHitList uniqueHits;
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			if (hit.Actor != prevActor)
			{
				prevActor = hit.Actor;
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

Array<UActor*> OverlapTester::CollidingActors(const vec3& origin, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dradius = radius;
	vec3 extents = { radius, radius, radius };

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* actor : GetActors(x, y, z))
					{
						if (actor->Collision.CheckCounter != checkCounter)
						{
							actor->Collision.CheckCounter = checkCounter;
                            if(engine->LaunchInfo.IsHarryPotter3()&&!actor->bCollideActors())continue;
							if (SphereActorOverlap(dorigin, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

Array<UActor*> OverlapTester::CollidingActors(const vec3& origin, float height, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dheight = height;
	double dradius = radius;
	vec3 extents = { radius, radius, height };

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* actor : GetActors(x, y, z))
					{
						if (actor->Collision.CheckCounter != checkCounter)
						{
							actor->Collision.CheckCounter = checkCounter;
                            if(engine->LaunchInfo.IsHarryPotter3()&&!actor->bCollideActors())continue;
							if (CylinderActorOverlap(dorigin, dheight, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

Array<UActor*> OverlapTester::EncroachingActors(UActor* actor)
{
	UModel* brush = actor->Brush();
	if (!brush)
		return CollidingActors(actor->Location(), actor->CollisionHeight(), actor->CollisionRadius());

	// To do: is radius and height correct for a mover? Should it use the brush bounding box?

	vec3 origin = actor->Location();
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 extents = { radius, radius, height };

	mat4 rotateObjToWorld = Coords::Rotation(actor->Rotation()).ToMatrix();
	mat4 rotateWorldToObj = mat4::transpose(rotateObjToWorld);
	vec3 prePivot = actor->PrePivot();

	vec3 mainScale(1.0f);
	if (UMover* mover = UActor::TryCast<UMover>(actor))
	{
		mainScale = mover->MainScale().Scale;
	}

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* testActor : GetActors(x, y, z))
					{
						if ((engine->LaunchInfo.IsHarryPotter3()?testActor:actor)->Collision.CheckCounter != checkCounter)
						{
							(engine->LaunchInfo.IsHarryPotter3()?testActor:actor)->Collision.CheckCounter = checkCounter;
							if (testActor == actor || testActor->Brush())
								continue;

							vec3 localOrigin = (rotateWorldToObj * vec4(testActor->Location() - origin, 1.0f)).xyz() / mainScale + prePivot;

							float testHeight = testActor->CollisionHeight();
							float testRadius = testActor->CollisionRadius();
							vec3 testExtents = { testRadius, testRadius, testHeight };
                            if(engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(testActor)){
                                auto e=PoaCollision::ActorExtent(testActor);if(!e)continue;
                                if(BoxActorOverlap(to_dvec3(testActor->Location()),*e,actor))hits.push_back(testActor);
                                continue;
                            }
							testExtents /= mainScale;

							OverlapAABBModel test;
							auto modelHits = test.TestOverlap(actor, brush, localOrigin, testExtents, false);
							if (!modelHits.empty())
								hits.push_back(testActor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

bool OverlapTester::IsOverlapping(UActor* actor1, UActor* actor2)
{
    if(engine->LaunchInfo.IsHarryPotter3()){
        if(PoaCollision::BoxActor(actor1)){auto e=PoaCollision::ActorExtent(actor1);return e && BoxActorOverlap(to_dvec3(actor1->Location()),*e,actor2);}
        if(PoaCollision::BoxActor(actor2)){auto e=PoaCollision::ActorExtent(actor2);return e && BoxActorOverlap(to_dvec3(actor2->Location()),*e,actor1);}
    }

	if (engine->LaunchInfo.IsKnowWonder() && KW::IsBoxCollider(actor1) && !KW::IsBoxCollider(actor2)) // CollideType CT_Box
		return CylinderActorOverlap(to_dvec3(actor2->Location()), actor2->CollisionHeight(), actor2->CollisionRadius(), actor1);
	return CylinderActorOverlap(to_dvec3(actor1->Location()), actor1->CollisionHeight(), actor1->CollisionRadius(), actor2);
}

bool OverlapTester::CylinderCylinderOverlap(const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, const dvec3& cylinderCenterB, double cylinderHeightB, double cylinderRadiusB)
{
	dvec3 dist = cylinderCenterA - cylinderCenterB;
	double h = cylinderHeightA + cylinderHeightB;
	double r = cylinderRadiusA + cylinderRadiusB;
	return std::abs(dist.z) < h && dot(dist.xy(), dist.xy()) < r * r;
}

// Returns the squared distance between point c and segment ab
static double SqDistPointSegment(const dvec3& a, const dvec3& b, const dvec3& c)
{
	dvec3 ab = b - a;
	dvec3 ac = c - a;
	double e = dot(ac, ab);

	// Handle cases where c projects outside ab
	if (e <= 0.0)
	{
		return dot(ac, ac);
	}
	double f = dot(ab, ab);
	if (e >= f)
	{
		dvec3 bc = c - b;
		return dot(bc, bc);
	}

	// Handle cases where c projects onto ab
	return dot(ac, ac) - e * e / f;
}

bool OverlapTester::SphereCapsuleOverlap(const dvec3& sphereCenter, double sphereRadius, const dvec3& capsuleCenter, double capsuleHeight, double capsuleRadius)
{
	dvec3 base = capsuleCenter - capsuleHeight;
	dvec3 tip = capsuleCenter + capsuleHeight;

	// Compute (squared) distance between sphere center and capsule line segment
	double dist2 = SqDistPointSegment(base, tip, sphereCenter);

	// If (squared) distance smaller than (squared) sum of radii, they collide
	double totalradius = sphereRadius + capsuleRadius;
	return dist2 <= totalradius * totalradius;
}

bool OverlapTester::SphereActorOverlap(const dvec3& sphereCenter, double sphereRadius, UActor* actor)
{
    if(engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(actor) && !actor->Brush()){
        auto e=PoaCollision::ActorExtent(actor);if(!e)return false;
        dvec3 p=sphereCenter-to_dvec3(actor->Location());double dist=0;
        for(int k=0;k<3;++k){double v=std::max(std::abs(p[k])-(*e)[k],0.0);dist+=v*v;}
        return dist<sphereRadius*sphereRadius;
    }

	if (actor->Brush()) // Ignore brushes for now
		return false;

	if (engine->LaunchInfo.IsKnowWonder() && KW::IsBoxCollider(actor)) // CollideType CT_Box
		return KW::BoxActorOverlapSphere(actor, sphereCenter, sphereRadius);

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();

	// Note: this isn't entirely accurate - what does UE1 do?
	return SphereCapsuleOverlap(sphereCenter, sphereRadius, center, std::max(height - radius, 0.0), radius);
}

bool OverlapTester::CylinderActorOverlap(const dvec3& origin, double cylinderHeight, double cylinderRadius, UActor* actor)
{
    if(engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(actor) && !actor->Brush()){
        auto e=PoaCollision::ActorExtent(actor);return e && PoaCollision::BoxCylinderOverlap(origin-to_dvec3(actor->Location()),*e,cylinderRadius,cylinderHeight);
    }

    if (engine->LaunchInfo.IsHarryPotter3())
    {
        if (auto mesh = GetPoaStaticMesh(actor))
        {
            CollisionHitList hits;
            TracePoaStaticMesh(actor, mesh, origin, 0.0, dvec3(0.0), 1.0, cylinderHeight, cylinderRadius, hits);
            return !hits.empty();
        }
    }

	if (actor->Brush()) // Ignore brushes for now
		return false;

	if (engine->LaunchInfo.IsKnowWonder() && KW::IsBoxCollider(actor)) // CollideType CT_Box
		return KW::BoxActorOverlapCylinder(actor, origin, cylinderHeight, cylinderRadius);

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	return CylinderCylinderOverlap(origin, cylinderHeight, cylinderRadius, center, height, radius);
}

bool OverlapTester::BoxActorOverlap(const dvec3& origin,const dvec3& extent,UActor* actor)
{
    if(auto mesh=GetPoaStaticMesh(actor)){CollisionHitList hits;TracePoaStaticMeshExtent(actor,mesh,origin,0,dvec3(0.0),1,extent,hits);return !hits.empty();}
    if(auto mover=UObject::TryCast<UMover>(actor)){
        auto scale=mover->MainScale().Scale;if(!mover->Brush()||scale.x==0||scale.y==0||scale.z==0)return false;
        auto inverse=mat4::transpose(Coords::Rotation(mover->Rotation()).ToMatrix());
        vec3 local=(inverse*vec4(vec3(origin)-mover->Location(),1.0f)).xyz()/scale+mover->PrePivot();
        dvec3 e(0.0);for(int k=0;k<3;++k){vec3 axis(0.0f);axis[k]=1;auto v=(inverse*vec4(axis,0.0f)).xyz();for(int j=0;j<3;++j)e[j]+=std::abs(v[j])*extent[k];}
        e/=dvec3(std::abs(scale.x),std::abs(scale.y),std::abs(scale.z));
        OverlapAABBModel test;return !test.TestOverlap(mover,mover->Brush(),local,vec3(e),false).empty();
    }
    if(actor->Brush())return false;
    auto e=PoaCollision::ActorExtent(actor);if(!e)return false;auto p=origin-to_dvec3(actor->Location());
    return PoaCollision::BoxActor(actor)?PoaCollision::BoxOverlap(p,extent+*e):PoaCollision::BoxCylinderOverlap(p,extent,e->x,e->z);
}
