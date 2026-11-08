#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <cmath>
#include <stdexcept>

namespace PoaAssets
{
using Vector = std::array<float,3>;
struct Section { uint32_t Flags; uint16_t FirstIndex,FirstVertex,LastVertex,Unknown,Triangles; };
struct Vertex { Vector Position,Normal; };
struct StaticMeshData
{
    std::vector<Section> Sections;
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Colors,Alpha;
    std::vector<std::vector<std::array<float,2>>> UV;
    std::vector<uint16_t> Indices,WireIndices;
    int CollisionReference=0;
    size_t NativeBytes=0;
};
template<class Reader> size_t Count(Reader& r,size_t minimumBytes)
{
    const int n=r.ReadIndex();
    if(n<0 || size_t(n)>r.Remaining()/minimumBytes) throw std::runtime_error("PoA asset array exceeds object bounds");
    return size_t(n);
}
template<class Reader> float Float(Reader& r)
{
    float v=r.ReadFloat();
    if(!std::isfinite(v)) throw std::runtime_error("Non-finite PoA asset coordinate");
    return v;
}
template<class Reader> Vector ReadVector(Reader& r) { return {Float(r),Float(r),Float(r)}; }
// Reader begins after UPrimitive's box and sphere. EU versions 128/129 were
// checked against every StaticMesh export in the published EU payload.
template<class Reader> StaticMeshData ReadStaticMesh(Reader& r,int version)
{
    if(version!=128&&version!=129) throw std::runtime_error("Unverified PoA static mesh version");
    const auto start=r.Remaining();StaticMeshData out;
    const auto sections=Count(r,14);out.Sections.reserve(sections);
    for(size_t i=0;i<sections;++i)
        out.Sections.push_back({r.ReadUInt32(),r.ReadUInt16(),r.ReadUInt16(),r.ReadUInt16(),r.ReadUInt16(),r.ReadUInt16()});
    ReadVector(r);ReadVector(r);r.ReadUInt8(); // second serialized bounding box
    const auto vertices=Count(r,24);out.Vertices.reserve(vertices);
    for(size_t i=0;i<vertices;++i) out.Vertices.push_back({ReadVector(r),ReadVector(r)});
    r.ReadUInt32(); // revision
    for(auto colors:{&out.Colors,&out.Alpha})
    {
        const auto n=Count(r,4);colors->reserve(n);
        for(size_t i=0;i<n;++i) colors->push_back(r.ReadUInt32());
        r.ReadUInt32();
        if(n&&n!=vertices) throw std::runtime_error("PoA color/vertex count mismatch");
    }
    const auto channels=Count(r,9);
    if(channels>8) throw std::runtime_error("Excessive PoA UV channels");
    for(size_t channel=0;channel<channels;++channel)
    {
        const auto n=Count(r,8);std::vector<std::array<float,2>> uv;uv.reserve(n);
        for(size_t i=0;i<n;++i) uv.push_back({Float(r),Float(r)});
        r.ReadUInt32();r.ReadUInt32();
        if(n&&n!=vertices) throw std::runtime_error("PoA UV/vertex count mismatch");
        out.UV.push_back(std::move(uv));
    }
    for(auto indices:{&out.Indices,&out.WireIndices})
    {
        const auto n=Count(r,2);indices->reserve(n);
        for(size_t i=0;i<n;++i) indices->push_back(r.ReadUInt16());
        r.ReadUInt32();
    }
    out.CollisionReference=r.ReadIndex();
    for(auto index:out.Indices) if(index>=vertices) throw std::runtime_error("PoA mesh vertex index out of bounds");
    for(const auto& section:out.Sections)
    {
        if(size_t(section.FirstIndex)+size_t(section.Triangles)*3>out.Indices.size()) throw std::runtime_error("PoA mesh section exceeds index stream");
        if(section.Triangles&&(section.FirstVertex>section.LastVertex||section.LastVertex>=vertices)) throw std::runtime_error("PoA section vertex range invalid");
    }
    out.NativeBytes=start-r.Remaining();
    return out;
}
}
