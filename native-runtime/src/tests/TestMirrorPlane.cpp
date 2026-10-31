#include "Test.h"
#include "../knowwonder/KWMirrorPlane.h"
#include <limits>

TEST(MirrorPlane_OnlyCoplanarFacesShareReflection)
{
	CHECK(KW::SameMirrorPlane(1, 0, 0, 832, 1, 0, 0, 832));
	CHECK(KW::SameMirrorPlane(1, 0, 0, 832, 1, 0, 0, 832.005f));
	CHECK(!KW::SameMirrorPlane(1, 0, 0, 832, 1, 0, 0, 833));
	CHECK(!KW::SameMirrorPlane(1, 0, 0, 832, 0, 1, 0, 832));
	CHECK(!KW::SameMirrorPlane(1, 0, 0, 832, -1, 0, 0, -832));
	CHECK(!KW::SameMirrorPlane(1, 0, 0, 832, 1, 0, 0, std::numeric_limits<float>::quiet_NaN()));
}
