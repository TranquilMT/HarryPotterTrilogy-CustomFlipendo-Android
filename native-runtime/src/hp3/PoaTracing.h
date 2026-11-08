#pragma once
namespace PoaTracing
{
inline bool Enabled=false;
inline bool IncludeNative=false;
inline void Set(bool enabled,bool includeNative) { Enabled=enabled;IncludeNative=includeNative; }
inline bool ShouldTrace(bool native) { return Enabled&&(!native||IncludeNative); }
}
