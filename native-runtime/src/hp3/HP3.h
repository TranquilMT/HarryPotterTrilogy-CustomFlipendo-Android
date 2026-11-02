'// HP3 (Prisoner of Azkaban) experimental stubs.
//
// HP3 uses Unreal Engine 2 and is NOT supported. This module is disabled by
// default (HP3_ENABLED=0). Nothing here may be enabled at runtime; the guard
// is compile-time only. Do not include this header in production code paths
// unless HP3_ENABLED is explicitly set to 1.
#pragma once

#ifndef HP3_ENABLED
#define HP3_ENABLED 0
#endif

#if HP3_ENABLED
namespace HP3 {
inline bool IsEnabled() { return true; }
}
#else
namespace HP3 {
inline bool IsEnabled() { return false; }
}
#endif
