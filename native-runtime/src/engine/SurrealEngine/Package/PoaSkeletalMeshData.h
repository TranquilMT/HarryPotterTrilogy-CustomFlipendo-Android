#pragma once
#include "PoaAnimationData.h"
namespace PoaAssets
{
struct SkeletalBone { std::string Name;uint32_t Flags;Quat Orientation;Vector Position;float Length;Vector Size;uint32_t Children;int32_t Parent; };
struct SkeletalWedge { uint16_t Point;std::array<float,2> UV; };
struct SkeletalTriangle { std::array<uint16_t,3> Wedges;uint8_t Material,AuxMaterial;uint32_t Smoothing; };
struct SkeletalInfluence { float Weight;uint16_t Point,Bone; };
struct SkeletalSocket { std::string Alias,Bone;std::array<Vector,4> Coordinates; };
struct SkeletalMeshData
{
    std::vector<int> Materials;
    struct MaterialSlot { uint32_t Flags;int32_t TextureIndex; };
    std::vector<MaterialSlot> MaterialSlots;
    Vector Scale,Origin;std::array<int32_t,3> Rotation;
    std::vector<SkeletalBone> Bones;
    std::vector<Vector> Points;
    std::vector<SkeletalWedge> Wedges;
    std::vector<SkeletalTriangle> Triangles;
    std::vector<SkeletalInfluence> Influences;
    std::vector<SkeletalSocket> Sockets;
    int AnimationReference=0;
    size_t LODCount=0;
};
// Version-129 EU meshes use absolute end offsets on lazy arrays. Decode every
// array in bounds, and check its endpoint against the containing package.
template<class Reader,class Read> void ReadSkeletalLazy(Reader& r,size_t width,Read read)
{
    const int32_t end=r.ReadInt32();const auto n=Count(r,width);
    for(size_t i=0;i<n;++i) read();
    if(end<0||size_t(end)!=r.Tell()) throw std::runtime_error("PoA skeletal lazy endpoint mismatch");
}
template<class Reader> void SkipSkeletalArray(Reader& r,size_t width)
{
    const auto n=Count(r,width);for(size_t i=0;i<n;++i)for(size_t b=0;b<width;++b)r.ReadUInt8();
}
template<class Reader> SkeletalMeshData ReadSkeletalMesh(Reader& r,int version)
{
    if(version!=129||r.ReadInt32()!=4) throw std::runtime_error("Unverified PoA skeletal mesh version");
    SkeletalMeshData mesh;
    if(r.ReadInt32()<0) throw std::runtime_error("Negative PoA skeletal vertex count");
    SkipSkeletalArray(r,4); // obsolete packed vertices
    auto n=Count(r,1);for(size_t i=0;i<n;++i)mesh.Materials.push_back(r.ReadIndex());
    mesh.Scale=ReadVector(r);mesh.Origin=ReadVector(r);for(auto&v:mesh.Rotation)v=r.ReadInt32();
    for(size_t width:{2,8,2,10})SkipSkeletalArray(r,width);
    n=Count(r,8);for(size_t i=0;i<n;++i)mesh.MaterialSlots.push_back({r.ReadUInt32(),r.ReadInt32()}); // legacy LOD tables
    for(size_t i=0;i<3;++i)Float(r);r.ReadInt32();Float(r);Float(r);
    r.ReadInt32();r.ReadIndex(); // impostor flags/material
    ReadVector(r);for(size_t i=0;i<3;++i)r.ReadInt32();ReadVector(r);r.ReadUInt32();
    for(size_t i=0;i<3;++i)r.ReadInt32();Float(r); // impostor modes, tessellation
    SkipSkeletalArray(r,12); // obsolete points
    n=Count(r,57);mesh.Bones.reserve(n);
    for(size_t i=0;i<n;++i)
    {
        SkeletalBone b;b.Name=r.ReadName().ToString();b.Flags=r.ReadUInt32();
        for(auto&v:b.Orientation)v=Float(r);
        b.Position=ReadVector(r);b.Length=Float(r);b.Size=ReadVector(r);b.Children=r.ReadUInt32();b.Parent=r.ReadInt32();
        if(b.Parent < -1 || (b.Parent>=0&&size_t(b.Parent)>=n) || (i&&b.Parent==int32_t(i)))throw std::runtime_error("Invalid PoA skeletal parent");
        mesh.Bones.push_back(std::move(b));
    }
    mesh.AnimationReference=r.ReadIndex();r.ReadInt32(); // depth
    n=Count(r,5);for(size_t i=0;i<n;++i){SkipSkeletalArray(r,2);r.ReadInt32();}
    SkipSkeletalArray(r,4); // old bone influences
    std::vector<std::string> aliases,boneNames;
    for(auto* names:{&aliases,&boneNames}){n=Count(r,1);for(size_t i=0;i<n;++i)names->push_back(r.ReadName().ToString());}
    n=Count(r,48);
    if(n!=aliases.size()||n!=boneNames.size()) throw std::runtime_error("PoA socket arrays disagree");
    for(size_t i=0;i<n;++i){SkeletalSocket socket;socket.Alias=aliases[i];socket.Bone=boneNames[i];for(auto&v:socket.Coordinates)v=ReadVector(r);mesh.Sockets.push_back(std::move(socket));}
    mesh.LODCount=Count(r,60);
    for(size_t lod=0;lod<mesh.LODCount;++lod)
    {
        SkipSkeletalArray(r,4);SkipSkeletalArray(r,16);r.ReadInt32();
        SkipSkeletalArray(r,18);SkipSkeletalArray(r,18);
        for(size_t k=0;k<2;++k){SkipSkeletalArray(r,2);r.ReadInt32();}
        for(size_t k=0;k<3;++k)r.ReadInt32();SkipSkeletalArray(r,32);
        for(size_t width:{8,10,8,12})ReadSkeletalLazy(r,width,[&](){for(size_t b=0;b<width;++b)r.ReadUInt8();});
        Float(r);Float(r);for(size_t k=0;k<4;++k)r.ReadInt32();
    }
    r.ReadIndex(); // old model reference
    ReadSkeletalLazy(r,12,[&](){mesh.Points.push_back(ReadVector(r));});
    ReadSkeletalLazy(r,10,[&](){SkeletalWedge w;w.Point=r.ReadUInt16();w.UV={Float(r),Float(r)};mesh.Wedges.push_back(w);});
    ReadSkeletalLazy(r,12,[&](){SkeletalTriangle t;for(auto&v:t.Wedges)v=r.ReadUInt16();t.Material=r.ReadUInt8();t.AuxMaterial=r.ReadUInt8();t.Smoothing=r.ReadUInt32();mesh.Triangles.push_back(t);});
    ReadSkeletalLazy(r,8,[&](){SkeletalInfluence influence;influence.Weight=Float(r);influence.Point=r.ReadUInt16();influence.Bone=r.ReadUInt16();mesh.Influences.push_back(influence);});
    for(size_t k=0;k<2;++k)ReadSkeletalLazy(r,2,[&](){r.ReadUInt16();});
    r.ReadUInt32();r.ReadIndex(); // authentication and Karma object
    for(size_t width:{16,24}){n=Count(r,width+1);for(size_t i=0;i<n;++i){r.ReadName();for(size_t b=0;b<width/4;++b)Float(r);}}
    n=Count(r,1);for(size_t i=0;i<n;++i)r.ReadIndex();
    if(r.Remaining())throw std::runtime_error("Unparsed PoA skeletal mesh bytes");
    for(const auto&w:mesh.Wedges)if(w.Point>=mesh.Points.size())throw std::runtime_error("PoA skeletal wedge point out of range");
    for(const auto&t:mesh.Triangles){for(auto w:t.Wedges)if(w>=mesh.Wedges.size())throw std::runtime_error("PoA skeletal face wedge out of range");if(t.Material>=mesh.MaterialSlots.size())throw std::runtime_error("PoA skeletal material out of range");}
    for(const auto&i:mesh.Influences)if(i.Point>=mesh.Points.size()||i.Bone>=mesh.Bones.size()||i.Weight<0)throw std::runtime_error("Invalid PoA skeletal influence");
    return mesh;
}
struct BoneTransform { Quat Rotation={0,0,0,1};Vector Position={0,0,0}; };
inline Vector RotateBoneVector(Quat q,Vector v)
{
    q=UnitAnimationQuat(q);Vector cross={q[1]*v[2]-q[2]*v[1],q[2]*v[0]-q[0]*v[2],q[0]*v[1]-q[1]*v[0]};
    Vector second={q[1]*cross[2]-q[2]*cross[1],q[2]*cross[0]-q[0]*cross[2],q[0]*cross[1]-q[1]*cross[0]};
    for(size_t i=0;i<3;++i)v[i]+=2*(q[3]*cross[i]+second[i]);return v;
}
inline Quat MultiplyBoneQuats(Quat a,Quat b)
{
    return UnitAnimationQuat({a[3]*b[0]+a[0]*b[3]+a[1]*b[2]-a[2]*b[1],a[3]*b[1]-a[0]*b[2]+a[1]*b[3]+a[2]*b[0],a[3]*b[2]+a[0]*b[1]-a[1]*b[0]+a[2]*b[3],a[3]*b[3]-a[0]*b[0]-a[1]*b[1]-a[2]*b[2]});
}
inline BoneTransform ComposeBones(const BoneTransform& parent,const BoneTransform& child)
{
    BoneTransform result;result.Rotation=MultiplyBoneQuats(parent.Rotation,child.Rotation);result.Position=RotateBoneVector(parent.Rotation,child.Position);for(size_t i=0;i<3;++i)result.Position[i]+=parent.Position[i];return result;
}
inline std::vector<BoneTransform> SkeletalLocalPose(const SkeletalMeshData& mesh,const AnimationData* animation=nullptr,size_t sequence=0,float frame=0,bool loop=true)
{
    std::vector<BoneTransform> local;local.reserve(mesh.Bones.size());
    for(const auto& bone:mesh.Bones)
    {
        BoneTransform transform;transform.Rotation=UnitAnimationQuat(bone.Orientation);transform.Position=bone.Position;
        if(animation)
        {
            auto found=std::find_if(animation->Bones.begin(),animation->Bones.end(),[&](const auto&b){return b.Name==bone.Name;});
            if(found!=animation->Bones.end())
            {
                auto sample=SampleAnimationBone(*animation,sequence,size_t(found-animation->Bones.begin()),frame,loop);
                if(sample.HasRotation)transform.Rotation=sample.Rotation;if(sample.HasPosition)transform.Position=sample.Position;
            }
        }
        // UEViewer's reference builds world-to-local axes and conjugates the
        // root before parent UnTransformCoords. With active local-to-world
        // rotations here, that is equivalent to conjugating child bones.
        if(!local.empty())for(size_t c=0;c<3;++c)transform.Rotation[c]=-transform.Rotation[c];
        local.push_back(transform);
    }
    return local;
}
inline std::vector<BoneTransform> ComposeSkeletalPose(const SkeletalMeshData& mesh,const std::vector<BoneTransform>& local)
{
    if(local.size()!=mesh.Bones.size())throw std::runtime_error("PoA local pose size mismatch");
    std::vector<BoneTransform> global(local.size());std::vector<uint8_t> state(local.size(),0);
    for(size_t i=0;i<local.size();++i)
    {
        std::vector<size_t> path;size_t index=i;
        while(state[index]!=2)
        {
            if(state[index]==1)throw std::runtime_error("Cyclic PoA skeleton");
            state[index]=1;path.push_back(index);const auto parent=mesh.Bones[index].Parent;
            if(parent<0||size_t(parent)==index)
            {
                if(index!=0&&parent>=0)throw std::runtime_error("Invalid PoA skeletal root");
                global[index]=local[index];state[index]=2;path.pop_back();break;
            }
            if(size_t(parent)>=local.size())throw std::runtime_error("Invalid PoA skeletal parent");
            index=size_t(parent);
        }
        while(!path.empty())
        {
            index=path.back();path.pop_back();const auto parent=mesh.Bones[index].Parent;
            global[index]=ComposeBones(global[parent],local[index]);state[index]=2;
        }
    }
    return global;
}
inline std::vector<BoneTransform> SkeletalPose(const SkeletalMeshData& mesh,const AnimationData* animation=nullptr,size_t sequence=0,float frame=0,bool loop=true)
{
    return ComposeSkeletalPose(mesh,SkeletalLocalPose(mesh,animation,sequence,frame,loop));
}
inline std::vector<Vector> SkinSkeletalPoints(const SkeletalMeshData& mesh,const std::vector<BoneTransform>& pose)
{
    if(pose.size()!=mesh.Bones.size())throw std::runtime_error("PoA skin pose count mismatch");
    auto bind=SkeletalPose(mesh);std::vector<std::array<double,3>> sum(mesh.Points.size(),{0,0,0});std::vector<double> weights(mesh.Points.size(),0);
    for(const auto&i:mesh.Influences)
    {
        if(i.Point>=mesh.Points.size()||i.Bone>=bind.size()||!std::isfinite(i.Weight)||i.Weight<0)throw std::runtime_error("Invalid PoA skin weight");
        if(i.Weight==0)continue;Vector local=mesh.Points[i.Point];for(size_t c=0;c<3;++c)local[c]-=bind[i.Bone].Position[c];
        auto inverse=bind[i.Bone].Rotation;for(size_t c=0;c<3;++c)inverse[c]=-inverse[c];local=RotateBoneVector(inverse,local);auto transformed=RotateBoneVector(pose[i.Bone].Rotation,local);
        for(size_t c=0;c<3;++c)sum[i.Point][c]+=double(i.Weight)*(double(transformed[c])+pose[i.Bone].Position[c]);weights[i.Point]+=i.Weight;
    }
    auto points=mesh.Points;
    for(size_t p=0;p<points.size();++p)if(weights[p]>0)for(size_t c=0;c<3;++c){points[p][c]=float(sum[p][c]/weights[p]);if(!std::isfinite(points[p][c]))throw std::runtime_error("Non-finite PoA skinned point");}
    return points;
}
}
