#pragma once
#include "Math/vec.h"
#include <cmath>

namespace KW
{
	// A long wand's endpoint is the centre of its far end cap, not the
	// diagonal corner of its box (which displaces effects sideways).
	inline vec3 WandTipLocal(const vec3& minimum, const vec3& maximum)
	{
		vec3 extent = maximum - minimum;
		int axis = 0;
		if (extent.y > extent.x) axis = 1;
		if (extent.z > extent[axis]) axis = 2;
		vec3 tip = (minimum + maximum) * 0.5f;
		tip[axis] = std::fabs(maximum[axis]) >= std::fabs(minimum[axis]) ? maximum[axis] : minimum[axis];
		return tip;
	}
}
