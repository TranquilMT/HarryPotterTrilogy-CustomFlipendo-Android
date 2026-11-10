#pragma once
class UObject;
namespace PoaGui { void RegisterNatives(); void Initialize(); void Shutdown(); void Tick(float elapsed); UObject* Controller(); }
