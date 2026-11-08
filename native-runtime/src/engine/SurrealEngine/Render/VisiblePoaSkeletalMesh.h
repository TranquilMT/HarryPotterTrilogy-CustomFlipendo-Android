#pragma once
#include "VisiblePoaStaticMesh.h"
#include "Packages/Engine/Resources/Mesh/UPoaSkeletalMesh.h"
#include "Packages/Engine/Resources/Mesh/UPoaMeshAnimation.h"
// Initial full-resolution CPU skinning path. Actor's base animation fields are
// sampled; multi-channel blending, root motion and object notifies remain open.
inline bool DrawPoaSkeletalMesh(VisibleFrame* frame,UActor* actor,UPoaSkeletalMesh* mesh,bool translucentPass)
{
    mesh->LoadNow();
    const auto& geometry=mesh->Geometry;
    const PoaAssets::AnimationData* animation=nullptr;size_t sequence=0;float time=0;bool loop=false;
    if(mesh->AnimationResource&&actor->HasProperty("AnimSequence")&&actor->HasProperty("AnimFrame"))
    {
        const auto& resource=mesh->AnimationResource->Animation;
        for(size_t i=0;i<resource.Sequences.size();++i)if(NameString(resource.Sequences[i].Name)==actor->GetName("AnimSequence"))
        {
            animation=&resource;sequence=i;time=std::max(actor->GetFloat("AnimFrame"),0.0f)*resource.Moves[i].Duration;
            loop=actor->HasProperty("bAnimLoop")&&actor->GetBool("bAnimLoop");break;
        }
    }
    auto pose=PoaAssets::SkeletalPose(geometry,animation,sequence,time,loop);
    auto points=PoaAssets::SkinSkeletalPoints(geometry,pose);
    const auto transform=PoaStaticMeshToWorld(actor)*mesh->meshToObject;
    const auto x=(transform*vec4(1,0,0,0)).xyz(),y=(transform*vec4(0,1,0,0)).xyz(),z=(transform*vec4(0,0,1,0)).xyz();
    const auto determinant=dot(cross(x,y),z);if(std::abs(determinant)<1e-12f)return false;
    bool pending=false;
    for(size_t slot=0;slot<geometry.MaterialSlots.size();++slot)
    {
        const auto& materialSlot=geometry.MaterialSlots[slot];const int index=materialSlot.TextureIndex;
        if(index<0||size_t(index)>=geometry.Materials.size())continue;
        auto material=mesh->SourcePackage->GetUObject(geometry.Materials[index]);
        if(actor->HasProperty("Skin")&&actor->GetUObject("Skin"))material=actor->GetUObject("Skin");
        uint32_t flags=materialSlot.Flags;std::set<UObject*> seen;auto texture=PoaDiffuseTexture(material,flags,seen);if(!texture)continue;
        if(actor->Style()==STY_Masked)flags|=PF_Masked;
        if(actor->Style()==STY_Translucent)flags|=PF_Translucent;
        if(actor->Style()==STY_Modulated)flags|=PF_Modulated;
        const bool translucent=(flags&(PF_Translucent|PF_Modulated|PF_Highlighted))!=0;
        if(translucent&&!translucentPass){pending=true;continue;}if(!translucent&&translucentPass)continue;
        engine->render->UpdateTexture(texture);TextureInfo info;engine->render->UpdateTextureInfo(info,texture);
        for(const auto& triangle:geometry.Triangles)if(triangle.Material==slot)
        {
            GouraudVertex vertices[3];
            for(size_t c=0;c<3;++c)
            {
                const auto corner=determinant<0&&c?3-c:c;const auto& wedge=geometry.Wedges[triangle.Wedges[corner]];const auto& p=points[wedge.Point];
                vertices[c].Point=(transform*vec4(p[0],p[1],p[2],1)).xyz();vertices[c].UV={wedge.UV[0]*info.USize,wedge.UV[1]*info.VSize};
                vertices[c].Light=vec3(1.0f);vertices[c].Fog=vec4(0.0f);
            }
            frame->Device->DrawGouraudPolygon(&frame->Frame,info,vertices,3,flags);
        }
    }
    return pending;
}
