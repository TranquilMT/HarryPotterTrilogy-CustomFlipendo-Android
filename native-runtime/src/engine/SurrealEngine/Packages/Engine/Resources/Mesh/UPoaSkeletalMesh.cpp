#include "Precomp.h"
#include "UPoaSkeletalMesh.h"
#include "UPoaMeshAnimation.h"
#include "Package/Package.h"
#include "Math/coords.h"
void UPoaSkeletalMesh::Load(ObjectStream* stream)
{
    UPrimitive::Load(stream);
    auto geometry=PoaAssets::ReadSkeletalMesh(*stream,stream->GetVersion());
    PoaAssets::SkeletalPose(geometry); // validate hierarchy before replacing live data
    auto sourcePackage=stream->GetPackage();
    UPoaMeshAnimation* animationResource=nullptr;
    if(auto object=sourcePackage->GetUObject(geometry.AnimationReference))
    {
        object->LoadNow();animationResource=UObject::TryCast<UPoaMeshAnimation>(object);
    }
    Array<MeshAnimSeq> sequences;
    if(animationResource) for(const auto& sequence:animationResource->Animation.Sequences)
    {
        if(sequence.Frames<=0) continue; // inherited actor playback divides by frame count
        MeshAnimSeq entry;entry.Name=NameString(sequence.Name);entry.Group=sequence.Groups.empty()?NameString("None"):NameString(sequence.Groups[0]);
        entry.StartFrame=sequence.StartFrame;entry.NumFrames=sequence.Frames;entry.Rate=sequence.Rate;
        // Function notifies can use the existing actor event route. Object
        // notifies require UE2 Notify(actor, sequence) dispatch separately.
        for(const auto& notify:sequence.Notifies) if(notify.ObjectReference==0&&notify.Function!="None")entry.Notifys.push_back({notify.Time,NameString(notify.Function)});
        sequences.push_back(std::move(entry));
    }
    Geometry=std::move(geometry);SourcePackage=sourcePackage;AnimationResource=animationResource;AnimSeqs=std::move(sequences);
    Scale={Geometry.Scale[0],Geometry.Scale[1],Geometry.Scale[2]};Origin={Geometry.Origin[0],Geometry.Origin[1],Geometry.Origin[2]};
    RotOrigin.Pitch=Geometry.Rotation[0];RotOrigin.Yaw=Geometry.Rotation[1];RotOrigin.Roll=Geometry.Rotation[2];
    meshToObject=Coords::Rotation(RotOrigin).ToMatrix()*mat4::scale(Scale)*mat4::translate(-Origin);
}
void UPoaSkeletalMesh::Save(PackageStreamWriter*)
{
    Exception::Throw("PoA skeletal mesh package rewriting is not implemented");
}
