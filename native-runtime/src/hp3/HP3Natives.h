'// HP3 (Prisoner of Azkaban) native function registration (experimental).
//
// HP3 uses UE2 and its native set differs from HP1/HP2. Each native must be
// verified against retail data before a body is implemented. This file is a
// placeholder: RegisterHP3Natives() is intentionally empty and is never called
// while HP3_ENABLED is 0.
#pragma once

#ifndef HP3_ENABLED
#define HP3_ENABLED 0
#endif

#if HP3_ENABLED
namespace HP3 {
inline void RegisterHP3Natives() {}
}
#endif
