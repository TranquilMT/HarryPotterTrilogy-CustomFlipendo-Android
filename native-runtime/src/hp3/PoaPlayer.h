#pragma once
#include <string>
class UObject;class UActor;class UViewport;
namespace PoaPlayer {
void Login(UObject* game,UViewport* viewport,const std::string& portal,const std::string& options);
UActor* ViewTarget(UActor* actor);
void SetButton(UObject* actor,const std::string& name,bool down);
}
