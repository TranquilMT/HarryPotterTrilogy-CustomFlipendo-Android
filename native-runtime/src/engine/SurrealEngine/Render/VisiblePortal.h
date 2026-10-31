#pragma once

#include "VisibleNode.h"
#include "BspClipper.h"

class BspNode;
class UMover;
class USkyZoneInfo;
class UWarpZoneInfo;

class VisiblePortal
{
public:
	Array<VisibleNode> Nodes;
	Array<PortalSpan> Spans;
	USkyZoneInfo* SkyZone = nullptr;
	UWarpZoneInfo* WarpZone = nullptr;

	// A mirror on a mover polygon instead of a BSP surface (UE1 adds mover polygons to the BSP, so they mirror too):
	// its world-space plane, and the polygon to seal the portal with
	UMover* Mover = nullptr;
	int MoverPoly = -1;
	vec3 MirrorBase = vec3(0.0f);
	vec3 MirrorNormal = vec3(0.0f);
};
