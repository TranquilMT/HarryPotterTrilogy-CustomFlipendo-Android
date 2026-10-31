#include "Test.h"
#include "../hp2/HP2FrontEndLayout.h"
#include "../knowwonder/KWWandTip.h"

TEST(HP2FrontEnd_FitsSmallAndWideScreens)
{
	for (auto size : { std::pair<double,double>{320,180}, {960,540}, {1390,632}, {2560,1440} })
	{
		auto fit = HP2::FrontEndLayout::Fit(size.first, size.second);
		CHECK(fit.scale > 0);
		CHECK(fit.x >= -0.001 && fit.y >= -0.001);
		CHECK(fit.x + 480 * fit.scale <= size.first + 0.001);
		CHECK(fit.y + 536 * fit.scale <= size.second + 0.001);
		// The rendered button and touch coordinate must describe the same point.
		CHECK_NEAR((fit.x + 120 * fit.scale - fit.x) / fit.scale, 120, 1e-9);
		CHECK_NEAR((fit.y + 400 * fit.scale - fit.y) / fit.scale, 400, 1e-9);
	}
}

TEST(WandTip_UsesEndCapCentreInsteadOfDiagonalCorner)
{
	vec3 tip = KW::WandTipLocal(vec3(-1,-2,0), vec3(1,2,25));
	CHECK_NEAR(tip.x, 0, 1e-6); CHECK_NEAR(tip.y, 0, 1e-6); CHECK_NEAR(tip.z, 25, 1e-6);
	vec3 reversed = KW::WandTipLocal(vec3(-35,-1,-1), vec3(0,1,1));
	CHECK_NEAR(reversed.x, -35, 1e-6); CHECK_NEAR(reversed.y, 0, 1e-6); CHECK_NEAR(reversed.z, 0, 1e-6);
}
