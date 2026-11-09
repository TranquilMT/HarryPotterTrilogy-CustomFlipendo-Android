#pragma once
class UActor;
class UObject;
namespace PoaBones
{
void RegisterNatives();
bool IsAttached(UActor* actor);
bool UpdateAttachment(UActor* actor,int depth=0);
void UpdateAttachments(UActor* parent);
void SyncAttached(UActor* parent,UActor* child,bool add);
}
