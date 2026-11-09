#include "Precomp.h"
#include "PoaActorAnimation.h"
#include "PoaNotifies.h"
#include "Package/PoaAnimationChannels.h"
#include "Package/PoaAnimationStateIO.h"
#include "Package/ObjectStream.h"
#include "Package/PackageStream.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/Mesh/UPoaSkeletalMesh.h"
#include "Packages/Engine/Resources/Mesh/UPoaMeshAnimation.h"
#include "Package/Package.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "VM/ScriptCall.h"
namespace PoaActorAnimation
{
static PoaChannels::State& State(UActor* actor)
{
    if(!actor->PoaAnimation)actor->PoaAnimation=std::make_shared<PoaChannels::State>();
    return *actor->PoaAnimation;
}
static int32_t ChannelIndex(UActor* actor,std::optional<int32_t> channel)
{
    return channel?*channel:(actor->HasProperty("DefaultAnimChannel")?actor->GetInt("DefaultAnimChannel"):0);
}
static UPoaSkeletalMesh* Mesh(UActor* actor)
{
    auto object=actor->Mesh();if(object)object->LoadNow();return UObject::TryCast<UPoaSkeletalMesh>(object);
}
static UPoaMeshAnimation* Resource(UActor* actor)
{
    if(actor->PoaAnimationObject){actor->PoaAnimationObject->LoadNow();return UObject::Cast<UPoaMeshAnimation>(actor->PoaAnimationObject);}
    auto mesh=Mesh(actor);return mesh?mesh->AnimationResource:nullptr;
}
static UPoaMeshAnimation* RequireResource(UActor* actor)
{
    auto resource=Resource(actor);if(!resource)Exception::Throw("PoA actor has no skeletal animation resource");return resource;
}
static void PlayAnim(UObject* self,const NameString& sequence,std::optional<float> rate,std::optional<float> tween,std::optional<int32_t> channel)
{
    auto actor=UObject::Cast<UActor>(self);auto resource=RequireResource(actor);
    PoaChannels::Play(State(actor),resource->Animation,sequence.ToString(),rate.value_or(1),tween.value_or(0),ChannelIndex(actor,channel),false,Mesh(actor)?&Mesh(actor)->Geometry:nullptr);
}
static void LoopAnim(UObject* self,const NameString& sequence,std::optional<float> rate,std::optional<float> tween,std::optional<int32_t> channel)
{
    auto actor=UObject::Cast<UActor>(self);auto resource=RequireResource(actor);
    PoaChannels::Play(State(actor),resource->Animation,sequence.ToString(),rate.value_or(1),tween.value_or(0),ChannelIndex(actor,channel),true,Mesh(actor)?&Mesh(actor)->Geometry:nullptr);
}
static void TweenAnim(UObject* self,const NameString& sequence,float time,std::optional<int32_t> channel)
{
    auto actor=UObject::Cast<UActor>(self);auto resource=RequireResource(actor);
    PoaChannels::Play(State(actor),resource->Animation,sequence.ToString(),0,time,ChannelIndex(actor,channel),false,Mesh(actor)?&Mesh(actor)->Geometry:nullptr);
}
static void IsAnimating(UObject* self,std::optional<int32_t> channel,BitfieldBool& result)
{
    auto actor=UObject::Cast<UActor>(self);const auto& c=State(actor).At(ChannelIndex(actor,channel));result=c.Tween>0||(c.Playing&&c.Rate!=0);
}
static void FinishAnim(UObject* self,std::optional<int32_t> channel)
{
    auto actor=UObject::Cast<UActor>(self);auto& state=State(actor);const int index=ChannelIndex(actor,channel);auto& c=state.At(index);
    if((!c.Playing||c.Rate==0)&&c.Tween==0)
    {
        state.Waiting=-1;if(actor->StateFrame&&actor->StateFrame->GetLatentState()==LatentRunState::FinishAnim)actor->StateFrame->SetLatentState(LatentRunState::Continue);
        return;
    }
    c.Loop=false;state.Waiting=index;
    if(actor->StateFrame)actor->StateFrame->SetLatentState(LatentRunState::FinishAnim);
}
static void StopAnimating(UObject* self,std::optional<bool> clearOthers)
{
    auto actor=UObject::Cast<UActor>(self);PoaChannels::Stop(State(actor),clearOthers.value_or(false));
    if(actor->StateFrame&&actor->StateFrame->GetLatentState()==LatentRunState::FinishAnim)actor->StateFrame->SetLatentState(LatentRunState::Continue);
}
static void HasAnim(UObject* self,const NameString& sequence,BitfieldBool& result)
{
    auto resource=Resource(UObject::Cast<UActor>(self));result=false;if(!resource)return;
    for(const auto& s:resource->Animation.Sequences)if(PoaChannels::SameName(s.Name,sequence.ToString())){result=true;return;}
}
static void GetAnimParams(UObject* self,int32_t channel,NameString& name,float& frame,float& rate)
{
    auto actor=UObject::Cast<UActor>(self);auto& c=State(actor).At(channel);name=NameString(c.Name.empty()?"None":c.Name);frame=float(c.Phase);rate=0;
    if(!c.Name.empty()&&c.Playing){auto resource=RequireResource(actor);const auto& seq=resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,c.Name));rate=c.Rate*seq.Rate/seq.Frames;}
}
static void GetRootRotationDelta(Rotator& result)
{
    // EU USkeletalMeshInstance::GetRootRotationDelta constructs (0,0,0)
    // unconditionally after its optional pose refresh (0x104e16c7).
    // This does not implement translational root-motion extraction/physics.
    result={0,0,0};
}
static float ChannelRate(UActor* actor,const PoaChannels::Channel& channel)
{
    if(channel.Name.empty()||!channel.Playing)return 0;
    auto resource=RequireResource(actor);const auto& sequence=resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,channel.Name));
    return channel.Rate*sequence.Rate/sequence.Frames;
}
static void GetAnimTime(UObject* self,std::optional<int32_t> channel,float& result)
{
    auto actor=UObject::Cast<UActor>(self);result=float(State(actor).At(ChannelIndex(actor,channel)).Phase);
}
static void GetAnimFrame(UObject* self,std::optional<int32_t> channel,float& result)
{
    auto actor=UObject::Cast<UActor>(self);const auto& c=State(actor).At(ChannelIndex(actor,channel));result=0;
    if(!c.Name.empty()){auto resource=RequireResource(actor);result=float(c.Phase*resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,c.Name)).Frames);}
}
static void GetAnimRate(UObject* self,std::optional<int32_t> channel,float& result)
{
    auto actor=UObject::Cast<UActor>(self);result=ChannelRate(actor,State(actor).At(ChannelIndex(actor,channel)));
}
static void SetAnimRate(UObject* self,float rate,std::optional<int32_t> channel)
{
    if(!std::isfinite(rate)||rate<0)Exception::Throw("Unsupported PoA backwards/non-finite animation rate");
    auto actor=UObject::Cast<UActor>(self);auto& c=State(actor).At(ChannelIndex(actor,channel));c.Rate=rate;c.Playing=!c.Name.empty()&&rate!=0;
}
static void SetAnimTime(UObject* self,float time,std::optional<int32_t> channel)
{
    if(!std::isfinite(time))Exception::Throw("Non-finite PoA animation time");
    auto actor=UObject::Cast<UActor>(self);State(actor).At(ChannelIndex(actor,channel)).Phase=time;
}
static void SetAnimFrame(UObject* self,float frame,std::optional<int32_t> channel,std::optional<int32_t> unit)
{
    if(!std::isfinite(frame))Exception::Throw("Non-finite PoA animation frame");
    auto actor=UObject::Cast<UActor>(self);auto& c=State(actor).At(ChannelIndex(actor,channel));double phase=frame;
    if(unit.value_or(0)==0&&!c.Name.empty())
    {
        auto resource=RequireResource(actor);auto frames=resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,c.Name)).Frames;
        // Retail divides by Frames-1, while GetAnimFrame multiplies by Frames.
        // Keep that asymmetry; avoid its one-frame divide-by-zero defect.
        if(frames>1)phase/=frames-1;
    }
    c.Phase=phase;
}
static void FreezeAnimAt(UObject* self,float time,std::optional<int32_t> channel)
{
    SetAnimTime(self,time,channel);auto actor=UObject::Cast<UActor>(self);auto& c=State(actor).At(ChannelIndex(actor,channel));c.Rate=0;c.Playing=false;c.Tween=0;c.TweenTotal=0;
}
static void GetAnimSequence(UObject* self,std::optional<int32_t> channel,NameString& result)
{
    const auto actor=UObject::Cast<UActor>(self);auto& c=State(actor).At(ChannelIndex(actor,channel));result=NameString(c.Name.empty()?"None":c.Name);
}
static void GetAnimNumFrames(UObject* self,std::optional<int32_t> channel,float& result)
{
    auto actor=UObject::Cast<UActor>(self);const auto& c=State(actor).At(ChannelIndex(actor,channel));result=0;
    if(!c.Name.empty()){auto resource=RequireResource(actor);result=float(resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,c.Name)).Frames);}
}
static void AnimIsInGroup(UObject* self,int32_t channel,const NameString& group,BitfieldBool& result)
{
    auto actor=UObject::Cast<UActor>(self);const auto& c=State(actor).At(channel);result=false;if(c.Name.empty())return;
    auto resource=RequireResource(actor);const auto& seq=resource->Animation.Sequences.at(PoaChannels::Sequence(resource->Animation,c.Name));
    for(const auto& name:seq.Groups)if(PoaChannels::SameName(name,group.ToString())){result=true;return;}
}
static void LinkSkelAnim(UObject* self,UObject* animation,std::optional<UObject*> replacement)
{
    auto actor=UObject::Cast<UActor>(self);UPoaMeshAnimation* resource=nullptr;UPoaSkeletalMesh* mesh=nullptr;
    if(animation){animation->LoadNow();resource=UObject::Cast<UPoaMeshAnimation>(animation);}
    if(replacement.value_or(nullptr)){(*replacement)->LoadNow();mesh=UObject::Cast<UPoaSkeletalMesh>(*replacement);}
    actor->PoaAnimationObject=resource;if(mesh)actor->Mesh()=mesh;
    actor->PoaAnimation=std::make_shared<PoaChannels::State>();
    if(actor->StateFrame&&actor->StateFrame->GetLatentState()==LatentRunState::FinishAnim)actor->StateFrame->SetLatentState(LatentRunState::Continue);
}
static void AnimBlendParams(UObject* self,int32_t stage,std::optional<float> alpha,std::optional<float> inTime,std::optional<float> outTime,std::optional<NameString> bone,std::optional<bool> global)
{
    auto actor=UObject::Cast<UActor>(self);const std::string mask=bone?bone->ToString():"None";
    if(!PoaChannels::SameName(mask,"None"))
    {
        auto mesh=Mesh(actor);bool found=false;if(mesh)for(const auto& b:mesh->Geometry.Bones)found|=PoaChannels::SameName(b.Name,mask);
        if(!found)Exception::Throw("PoA blend root bone not found: "+mask);
    }
    if(!std::isfinite(inTime.value_or(0))||!std::isfinite(outTime.value_or(0)))Exception::Throw("Non-finite PoA blend coefficient");
    // Retail SetBlendParams writes alpha immediately, stores max(1, In/Out),
    // and clears the BlendToAlpha timer. InTime is not that timer's duration.
    auto& state=State(actor);PoaChannels::Blend(state,stage,alpha.value_or(1),0,mask);
    state.At(stage).BlendIn=std::max(1.0f,inTime.value_or(0));state.At(stage).BlendOut=std::max(1.0f,outTime.value_or(0));state.At(stage).GlobalPose=global.value_or(false);
}
static void AnimBlendToAlpha(UObject* self,int32_t stage,float target,float seconds)
{
    auto& state=State(UObject::Cast<UActor>(self));const auto bone=state.At(stage).Bone;PoaChannels::Blend(state,stage,target,seconds,bone);
}
std::vector<PoaAssets::BoneTransform> Pose(UActor* actor,UPoaSkeletalMesh* mesh)
{
    auto resource=Resource(actor);if(!resource||!actor->PoaAnimation)return PoaAssets::SkeletalPose(mesh->Geometry);
    return PoaChannels::Pose(mesh->Geometry,resource->Animation,*actor->PoaAnimation);
}
void Tick(UActor* actor,float elapsed)
{
    if(!actor->PoaAnimation)return;
    auto resource=Resource(actor);if(!resource)return;
    GCRoot<UActor> actorRoot(actor);
    GCRoot<UPoaMeshAnimation> resourceRoot(resource);
    auto generationState=actor->PoaAnimation;
    const auto events=PoaChannels::Tick(*generationState,resource->Animation,elapsed);
    auto& state=*actor->PoaAnimation;
    if(state.Waiting>=0)
    {
        const auto& channel=state.At(state.Waiting);
        if((!channel.Playing||channel.Rate==0)&&channel.Tween==0)
        {
            state.Waiting=-1;if(actor->StateFrame&&actor->StateFrame->GetLatentState()==LatentRunState::FinishAnim)actor->StateFrame->SetLatentState(LatentRunState::Continue);
        }
    }
    for(const auto& event:events)
    {
        if(actor->PoaAnimation!=generationState||actor->bDeleteMe())break;
        if(actor->PoaAnimation->At(event.Channel).Generation!=event.Generation)continue;
        if(event.End)CallEvent(actor,EventName::AnimEnd,{ExpressionValue::IntValue(event.Channel)});
        else if(event.ObjectReference)
        {
            auto notify=resource->package->GetUObject(event.ObjectReference);if(!notify)Exception::Throw("PoA missing animation notify object");notify->LoadNow();
            if(!PoaNotifies::Dispatch(notify,actor))Exception::Throw("Unsupported PoA native animation notify");
        }
        else if(!PoaChannels::SameName(event.Function,"None")&&!event.Function.empty())CallEvent(actor,NameString(event.Function));
    }

}
void Load(UActor* actor,ObjectStream* stream)
{
    if(stream->Remaining()<4){actor->PoaAnimation.reset();actor->PoaAnimationObject=nullptr;return;}
    const auto start=stream->Tell();
    if(stream->ReadUInt32()!=PoaChannels::StateMagic){stream->Seek(start);actor->PoaAnimation.reset();actor->PoaAnimationObject=nullptr;return;}
    if(stream->ReadUInt32()!=PoaChannels::StateTag)Exception::Throw("Corrupt PoA actor animation save tag");
    if(stream->ReadUInt32()!=PoaChannels::StateVersion)Exception::Throw("Unsupported PoA actor animation save version");
    auto resource=stream->ReadObject<UPoaMeshAnimation>();
    auto replacement=std::make_shared<PoaChannels::State>(PoaChannels::ReadState(*stream));
    if(stream->Remaining()!=0)Exception::Throw("Unexpected bytes after PoA actor animation save state");
    actor->PoaAnimationObject=resource;actor->PoaAnimation=std::move(replacement);
}
void Save(UActor* actor,PackageStreamWriter* stream)
{
    if(!actor->PoaAnimation&&!actor->PoaAnimationObject)return;
    const auto state=actor->PoaAnimation?*actor->PoaAnimation:PoaChannels::State();PoaChannels::Validate(state);
    stream->WriteUInt32(PoaChannels::StateMagic);stream->WriteUInt32(PoaChannels::StateTag);stream->WriteUInt32(PoaChannels::StateVersion);
    stream->WriteObject(actor->PoaAnimationObject);PoaChannels::WriteState(*stream,state);
}
void RegisterNatives()
{
    RegisterVMNativeFunc_4("Actor","PlayAnim",&PlayAnim,259);
    RegisterVMNativeFunc_4("Actor","LoopAnim",&LoopAnim,260);
    RegisterVMNativeFunc_3("Actor","TweenAnim",&TweenAnim,294);
    RegisterVMNativeFunc_2("Actor","IsAnimating",&IsAnimating,282);
    RegisterVMNativeFunc_1("Actor","FinishAnim",&FinishAnim,261);
    RegisterVMNativeFunc_1("Actor","StopAnimating",&StopAnimating,0);
    RegisterVMNativeFunc_2("Actor","HasAnim",&HasAnim,263);
    RegisterVMNativeFunc_4("Actor","GetAnimParams",&GetAnimParams,0);
    RegisterVMNativeFunc_1("Actor","GetRootRotationDelta",&GetRootRotationDelta,0);
    RegisterVMNativeFunc_2("Actor","GetAnimTime",&GetAnimTime,602);
    RegisterVMNativeFunc_2("Actor","GetAnimFrame",&GetAnimFrame,601);
    RegisterVMNativeFunc_2("Actor","GetAnimRate",&GetAnimRate,600);
    RegisterVMNativeFunc_2("Actor","SetAnimRate",&SetAnimRate,0);
    RegisterVMNativeFunc_2("Actor","SetAnimTime",&SetAnimTime,0);
    RegisterVMNativeFunc_3("Actor","SetAnimFrame",&SetAnimFrame,0);
    RegisterVMNativeFunc_2("Actor","FreezeAnimAt",&FreezeAnimAt,0);
    RegisterVMNativeFunc_2("Actor","GetAnimSequence",&GetAnimSequence,603);
    RegisterVMNativeFunc_2("Actor","GetAnimNumFrames",&GetAnimNumFrames,604);
    RegisterVMNativeFunc_3("Actor","AnimIsInGroup",&AnimIsInGroup,0);
    RegisterVMNativeFunc_2("Actor","LinkSkelAnim",&LinkSkelAnim,0);
    RegisterVMNativeFunc_6("Actor","AnimBlendParams",&AnimBlendParams,0);
    RegisterVMNativeFunc_3("Actor","AnimBlendToAlpha",&AnimBlendToAlpha,0);
}
}
