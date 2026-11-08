#pragma once
#include "PoaStaticMeshData.h"
#include <string>
#include <algorithm>
namespace PoaAssets
{
using Quat=std::array<float,4>;
struct AnimationBone { std::string Name;uint32_t Flags;int32_t Parent; };
struct AnimationTrack { uint32_t Flags=0;std::vector<Quat> Rotation;std::vector<Vector> Position;std::vector<float> Time; };
struct AnimationMove { Vector RootSpeed;float Duration;int32_t StartBone;uint32_t Flags;std::vector<int32_t> BoneIndices;std::vector<AnimationTrack> Tracks;AnimationTrack Root; };
struct AnimationNotify { float Time;std::string Function;int32_t ObjectReference; };
struct AnimationSequence { float Tweak;std::string Name;std::vector<std::string> Groups;int32_t StartFrame,Frames;std::vector<AnimationNotify> Notifies;float Rate; };
struct AnimationData { std::vector<AnimationBone> Bones;std::vector<AnimationMove> Moves;std::vector<AnimationSequence> Sequences; };
template<class Reader> AnimationTrack ReadAnimationTrack(Reader& r)
{
    AnimationTrack track;track.Flags=r.ReadUInt32();
    auto n=Count(r,16);track.Rotation.reserve(n);
    for(size_t i=0;i<n;++i) track.Rotation.push_back({Float(r),Float(r),Float(r),Float(r)});
    n=Count(r,12);track.Position.reserve(n);
    for(size_t i=0;i<n;++i)
    {
        Vector position={r.ReadFloat(),r.ReadFloat(),r.ReadFloat()};
        const bool finite=std::all_of(position.begin(),position.end(),[](float v){return std::isfinite(v);});
        const bool absent=std::all_of(position.begin(),position.end(),[](float v){return std::isnan(v);});
        // Two retail low-memory Hermione resources contain an all-NaN
        // constant position key. Preserve it as an unavailable channel; the
        // sampler leaves that bone translation to the caller's bind pose.
        if(!finite&&!absent) throw std::runtime_error("Invalid PoA animation position key");
        track.Position.push_back(position);
    }
    n=Count(r,4);track.Time.reserve(n);
    for(size_t i=0;i<n;++i)
    {
        float time=Float(r);
        if(time<0||(!track.Time.empty()&&time<track.Time.back())) throw std::runtime_error("Invalid PoA animation key time");
        track.Time.push_back(time);
    }
    if(!track.Time.empty()&&((track.Rotation.size()>1&&track.Rotation.size()!=n)||(track.Position.size()>1&&track.Position.size()!=n)))
        throw std::runtime_error("PoA animation key counts disagree");
    return track;
}
template<class Reader> AnimationData ReadAnimation(Reader& r,int packageVersion)
{
    if(packageVersion!=129||r.ReadInt32()!=4) throw std::runtime_error("Unverified PoA animation version");
    AnimationData out;const auto bones=Count(r,9);out.Bones.reserve(bones);
    for(size_t i=0;i<bones;++i)
    {
        AnimationBone bone;bone.Name=r.ReadName().ToString();bone.Flags=r.ReadUInt32();bone.Parent=r.ReadInt32();
        if(bone.Parent < -1 || (bone.Parent>=0&&size_t(bone.Parent)>=bones)) throw std::runtime_error("PoA animation parent outside skeleton");
        out.Bones.push_back(std::move(bone));
    }
    const auto moves=Count(r,34);out.Moves.reserve(moves);
    for(size_t i=0;i<moves;++i)
    {
        AnimationMove move;move.RootSpeed=ReadVector(r);move.Duration=Float(r);move.StartBone=r.ReadInt32();move.Flags=r.ReadUInt32();
        if(move.Duration<0) throw std::runtime_error("Negative PoA animation duration");
        auto n=Count(r,4);move.BoneIndices.reserve(n);
        for(size_t j=0;j<n;++j) move.BoneIndices.push_back(r.ReadInt32());
        n=Count(r,7);move.Tracks.reserve(n);
        for(size_t j=0;j<n;++j) move.Tracks.push_back(ReadAnimationTrack(r));
        move.Root=ReadAnimationTrack(r);
        if(move.Flags>=3)
        {
            // All 140 original EU resources use ordinary analog tracks and an
            // empty flex-track array. Other UE2 compression is not guessed.
            if(r.ReadIndex()!=0) throw std::runtime_error("Unsupported PoA flex animation tracks");
        }
        for(auto index:move.BoneIndices) if(index < -1 || (index>=0&&size_t(index)>=move.Tracks.size())) throw std::runtime_error("PoA bone-to-track index out of range");
        out.Moves.push_back(std::move(move));
    }
    const auto sequences=Count(r,19);out.Sequences.reserve(sequences);
    for(size_t i=0;i<sequences;++i)
    {
        AnimationSequence sequence;sequence.Tweak=Float(r);sequence.Name=r.ReadName().ToString();
        auto n=Count(r,1);sequence.Groups.reserve(n);
        for(size_t j=0;j<n;++j) sequence.Groups.push_back(r.ReadName().ToString());
        sequence.StartFrame=r.ReadInt32();sequence.Frames=r.ReadInt32();
        if(sequence.StartFrame<0||sequence.Frames<0) throw std::runtime_error("Invalid PoA animation sequence frames");
        n=Count(r,6);sequence.Notifies.reserve(n);
        for(size_t j=0;j<n;++j)
        {
            AnimationNotify notify;notify.Time=Float(r);notify.Function=r.ReadName().ToString();notify.ObjectReference=r.ReadIndex();sequence.Notifies.push_back(std::move(notify));
        }
        sequence.Rate=Float(r);out.Sequences.push_back(std::move(sequence));
    }
    if(out.Sequences.size()!=out.Moves.size()) throw std::runtime_error("PoA move/sequence counts disagree");
    if(r.Remaining()!=0) throw std::runtime_error("Unparsed PoA animation bytes");
    return out;
}
struct BoneSample { bool HasRotation=false,HasPosition=false;Quat Rotation={0,0,0,1};Vector Position={0,0,0}; };
inline Quat UnitAnimationQuat(Quat q)
{
    double norm=0;for(auto x:q) norm+=double(x)*x;
    if(!std::isfinite(norm)) throw std::runtime_error("Invalid PoA animation quaternion");
    // EU low-memory character packages contain constant zero quaternion keys.
    // The retail FQuat-to-coordinate formula maps zero to the identity basis.
    if(norm==0) return {0,0,0,1};
    const double scale=1/std::sqrt(norm);for(auto&x:q)x=float(double(x)*scale);return q;
}
inline Quat InterpolateAnimationQuat(Quat a,Quat b,float fraction)
{
    a=UnitAnimationQuat(a);b=UnitAnimationQuat(b);double dot=0;for(size_t i=0;i<4;++i) dot+=double(a[i])*b[i];
    if(dot<0){for(auto&x:b)x=-x;dot=-dot;}
    dot=std::clamp(dot,0.0,1.0);double wa=1-fraction,wb=fraction;
    if(dot<.9995){double angle=std::acos(dot),sine=std::sin(angle);wa=std::sin((1-fraction)*angle)/sine;wb=std::sin(fraction*angle)/sine;}
    Quat q;for(size_t i=0;i<4;++i)q[i]=float(wa*a[i]+wb*b[i]);return UnitAnimationQuat(q);
}
struct AnimationKeyPair { size_t A=0,B=0;float Fraction=0; };
inline AnimationKeyPair AnimationKeys(const AnimationTrack& track,size_t count,float frame,float duration,bool loop)
{
    if(count<=1) return {};
    auto time=[&](size_t i){return track.Time.empty()?float(double(i)*duration/count):track.Time.at(i);};
    if(frame<=time(0)) return {};
    for(size_t i=1;i<count;++i) if(frame<time(i))
    {
        const auto span=time(i)-time(i-1);return {i-1,i,span>0?(frame-time(i-1))/span:0};
    }
    if(loop&&duration>time(count-1)) return {count-1,0,(frame-time(count-1))/(duration-time(count-1))};
    return {count-1,count-1,0};
}
inline BoneSample SampleAnimationTrack(const AnimationTrack& track,float frame,float duration,bool loop)
{
    if(!std::isfinite(frame)||!std::isfinite(duration)||duration<0) throw std::runtime_error("Invalid PoA animation sample time");
    if(loop&&duration>0){frame=std::fmod(frame,duration);if(frame<0)frame+=duration;}else frame=std::max(frame,0.0f);
    BoneSample sample;
    if(!track.Rotation.empty())
    {
        const auto key=AnimationKeys(track,track.Rotation.size(),frame,duration,loop);
        sample.Rotation=InterpolateAnimationQuat(track.Rotation[key.A],track.Rotation[key.B],key.Fraction);sample.HasRotation=true;
    }
    if(!track.Position.empty())
    {
        const auto key=AnimationKeys(track,track.Position.size(),frame,duration,loop);
        for(size_t i=0;i<3;++i) if(!std::isfinite(track.Position[key.A][i])||!std::isfinite(track.Position[key.B][i])) return sample;
        for(size_t i=0;i<3;++i) sample.Position[i]=float(double(track.Position[key.A][i])*(1-key.Fraction)+double(track.Position[key.B][i])*key.Fraction);
        sample.HasPosition=true;
    }
    return sample;
}
inline BoneSample SampleAnimationBone(const AnimationData& animation,size_t sequence,size_t bone,float frame,bool loop)
{
    if(sequence>=animation.Moves.size()||bone>=animation.Bones.size()) throw std::runtime_error("PoA animation sample index out of range");
    const auto& move=animation.Moves[sequence];
    const auto index=move.BoneIndices.empty()?(bone<move.Tracks.size()?int32_t(bone):-1):(bone<move.BoneIndices.size()?move.BoneIndices[bone]:-1);
    if(index<0) return {};
    if(size_t(index)>=move.Tracks.size()) throw std::runtime_error("PoA animation track index out of range");
    return SampleAnimationTrack(move.Tracks[index],frame,move.Duration,loop);
}
}
