#include "Precomp.h"
#include "UPoaMeshAnimation.h"
void UPoaMeshAnimation::Load(ObjectStream* stream)
{
    UObject::Load(stream);
    auto animation=PoaAssets::ReadAnimation(*stream,stream->GetVersion());
    Animation=std::move(animation);
}
void UPoaMeshAnimation::Save(PackageStreamWriter*)
{
    Exception::Throw("PoA animation package rewriting is not implemented");
}
