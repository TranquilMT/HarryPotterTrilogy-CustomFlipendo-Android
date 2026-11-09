#pragma once
#include "Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h"
#include "Collision/BottomLevel/PoaSweepTriangle.h"
#include "CollisionHit.h"
inline void TracePoaStaticMeshExtent(UActor* actor,UPoaStaticMesh* mesh,const dvec3& origin,double tmin,const dvec3& direction,double tmax,const dvec3& extent,CollisionHitList& hits)
{
    if (!std::isfinite(extent.x) || !std::isfinite(extent.y) || !std::isfinite(extent.z) || extent.x<0 || extent.y<0 || extent.z<0) return;
    const auto flag=(extent.x==0.0&&extent.y==0.0&&extent.z==0.0)?"bBlockZeroExtentTraces":"bBlockNonZeroExtentTraces";
    if(actor->HasProperty(flag)&&!actor->GetBool(flag)) return;
    auto transform=PoaStaticMeshToWorld(actor);
    double nearest=tmax;dvec3 bestNormal(0.0);
    for(size_t sectionIndex=0;sectionIndex<mesh->Geometry.Sections.size();++sectionIndex)
    {
        if(sectionIndex<mesh->MaterialCollision.size()&&!mesh->MaterialCollision[sectionIndex]) continue;
        const auto& section=mesh->Geometry.Sections[sectionIndex];
        for(size_t i=0;i<section.Triangles;++i)
        {
            dvec3 points[3];
            for(size_t k=0;k<3;++k)
            {
                const auto index=mesh->Geometry.Indices[section.FirstIndex+i*3+k];
                const auto& p=mesh->Geometry.Vertices[index].Position;
                points[k]=dvec3((transform*vec4(p[0],p[1],p[2],1.0f)).xyz());
            }
            dvec3 normal;
            double t=PoaSweepTriangle(origin,direction,tmin,nearest,extent,points,normal);
            if(t<nearest){nearest=t;bestNormal=normal;}
        }
    }
    if(nearest<tmax) hits.push_back({float(nearest),vec3(bestNormal),actor,nullptr,nullptr});
}

// Preserve existing cylinder callers while allowing independent box width.
inline void TracePoaStaticMesh(UActor* actor,UPoaStaticMesh* mesh,const dvec3& origin,double tmin,const dvec3& direction,double tmax,double height,double radius,CollisionHitList& hits)
{
    TracePoaStaticMeshExtent(actor,mesh,origin,tmin,direction,tmax,dvec3(radius,radius,height),hits);
}
