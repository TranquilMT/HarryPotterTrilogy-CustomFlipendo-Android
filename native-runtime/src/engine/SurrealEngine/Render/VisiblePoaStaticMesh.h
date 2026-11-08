#pragma once
#include "VisibleFrame.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Engine.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include <set>
// Initial UE2 material path: direct textures and Shader.Diffuse/Modifier.Material.
// Layered material operations, lightmaps and animated UVs remain separate work.
inline UTexture* PoaDiffuseTexture(UObject* material,uint32_t& flags,std::set<UObject*>& seen)
{
    if(!material||seen.size()>=32||!seen.insert(material).second) return nullptr;
    material->LoadNow();
    for(const auto& name:{"TwoSided","bTwoSided"})
        if(material->HasProperty(name)&&material->GetBool(name)) flags|=PF_TwoSided;
    if(material->HasProperty("bMasked")&&material->GetBool("bMasked")) flags|=PF_Masked;
    if(material->HasProperty("AlphaTest")&&material->GetBool("AlphaTest")) flags|=PF_Masked;
    if(material->HasProperty("bAlphaTexture")&&material->GetBool("bAlphaTexture")) flags|=PF_Highlighted;
    if(material->HasProperty("OutputBlending"))
    {
        auto blend=material->GetByte("OutputBlending");
        if(blend==1) flags|=PF_Masked;
        else if(blend==2) flags|=PF_Modulated;
        else if(blend==3) flags|=PF_Translucent;
        else if(blend>=4) return nullptr; // invisible/brighten/darken require separate material operations
    }
    if(material->HasProperty("FrameBufferBlending"))
    {
        auto blend=material->GetByte("FrameBufferBlending");
        if(blend==1) flags|=PF_Modulated;
        else if(blend==2) flags|=PF_Highlighted;
        else if(blend==4) flags|=PF_Translucent;
        else if(blend!=0) return nullptr;
    }
    if(auto texture=UObject::TryCast<UTexture>(material)) return texture;
    for(const auto& name:{"Diffuse","Material","FallbackMaterial"})
        if(material->HasProperty(name)) if(auto texture=PoaDiffuseTexture(material->GetUObject(name),flags,seen)) return texture;
    return nullptr;
}
inline bool DrawPoaStaticMesh(VisibleFrame* frame,UActor* actor,bool translucentPass)
{
    auto mesh=GetPoaStaticMesh(actor);if(!mesh) return false;
    const auto transform=PoaStaticMeshToWorld(actor);
    const auto axisX=(transform*vec4(1,0,0,0)).xyz();
    const auto axisY=(transform*vec4(0,1,0,0)).xyz();
    const auto axisZ=(transform*vec4(0,0,1,0)).xyz();
    const auto determinant=dot(cross(axisX,axisY),axisZ);
    if(std::abs(determinant)<1e-12f) return false;
    const bool reverseWinding=determinant<0;
    bool translucentPending=false;
    for(size_t sectionIndex=0;sectionIndex<mesh->Geometry.Sections.size();++sectionIndex)
    {
        const auto& section=mesh->Geometry.Sections[sectionIndex];
        uint32_t flags=0;
        std::set<UObject*> seen;
        UObject* material=mesh->SourcePackage->GetUObject(mesh->MaterialReferences[sectionIndex]);
        UTexture* texture=PoaDiffuseTexture(material,flags,seen);
        if(!texture) continue; // unsupported material remains explicit, not a fake texture
        if(actor->Style()==STY_Masked) flags|=PF_Masked;
        if(actor->Style()==STY_Translucent) flags|=PF_Translucent;
        if(actor->Style()==STY_Modulated) flags|=PF_Modulated;
        const bool translucent=(flags&(PF_Translucent|PF_Modulated|PF_Highlighted))!=0;
        if(translucent&&!translucentPass){translucentPending=true;continue;}
        if(!translucent&&translucentPass) continue;
        engine->render->UpdateTexture(texture);
        TextureInfo info;engine->render->UpdateTextureInfo(info,texture);
        for(size_t triangle=0;triangle<section.Triangles;++triangle)
        {
            GouraudVertex points[3];
            for(size_t corner=0;corner<3;++corner)
            {
                const auto orderedCorner=reverseWinding&&corner?3-corner:corner;
                const auto index=mesh->Geometry.Indices[section.FirstIndex+triangle*3+orderedCorner];
                const auto& vertex=mesh->Geometry.Vertices[index];
                points[corner].Point=(transform*vec4(vertex.Position[0],vertex.Position[1],vertex.Position[2],1.0f)).xyz();
                points[corner].UV=vec2(0.0f);
                if(!mesh->Geometry.UV.empty()&&!mesh->Geometry.UV[0].empty())
                {
                    const auto& uv=mesh->Geometry.UV[0][index];
                    points[corner].UV={uv[0]*info.USize,uv[1]*info.VSize};
                }
                points[corner].Light=vec3(1.0f);points[corner].Fog=vec4(0.0f);
                if(!mesh->Geometry.Colors.empty())
                {
                    auto color=mesh->Geometry.Colors[index];
                    points[corner].Light=vec3(float((color>>16)&255),float((color>>8)&255),float(color&255))/255.0f;
                }
            }
            frame->Device->DrawGouraudPolygon(&frame->Frame,info,points,3,flags);
        }
    }
    return translucentPending;
}
