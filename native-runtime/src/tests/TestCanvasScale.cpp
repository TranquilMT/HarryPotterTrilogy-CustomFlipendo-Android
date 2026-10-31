#include "Test.h"
#include "../engine/SurrealWidgets/include/surrealwidgets/core/canvas.h"
#include "../hp2/HP2FrontEndLayout.h"

TEST(CanvasScale_PreservesPhysicalWidgetClip)
{
	for (double dpi : {1.0, 2.0, 2.75, 3.0})
	{
		// Reproduce a landscape Android surface and its pre-existing widget clip.
		const double width = 1536 / dpi, height = 687 / dpi;
		auto layout = HP2::FrontEndLayout::Fit(width, height);
		const double scale = dpi * layout.scale;
		Rect original = Rect::xywh(0, 0, width, height);
		Rect clip = Canvas::rescaleClip(original, dpi, scale);
		CHECK_NEAR(clip.width * scale, 1536, 1e-8);
		CHECK_NEAR(clip.height * scale, 687, 1e-8);
		// Bottom/right menu content must fit inside the inherited physical clip.
		CHECK((layout.x + 480 * layout.scale) * dpi <= clip.width * scale + 1e-8);
		CHECK((layout.y + 536 * layout.scale) * dpi <= clip.height * scale + 1e-8);
		Rect restored = Canvas::rescaleClip(clip, scale, dpi);
		CHECK_NEAR(restored.width, original.width, 1e-8);
		CHECK_NEAR(restored.height, original.height, 1e-8);
	}
}
