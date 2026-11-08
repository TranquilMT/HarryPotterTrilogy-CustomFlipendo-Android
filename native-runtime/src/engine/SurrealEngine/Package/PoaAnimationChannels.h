#pragma once
#include "PoaSkeletalMeshData.h"
#include <cctype>
namespace PoaChannels
{
constexpr size_t ChannelCount=16; // bounded port limit; not a universal UE2 limit
inline bool SameName(const std::string&a,const std::string&b)
{
    if(a.size()!=b.size())return false;
    for(size_t i=0;i<a.size();++i)if(std::tolower(static_cast<unsigned char>(a[i]))!=std::tolower(static_cast<unsigned char>(b[i])))return false;
    return true;
}
inline size_t Sequence(const PoaAssets::AnimationData& data,const std::string& name)
{
    for(size_t i=0;i<data.Sequences.size();++i)if(SameName(data.Sequences[i].Name,name))return i;
    throw std::runtime_error("PoA animation sequence not found: "+name);
}
struct Channel
{
    std::string Name,FromName,Bone;
    std::vector<PoaAssets::BoneTransform> FromPose;
    double Phase=0,FromPhase=0;
    float Rate=1,Alpha=0,TargetAlpha=0,AlphaTime=0,Tween=0,TweenTotal=0;
    float BlendIn=1,BlendOut=1;
    bool Playing=false,Loop=false,GlobalPose=false;
    uint64_t Generation=0;
};
struct State
{
    std::array<Channel,ChannelCount> Channels;
    int Waiting=-1;
    State(){Channels[0].Alpha=Channels[0].TargetAlpha=1;}
    Channel& At(int channel){if(channel<0||size_t(channel)>=Channels.size())throw std::runtime_error("PoA animation channel out of range");return Channels[channel];}
};
struct Event { int Channel;uint64_t Generation;bool End;std::string Function;int ObjectReference; };
inline std::vector<PoaAssets::BoneTransform> ChannelPose(const PoaAssets::SkeletalMeshData& mesh,const PoaAssets::AnimationData& data,const Channel& channel);
inline void Play(State& state,const PoaAssets::AnimationData& data,const std::string& name,float rate,float tween,int channel,bool loop,const PoaAssets::SkeletalMeshData* mesh=nullptr)
{
    const auto sequence=Sequence(data,name);
    if(sequence>=data.Moves.size()||data.Sequences[sequence].Frames<=0)throw std::runtime_error("Invalid PoA animation sequence");
    if(!std::isfinite(rate)||rate<0||!std::isfinite(tween)||tween<0)throw std::runtime_error("Invalid PoA animation playback parameters");
    auto& c=state.At(channel);auto snapshot=tween>0&&mesh?ChannelPose(*mesh,data,c):std::vector<PoaAssets::BoneTransform>();c.FromPose=std::move(snapshot);c.FromName=c.Name;c.FromPhase=c.Phase;c.Name=data.Sequences[sequence].Name;c.Phase=0;c.Rate=rate;c.Loop=loop;c.Playing=true;c.Tween=c.TweenTotal=tween;++c.Generation;
}
inline void Blend(State& state,int channel,float alpha,float seconds,const std::string& bone)
{
    if(!std::isfinite(alpha)||alpha<0||alpha>1||!std::isfinite(seconds)||seconds<0)throw std::runtime_error("Invalid PoA animation blend");
    auto& c=state.At(channel);c.TargetAlpha=alpha;c.AlphaTime=seconds;c.Bone=bone;if(seconds==0)c.Alpha=alpha;
}
inline std::vector<Event> Tick(State& state,const PoaAssets::AnimationData& data,float elapsed)
{
    if(!std::isfinite(elapsed)||elapsed<0)throw std::runtime_error("Invalid PoA animation elapsed time");
    std::vector<Event> events;
    for(size_t index=0;index<state.Channels.size();++index)
    {
        auto& c=state.Channels[index];
        if(c.AlphaTime>0){const float step=std::min(elapsed,c.AlphaTime);c.Alpha+=(c.TargetAlpha-c.Alpha)*step/c.AlphaTime;c.AlphaTime-=step;}
        float remaining=elapsed;
        if(c.Tween>0)
        {
            const float step=std::min(remaining,c.Tween);c.Tween-=step;remaining-=step;
            if(c.Tween==0&&c.Rate==0&&c.Playing){c.Playing=false;events.push_back({int(index),c.Generation,true,{},0});}
        }
        if(!c.Playing||c.Name.empty()||remaining<=0)continue;
        const auto sequence=Sequence(data,c.Name);const auto& seq=data.Sequences.at(sequence);
        const double speed=double(c.Rate)*seq.Rate/seq.Frames;
        if(speed<=0)continue;
        double delta=double(remaining)*speed;const double last=double(seq.Frames-1)/seq.Frames;
        size_t cycles=0;
        while(delta>0&&c.Playing)
        {
            if(++cycles>4096)throw std::runtime_error("PoA animation tick exceeds event-cycle budget");
            const double end=c.Loop?1.0:std::max(last,c.Phase);const double next=std::min(end,c.Phase+delta);
            for(const auto& notify:seq.Notifies)if(notify.Time>c.Phase&&notify.Time<=next)
                events.push_back({int(index),c.Generation,false,notify.Function,notify.ObjectReference});
            if((c.Phase<last&&next>=last)||(last==0&&c.Phase==0))events.push_back({int(index),c.Generation,true,{},0});
            delta-=std::max(0.0,next-c.Phase);c.Phase=next;
            if(next>=end){if(c.Loop)c.Phase=0;else c.Playing=false;}else break;
        }
    }
    return events;
}
inline void Stop(State& state,bool clearOthers)
{
    for(size_t i=0;i<state.Channels.size();++i){auto&c=state.Channels[i];c.Playing=false;c.Tween=0;++c.Generation;if(clearOthers&&i){const auto generation=c.Generation;c=Channel();c.Generation=generation;}}
    state.Waiting=-1;
}
inline std::vector<PoaAssets::BoneTransform> ChannelPose(const PoaAssets::SkeletalMeshData& mesh,const PoaAssets::AnimationData& data,const Channel& c)
{
    using namespace PoaAssets;if(c.Name.empty())return SkeletalLocalPose(mesh);
    const auto sequence=Sequence(data,c.Name);auto target=SkeletalLocalPose(mesh,&data,sequence,float(c.Phase*data.Moves.at(sequence).Duration),c.Loop);
        if(c.Tween>0&&c.TweenTotal>0)
        {
            auto from=c.FromPose.empty()?SkeletalLocalPose(mesh):c.FromPose;
            if(from.size()!=mesh.Bones.size())throw std::runtime_error("PoA interrupted tween skeleton changed; relink required");
            if(c.FromPose.empty()&&!c.FromName.empty()){auto previous=Sequence(data,c.FromName);from=SkeletalLocalPose(mesh,&data,previous,float(c.FromPhase*data.Moves.at(previous).Duration),false);}
            float fraction=1-c.Tween/c.TweenTotal;
            for(size_t i=0;i<target.size();++i){target[i].Rotation=InterpolateAnimationQuat(from[i].Rotation,target[i].Rotation,fraction);for(size_t axis=0;axis<3;++axis)target[i].Position[axis]=from[i].Position[axis]*(1-fraction)+target[i].Position[axis]*fraction;}
        }
    return target;
}
inline std::vector<PoaAssets::BoneTransform> Pose(const PoaAssets::SkeletalMeshData& mesh,const PoaAssets::AnimationData& data,const State& state)
{
    using namespace PoaAssets;auto local=SkeletalLocalPose(mesh);
    for(const auto& c:state.Channels)
    {
        if(c.Name.empty()||c.Alpha<=0)continue;
        auto target=ChannelPose(mesh,data,c);
        int root=-1;if(!c.Bone.empty()&&!SameName(c.Bone,"None"))
        {
            for(size_t i=0;i<mesh.Bones.size();++i)if(SameName(mesh.Bones[i].Name,c.Bone)){root=int(i);break;}
            if(root<0)throw std::runtime_error("PoA animation blend bone not found");
        }
        // Retail GetFrame tests bGlobalPose only at the blend root. Convert
        // that root's channel-global target into the current parent's space;
        // descendants continue with ordinary local-space blending.
        if(c.GlobalPose&&root>0)
        {
            const auto parent=mesh.Bones[root].Parent;
            if(parent>=0&&parent!=root)
            {
                const auto currentGlobal=ComposeSkeletalPose(mesh,local);const auto targetGlobal=ComposeSkeletalPose(mesh,target);
                auto inverse=currentGlobal.at(size_t(parent)).Rotation;for(size_t axis=0;axis<3;++axis)inverse[axis]=-inverse[axis];
                auto position=targetGlobal[size_t(root)].Position;for(size_t axis=0;axis<3;++axis)position[axis]-=currentGlobal[size_t(parent)].Position[axis];
                target[size_t(root)].Position=RotateBoneVector(inverse,position);
                target[size_t(root)].Rotation=MultiplyBoneQuats(inverse,targetGlobal[size_t(root)].Rotation);
            }
        }
        for(size_t i=0;i<local.size();++i)
        {
            bool affected=root<0;int bone=int(i);
            for(size_t steps=0;!affected&&bone>=0&&steps<mesh.Bones.size();++steps){if(size_t(bone)>=mesh.Bones.size())throw std::runtime_error("Invalid PoA blend skeleton");if(bone==root){affected=true;break;}auto parent=mesh.Bones[bone].Parent;if(parent==bone)break;bone=parent;}
            if(!affected)continue;
            local[i].Rotation=InterpolateAnimationQuat(local[i].Rotation,target[i].Rotation,c.Alpha);
            for(size_t axis=0;axis<3;++axis)local[i].Position[axis]=local[i].Position[axis]*(1-c.Alpha)+target[i].Position[axis]*c.Alpha;
        }
    }
    return ComposeSkeletalPose(mesh,local);
}
}
