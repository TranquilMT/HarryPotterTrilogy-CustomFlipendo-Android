#pragma once
#include <cmath>

namespace KW
{
	// BSP normals are unit vectors. Keep opposite-facing planes separate as well:
	// their reflected view has a different clipping half-space.
	inline bool SameMirrorPlane(float ax, float ay, float az, float aw,
		float bx, float by, float bz, float bw)
	{
		return std::fabs(ax - bx) <= 0.0001f && std::fabs(ay - by) <= 0.0001f &&
			std::fabs(az - bz) <= 0.0001f && std::fabs(aw - bw) <= 0.01f;
	}
}
