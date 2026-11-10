#pragma once
#include "Packages/Engine/Actors/UActor.h"
#include "VM/ScriptCall.h"
// UE2 Controller is an Actor, not a Pawn. Script properties remain reflected.
class UPoaController : public UActor { public: using UActor::UActor; };
class UPoaPlayerController : public UPoaController
{
public:
    using UPoaController::UPoaController;
    void Tick(float elapsed) override
    {
        UActor::Tick(elapsed);
        // PlayerTick's original script invokes the PlayerInput object and state
        // PlayerMove. Do not also run UE1 PlayerPawn input or duplicate it here.
        if(!bDeleteMe() && HasProperty("Player") && GetUObject("Player"))
            CallEvent(this,"PlayerTick",{ExpressionValue::FloatValue(elapsed)});
    }
};
