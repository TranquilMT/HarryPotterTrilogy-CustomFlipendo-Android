#pragma once
#include <algorithm>

namespace HP2
{
	struct FrontEndLayout
	{
		double scale, x, y;
		static FrontEndLayout Fit(double width, double height)
		{
			double scale = std::max(0.001, std::min(width / 480.0, height / 536.0));
			return { scale, (width - 480.0 * scale) * 0.5, (height - 536.0 * scale) * 0.5 };
		}
	};
}
