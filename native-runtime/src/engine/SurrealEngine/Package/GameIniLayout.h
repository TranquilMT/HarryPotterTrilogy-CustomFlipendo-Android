#pragma once
#include "GameFolder.h"

// UE2 package version 129 is not a pre-220 UE1 engine build. PoA leaves the
// UE1 ancestry field unset, but uses repeated Paths= and separate User.ini.
inline bool UsesIndexedPackagePaths(const GameLaunchInfo& game)
{
    return !game.IsUE2() && !game.IsRune() && game.ue1Version <= 219;
}
inline bool UsesCombinedUserIni(const GameLaunchInfo& game)
{
    return game.IsKlingonHonorGuard() || (!game.IsUE2() && game.ue1Version <= 219);
}
inline bool LoadsSeparateUserIni(const GameLaunchInfo& game)
{
    return game.IsUE2() || game.ue1Version > 219;
}

inline bool UsesDefaultPackagePaths(const GameLaunchInfo& game, bool hasConfiguredPaths)
{
    return game.IsUE2() && !hasConfiguredPaths;
}
