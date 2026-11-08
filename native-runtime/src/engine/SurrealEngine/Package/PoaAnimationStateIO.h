#pragma once
#include "PoaAnimationChannels.h"
#include <bit>
namespace PoaChannels
{
// Port-owned actor trailer. This is not the original PC skeletal-instance
// serialization and does not claim PC save-game interchange.
constexpr uint32_t StateMagic=0x43335048,StateTag=0x534d4941,StateVersion=1;
inline void Validate(const State& state)
{
    if(state.Waiting < -1 || state.Waiting>=int(ChannelCount))throw std::runtime_error("Invalid saved PoA animation wait channel");
    for(const auto& c:state.Channels)
    {
        if(!std::isfinite(c.BlendIn)||c.BlendIn<1||!std::isfinite(c.BlendOut)||c.BlendOut<1)throw std::runtime_error("Invalid saved PoA blend coefficient");
        if(c.FromPose.size()>4096)throw std::runtime_error("Saved PoA tween skeleton too large");
        for(const auto& transform:c.FromPose)
        {
            double norm=0;for(auto v:transform.Rotation){if(!std::isfinite(v))throw std::runtime_error("Invalid saved PoA tween quaternion");norm+=double(v)*v;}
            if(std::abs(norm-1)>1e-3)throw std::runtime_error("Non-unit saved PoA tween quaternion");
            for(auto v:transform.Position)if(!std::isfinite(v))throw std::runtime_error("Invalid saved PoA tween position");
        }
        for(const auto* s:{&c.Name,&c.FromName,&c.Bone})if(s->size()>4096)throw std::runtime_error("Saved PoA animation name too long");
        if(!std::isfinite(c.Phase)||!std::isfinite(c.FromPhase)||!std::isfinite(c.Rate)||c.Rate<0)throw std::runtime_error("Invalid saved PoA animation timing");
        if(!std::isfinite(c.Alpha)||c.Alpha<0||c.Alpha>1||!std::isfinite(c.TargetAlpha)||c.TargetAlpha<0||c.TargetAlpha>1)throw std::runtime_error("Invalid saved PoA animation alpha");
        if(!std::isfinite(c.AlphaTime)||c.AlphaTime<0||!std::isfinite(c.Tween)||c.Tween<0||!std::isfinite(c.TweenTotal)||c.TweenTotal<0||c.Tween>c.TweenTotal)throw std::runtime_error("Invalid saved PoA animation transition");
    }
}
template<class Writer> void WriteState(Writer& writer,const State& state)
{
    Validate(state);writer.WriteUInt32(uint32_t(ChannelCount));writer.WriteInt32(state.Waiting);
    for(const auto& c:state.Channels)
    {
        for(const auto* s:{&c.Name,&c.FromName,&c.Bone})writer.WriteString(*s);
        writer.WriteUInt64(std::bit_cast<uint64_t>(c.Phase));writer.WriteUInt64(std::bit_cast<uint64_t>(c.FromPhase));
        for(auto v:{c.Rate,c.Alpha,c.TargetAlpha,c.AlphaTime,c.Tween,c.TweenTotal,c.BlendIn,c.BlendOut})writer.WriteFloat(v);
        writer.WriteUInt32((c.Playing?1u:0u)|(c.Loop?2u:0u)|(c.GlobalPose?4u:0u));writer.WriteUInt32(uint32_t(c.FromPose.size()));
        for(const auto& transform:c.FromPose){for(auto v:transform.Rotation)writer.WriteFloat(v);for(auto v:transform.Position)writer.WriteFloat(v);}
    }
}
template<class Reader> State ReadState(Reader& reader)
{
    if(reader.ReadUInt32()!=ChannelCount)throw std::runtime_error("Unsupported saved PoA animation channel count");
    State state;state.Waiting=reader.ReadInt32();
    for(auto& c:state.Channels)
    {
        c.Name=reader.ReadString();c.FromName=reader.ReadString();c.Bone=reader.ReadString();
        c.Phase=std::bit_cast<double>(reader.ReadUInt64());c.FromPhase=std::bit_cast<double>(reader.ReadUInt64());
        c.Rate=reader.ReadFloat();c.Alpha=reader.ReadFloat();c.TargetAlpha=reader.ReadFloat();c.AlphaTime=reader.ReadFloat();c.Tween=reader.ReadFloat();c.TweenTotal=reader.ReadFloat();c.BlendIn=reader.ReadFloat();c.BlendOut=reader.ReadFloat();
        const auto flags=reader.ReadUInt32();if(flags&~7u)throw std::runtime_error("Unknown saved PoA animation flags");c.Playing=(flags&1u)!=0;c.Loop=(flags&2u)!=0;c.GlobalPose=(flags&4u)!=0;
        const auto count=reader.ReadUInt32();if(count>4096)throw std::runtime_error("Saved PoA tween skeleton too large");c.FromPose.resize(count);
        for(auto& transform:c.FromPose){for(auto& v:transform.Rotation)v=reader.ReadFloat();for(auto& v:transform.Position)v=reader.ReadFloat();}
    }
    Validate(state);return state;
}
}
