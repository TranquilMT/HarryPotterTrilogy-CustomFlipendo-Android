#pragma once
#include <cstdint>

namespace PoaPhysics
{
// EU raw values: None, Walking, Rotating, Spider and RootMotion preserve an
// existing base in setPhysics. Floor discovery/root-lock work is separate.
inline bool KeepsExistingBase(uint8_t raw)
{
    return raw == 0 || raw == 1 || raw == 5 || raw == 9 || raw == 12;
}

// Interpret serialized Physics without rewriting it: scripts and saves use the
// game's own enumeration. Return the shared UE1 handler number where one exists.
inline int EngineMode(uint8_t raw, bool poa)
{
    if (!poa || raw <= 6) return raw;
    switch (raw)
    {
    case 7: return 8;  // EU Interpolating -> PHYS_Interpolating
    case 8: return 9;  // EU MovingBrush -> PHYS_MovingBrush
    case 9: return 10; // EU Spider -> PHYS_Spider
    case 10: return 11;// EU Trailer -> PHYS_Trailer
    default: return 256 + raw; // Ladder, RootMotion, Karma, PushPulled or unknown
    }
}
}
