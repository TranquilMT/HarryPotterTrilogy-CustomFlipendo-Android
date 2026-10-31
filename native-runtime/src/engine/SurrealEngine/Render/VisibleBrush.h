#pragma once

#include "Math/mat.h"

class VisibleFrame;
class UActor;
class UModel;
class UMover;
struct Poly;

class VisibleBrush
{
public:
	bool Draw(VisibleFrame* frame, UActor* actor, bool translucentPass);

	// Mirror polygons (PF_Mirrored) facing the view become mirror portals of the frame, like mirrored BSP surfaces
	static void AddMirrorPortals(VisibleFrame* frame, UMover* mover);
	static void DrawMirrorSeal(VisibleFrame* frame, UMover* mover, int polyIndex);

private:
	static mat4 BrushToWorld(UMover* mover);
	static bool IsMirrorPortal(VisibleFrame* frame, uint32_t polyFlags);
	bool DrawBrushPoly(VisibleFrame* frame, UModel* model, const Poly& poly, int pass, UMover* mover, bool translucentPass);
};
