#include "Precomp.h"
#include "UPoaStaticMesh.h"
#include "Package/PackageStream.h"
namespace
{
struct Tag { NameString Name; int Kind=0;uint32_t End=0;bool Bool=false; };
Tag ReadTag(ObjectStream* stream)
{
    Tag tag;tag.Name=stream->ReadName();
    if(tag.Name.IsNone()) return tag;
    auto info=stream->ReadUInt8();tag.Kind=info&15;tag.Bool=(info&128)!=0;
    if(tag.Kind<1||tag.Kind>15) Exception::Throw("Invalid PoA mesh property type");
    if(tag.Kind==10) stream->ReadName();
    uint32_t size=0;
    switch((info>>4)&7)
    {
    case 0:size=1;break;case 1:size=2;break;case 2:size=4;break;
    case 3:size=12;break;case 4:size=16;break;
    case 5:size=stream->ReadUInt8();break;case 6:size=stream->ReadUInt16();break;case 7:size=stream->ReadUInt32();break;
    }
    if(tag.Kind!=3&&tag.Bool)
    {
        auto first=stream->ReadUInt8();
        if((first&192)==192) stream->Skip(3);
        else if(first&128) stream->Skip(1);
    }
    if(tag.Kind==3) size=0;
    if(size>stream->Remaining()) Exception::Throw("PoA mesh tag exceeds object");
    tag.End=stream->Tell()+size;
    return tag;
}
}
void UPoaStaticMesh::Load(ObjectStream* stream)
{
    if(AllFlags(Flags,ObjectFlags::HasStack)) Exception::Throw("Unexpected stateful PoA static mesh");
    const auto start=stream->Tell();
    Array<int> materials;Array<uint8_t> collision;bool simpleBox=false;
    while(true)
    {
        auto tag=ReadTag(stream);if(tag.Name.IsNone()) break;
        if(tag.Name=="Materials"&&tag.Kind==9)
        {
            const auto count=PoaAssets::Count(*stream,1);
            for(size_t i=0;i<count;++i)
            {
                int material=0;uint8_t enabled=1;
                while(true)
                {
                    auto member=ReadTag(stream);if(member.Name.IsNone()) break;
                    if(member.End>tag.End) Exception::Throw("PoA material exceeds its array tag");
                    if(member.Name=="Material"&&member.Kind==5) material=stream->ReadIndex();
                    else if(member.Name=="EnableCollision"&&member.Kind==3) enabled=member.Bool;
                    if(stream->Tell()>member.End) Exception::Throw("PoA material property size mismatch");
                    stream->Seek(member.End);
                }
                if(stream->Tell()>tag.End) Exception::Throw("PoA material array overrun");
                materials.push_back(material);collision.push_back(enabled);
            }
            if(stream->Tell()!=tag.End) Exception::Throw("PoA material array size mismatch");
        }
        else if(tag.Name=="UseSimpleBoxCollision"&&tag.Kind==3) simpleBox=tag.Bool;
        stream->Seek(tag.End);
    }
    stream->Seek(start);
    UPrimitive::Load(stream);
    auto geometry=PoaAssets::ReadStaticMesh(*stream,stream->GetVersion());
    if(geometry.Sections.size()>materials.size()) Exception::Throw("PoA mesh has missing section materials");
    Geometry=std::move(geometry);MaterialReferences=std::move(materials);MaterialCollision=std::move(collision);
    UseSimpleBoxCollision=simpleBox;SourcePackage=stream->GetPackage();
    // UE2 collision/editor trailer has a separate format. Geometry traces use
    // validated section triangles; do not reinterpret the trailer as UE1 data.
    stream->Skip(uint32_t(stream->Remaining()));
}
void UPoaStaticMesh::Save(PackageStreamWriter*)
{
    Exception::Throw("PoA static-mesh package rewriting is not implemented");
}
