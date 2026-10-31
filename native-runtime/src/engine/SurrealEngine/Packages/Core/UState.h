#pragma once

#include "UStruct.h"

class UFunction;

enum class ScriptStateFlags : uint32_t
{
	Editable = 0x00000001,
	Auto = 0x00000002,
	Simulated = 0x00000004
};

inline ScriptStateFlags operator|(ScriptStateFlags a, ScriptStateFlags b) { return (ScriptStateFlags)((uint32_t)a | (uint32_t)b); }
inline ScriptStateFlags operator&(ScriptStateFlags a, ScriptStateFlags b) { return (ScriptStateFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(ScriptStateFlags value, ScriptStateFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(ScriptStateFlags value, ScriptStateFlags flags) { return (uint32_t)(value & flags) != 0; }

class UState : public UStruct
{
public:
	using UStruct::UStruct;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	uint64_t ProbeMask = 0;
	uint64_t IgnoreMask = 0;
	uint16_t LabelTableOffset = 0;
	ScriptStateFlags StateFlags = {};

	// A state inherits the functions of the state it extends (BaseStruct): "state Beater extends Fly" runs Fly's
	// BeginState. UE1 finds them through the state's field hash, which holds the super state's fields too.
	UFunction* GetFunction(const NameString& name);
	std::map<NameString, UFunction*> Functions;
};
