#include "Precomp.h"
#include "PoaBones.h"
#include "PoaActorAnimation.h"
#include "PoaRotation.h"
#include "Package/PoaAnimationChannels.h"
#include "Package/PoaBoneTransforms.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/Mesh/UPoaSkeletalMesh.h"
#include "Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h"
#include "VM/NativeFunc.h"
#include <set>
namespace PoaBones
{
static UPoaSkeletalMesh* Mesh(UActor* actor)
{
    auto mesh=actor->Mesh();if(mesh)mesh->LoadNow();return UObject::TryCast<UPoaSkeletalMesh>(mesh);
}
static int BoneIndex(UPoaSkeletalMesh* mesh,const NameString& name)
{
    if(mesh)for(size_t i=0;i<mesh->Geometry.Bones.size();++i)if(PoaChannels::SameName(mesh->Geometry.Bones[i].Name,name.ToString()))return int(i);
    return -1;
}
static mat4 WorldBone(UActor* actor,const NameString& name)
{
    auto mesh=Mesh(actor);const int index=BoneIndex(mesh,name);
    if(index<0)Exception::Throw("PoA bone not found: "+name.ToString());
    auto pose=PoaActorAnimation::Pose(actor,mesh);
    return PoaStaticMeshToWorld(actor)*mesh->meshToObject*BoneMatrix(pose.at(size_t(index)));
}
static Rotator WorldRotation(const mat4& matrix)
{
    const auto q=quaternion::rotation_matrix(RotationMatrix(matrix));
    // Engine rotation_matrix returns the passive quaternion; PoaRotation's
    // retail conversion expects the corresponding active quaternion.
    return PoaRotation::ToRotator({-q.x,-q.y,-q.z,q.w});
}
bool IsAttached(UActor* actor)
{
    return actor&&actor->ActorBase()&&actor->HasProperty("AttachmentBone")&&!PoaChannels::SameName(actor->GetName("AttachmentBone").ToString(),"None");
}
bool UpdateAttachment(UActor* actor,int depth)
{
    if(!IsAttached(actor)||actor->bDeleteMe())return false;
    if(depth>=64)Exception::Throw("PoA bone attachment cycle/depth limit");
    GCRoot<UActor> actorRoot(actor);GCRoot<UActor> parent(actor->ActorBase());const auto name=actor->GetName("AttachmentBone");
    if(parent->bDeleteMe())return false;
    if(IsAttached(parent.get())&&!UpdateAttachment(parent.get(),depth+1))return false;
    auto mesh=Mesh(parent.get());if(BoneIndex(mesh,name)<0)return false;
    const auto offset=actor->HasProperty("RelativeLocation")?actor->GetVector("RelativeLocation"):vec3(0);
    const auto rotation=actor->HasProperty("RelativeRotation")?actor->GetRotator("RelativeRotation"):Rotator(0,0,0);
    auto transform=AttachmentMatrix(WorldBone(parent.get(),name),offset,rotation);
    const auto location=(transform*vec4(0,0,0,1)).xyz();
    if(length((transform*vec4(1,0,0,0)).xyz())<1e-8f||length((transform*vec4(0,1,0,0)).xyz())<1e-8f)return false;
    const auto targetRotation=WorldRotation(transform);
    if(length(location-actor->Location())>1e-5f&&!actor->SetLocation(location))return false;
    if(actor->bDeleteMe()||actor->ActorBase()!=parent.get()||actor->GetName("AttachmentBone")!=name)return false;
    actor->SetRotation(targetRotation);return !actor->bDeleteMe();
}
void UpdateAttachments(UActor* parent)
{
    GCRoot<UActor> parentRoot(parent);
    std::vector<std::unique_ptr<GCRoot<UActor>>> children;
    for(auto child:parent->BasedActors)if(child)children.push_back(std::make_unique<GCRoot<UActor>>(child));
    for(auto& child:children)if(child->get()->ActorBase()==parent&&IsAttached(child->get()))UpdateAttachment(child->get());
}
void SyncAttached(UActor* parent,UActor* child,bool add)
{
    if(!parent->HasProperty("Attached"))return;
    auto attached=parent->DynamicArray<UObject*>(parent->GetPropertyDataOffset("Attached"));
    size_t found=attached.size();for(size_t i=0;i<attached.size();++i)if(attached[i]==child){found=i;break;}
    if(add){if(found==attached.size())attached.push_back(child);}
    else if(found<attached.size()){for(size_t i=found+1;i<attached.size();++i)attached[i-1]=attached[i];attached.pop_back();}
}
static void HasBone(UObject* self,const NameString& name,BitfieldBool& result)
{
    result=BoneIndex(Mesh(UObject::Cast<UActor>(self)),name)>=0;
}
static void GetBoneCoords(UObject* self,const NameString& name,Coords& result)
{
    auto actor=UObject::Cast<UActor>(self);if(IsAttached(actor))UpdateAttachment(actor);result=QueryCoords(WorldBone(actor,name));
}
static void GetBoneRotation(UObject* self,const NameString& name,std::optional<int32_t> space,Rotator& result)
{
    // The inspected EU indexed helper does not read its Space argument.
    (void)space;auto actor=UObject::Cast<UActor>(self);if(IsAttached(actor))UpdateAttachment(actor);result=WorldRotation(WorldBone(actor,name));
}
static void BoneRefresh(UObject* self)
{
    auto actor=UObject::Cast<UActor>(self);if(auto mesh=Mesh(actor))PoaActorAnimation::Pose(actor,mesh);UpdateAttachments(actor);
}
static void AttachToBone(UObject* self,UObject* object,const NameString& name,BitfieldBool& result)
{
    result=false;auto parent=UObject::Cast<UActor>(self);auto child=UObject::TryCast<UActor>(object);
    if(!child||child==parent||parent->bDeleteMe()||child->bDeleteMe()||BoneIndex(Mesh(parent),name)<0)return;
    std::set<UActor*> seen;for(auto current=parent;current;current=current->ActorBase())if(current==child||!seen.insert(current).second)return;
    if(!child->HasProperty("AttachmentBone"))return;
    GCRoot<UActor> parentRoot(parent);GCRoot<UActor> childRoot(child);
    child->SetName("AttachmentBone",name);child->SetBase(parent,true);
    if(child->ActorBase()!=parent)return;
    result=true;UpdateAttachment(child);
}
static void DetachFromBone(UObject* self,UObject* object,BitfieldBool& result)
{
    result=false;auto parent=UObject::Cast<UActor>(self);auto child=UObject::TryCast<UActor>(object);
    if(!child||child->ActorBase()!=parent||!IsAttached(child))return;
    GCRoot<UActor> root(child);child->SetBase(nullptr,true);
    if(!child->ActorBase()&&child->HasProperty("AttachmentBone"))child->SetName("AttachmentBone","None");result=true;
}
void RegisterNatives()
{
    RegisterVMNativeFunc_2("Actor","HasBone",&HasBone,0);
    RegisterVMNativeFunc_2("Actor","GetBoneCoords",&GetBoneCoords,0);
    RegisterVMNativeFunc_3("Actor","GetBoneRotation",&GetBoneRotation,0);
    RegisterVMNativeFunc_0("Actor","BoneRefresh",&BoneRefresh,0);
    RegisterVMNativeFunc_3("Actor","AttachToBone",&AttachToBone,0);
    RegisterVMNativeFunc_2("Actor","DetachFromBone",&DetachFromBone,0);
}
}
