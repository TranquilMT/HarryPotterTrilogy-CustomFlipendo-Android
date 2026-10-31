
#include "Precomp.h"
#include "KWProf.h"
#include "VisibleFrame.h"
#include "VisibleCorona.h"
#include "VisibleBrush.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Actors/Info/UWarpZoneInfo.h"
#include "Packages/Engine/Actors/Info/USkyZoneInfo.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Extension/Windows/TabGroup/URootWindow.h"
#include "Engine.h"
#include "VM/ScriptCall.h"
#include "KW.h"
#include "HP2.h"
#include "KWMirrorPlane.h"

void VisibleFrame::Process(const vec3& location, const mat4& worldToView, const Coords& viewRotation, bool mirrorFlag, int portalDepth, const Array<PortalSpan>& portalSpans, const vec4& portalPlane)
{
	engine->render->Stats.Frames++;

	Device = engine->render->Device;
	FrameCounter = engine->render->FrameCounter++;
	MirrorFlag = mirrorFlag;
	PortalDepth = portalDepth;
	PortalPlane = portalPlane;

	SetupSceneFrame(worldToView);

	// A mirror on a mover reflects only what is in front of it. The clipper culls BSP surfaces by the plane, but
	// whatever crosses it (walls beside the mirror, actors) is drawn whole, and its part behind the mirror covers the
	// reflection when the camera is close. The device clips every vertex by NearClip (view space) instead.
	if (mirrorFlag && portalDepth > 0 && (portalPlane.x != 0.0f || portalPlane.y != 0.0f || portalPlane.z != 0.0f))
	{
		vec3 n = portalPlane.xyz();
		vec3 p = (Frame.WorldToView * vec4(n * -portalPlane.w, 1.0f)).xyz();
		vec3 nv = normalize((Frame.WorldToView * vec4(n, 0.0f)).xyz());
		Frame.NearClip = vec4(nv, -dot(p, nv));
	}

	Clipper.numDrawSpans = 0;
	Clipper.numSurfs = 0;
	Clipper.numTris = 0;
	Clipper.Setup(Frame.Projection * Frame.WorldToView * Frame.ObjectToWorld, portalSpans, portalPlane);

	ViewLocation = vec4(location, 1.0f);
	ViewZone = FindZoneAt(location);
	ZoneFilter = (OnlyViewZone && ViewZone > 0 && ViewZone < 64) ? (1ULL << ViewZone) : 0;
	if (engine->LaunchInfo.IsHarryPotter2())
		HP2::MarkLumosSurfaces(ViewZone);
	//ViewZoneMask = ViewZone ? 1ULL << ViewZone : -1;
	ViewRotation = viewRotation;

	OpaqueNodes.clear();
	Actors.clear();
	Translucents.clear();
	Coronas.clear();
	Portals.clear();

	if (engine->LaunchInfo.IsKnowWonder())
		KW::UpdateActorShadows(this);

	ProcessNode(&engine->Level->Model->Nodes[0]);
}

vec3 VisibleFrame::EyeLocation() const
{
	const mat4& m = Frame.WorldToView;
	return mat3::inverse(mat3(m)) * -vec3(m.matrix[12], m.matrix[13], m.matrix[14]);
}

void VisibleFrame::SetupSceneFrame(const mat4& worldToView)
{
	Frame.XB = engine->viewport->ViewportX();
	Frame.YB = engine->viewport->ViewportY();
	Frame.X = engine->viewport->ViewportWidth();
	Frame.Y = engine->viewport->ViewportHeight();
	Frame.FX = (float)engine->viewport->ViewportWidth();
	Frame.FY = (float)engine->viewport->ViewportHeight();

	if (engine->dxRootWindow && engine->dxRootWindow->RenderViewportSet)
	{
		float virtscale = engine->dxRootWindow->GetVirtualScale();
		float x = engine->dxRootWindow->renderX();
		float y = engine->dxRootWindow->renderY();
		float w = engine->dxRootWindow->renderWidth();
		float h = engine->dxRootWindow->renderHeight();

		// Center, scale and possibly extend viewport if it covers the entire root window
		if (x <= 0.0f && w >= engine->dxRootWindow->Width())
		{
			x = 0.0f;
			w = (float)engine->viewport->ViewportWidth();
		}
		else
		{
			x = (engine->dxRootWindow->UsedX + x) * virtscale;
			w *= virtscale;
		}
		if (y <= 0.0f && h >= engine->dxRootWindow->Height())
		{
			y = 0.0f;
			h = (float)engine->viewport->ViewportHeight();
		}
		else
		{
			y = (engine->dxRootWindow->UsedY + y) * virtscale;
			h *= virtscale;
		}

		Frame.X = (int)w;
		Frame.Y = (int)h;
		Frame.XB = (int)std::round(x);
		Frame.YB = (int)std::round(y);
		Frame.FX = w;
		Frame.FY = h;
	}

	Frame.FX2 = Frame.FX * 0.5f;
	Frame.FY2 = Frame.FY * 0.5f;
	Frame.ObjectToWorld = mat4::identity();
	Frame.WorldToView = worldToView;
	Frame.FovAngle = engine->CameraFovAngle;
	float Aspect = Frame.FY / Frame.FX;
	float RProjZ = (float)std::tan(radians(Frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / Frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / Frame.FY;
	Frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
}

void VisibleFrame::ProcessNode(BspNode* node)
{
	// Skip node if it is not part of the portal zones we have seen so far
	//if ((node->ZoneMask & ViewZoneMask) == 0)
	//	return;

	if (ZoneFilter && (node->ZoneMask & ZoneFilter) == 0)
		return;

	// Skip node if its AABB is not visible
	if (node->RenderBound != -1 && !Clipper.IsAABBVisible(engine->Level->Model->Bounds[node->RenderBound]))
	{
		return;
	}

	// Add bsp node actors to the visible set
	for (UActor* actor = node->ActorList; actor != nullptr; actor = actor->BspInfo.Next)
	{
		if (actor->LastDrawFrame != FrameCounter)
		{
			actor->LastDrawFrame = FrameCounter;

			VisibleActor visactor;
			visactor.Process(this, actor);
		}
	}

	// Decide which side the plane the camera is
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	bool swapFrontAndBack = dot(ViewLocation, plane) < 0.0f;
	int back = node->Back;
	int front = node->Front;
	if (swapFrontAndBack)
		std::swap(front, back);

	// Recursively divide front space (toward the viewer)
	if (front >= 0)
	{
		ProcessNode(&engine->Level->Model->Nodes[front]);
	}

	// Draw surfaces on this plane
	BspNode* polynode = node;
	while (true)
	{
		ProcessNodeSurface(polynode, swapFrontAndBack);

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	// Possibly divide back space (away from the viewer)
	if (back >= 0)
	{
		ProcessNode(&engine->Level->Model->Nodes[back]);
	}
}

void VisibleFrame::ProcessNodeSurface(BspNode* node, bool front)
{
	if (node->NumVertices < 3 || node->Surf < 0)
		return;

	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	int numverts = node->NumVertices;
	vec3* points = engine->render->GetTempVertexBuffer(numverts);
	BspVert* v = &model->Vertices[node->VertPool];
	if (MirrorFlag)
	{
		for (int j = 0; j < numverts; j++)
		{
			points[numverts - 1 - j] = model->Points[v[j].Vertex];
		}
	}
	else
	{
		for (int j = 0; j < numverts; j++)
		{
			points[j] = model->Points[v[j].Vertex];
		}
	}

	uint32_t PolyFlags = surface.PolyFlags;
	UTexture* texture = surface.Material;
	if (!texture)
		texture = engine->LevelInfo->DefaultTexture();

	if (surface.Material)
		PolyFlags |= surface.Material->PolyFlags();

	VisibleNode info;
	info.Node = node;
	// Whether the camera is in front of this node's own plane (a coplanar node can face the other way): the surface
	// then takes the front zone (Zone1) for its ambient light and texture panning. ProcessNode's swapFrontAndBack is the
	// opposite (true when the camera is behind its plane).
	info.Front = dot(ViewLocation, vec4(node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW)) >= 0.0f;
	info.PolyFlags = PolyFlags;

	// A one-sided surface seen from behind is neither drawn nor an occluder, as in UE1's BSP renderer. HP2's PrivetDr
	// has a lone editor guide plane (greenguide, not invisible on one side) between the title shot's camera and its sky
	// backdrop: drawn from behind it hid the sky as a dark rectangle behind the Harry Potter logo.
	if (!info.Front && !(PolyFlags & PF_TwoSided))
		return;

	if (PortalDepth < 4)
	{
		if ((PolyFlags & (PF_FakeBackdrop | PF_Invisible)) == PF_FakeBackdrop)
		{
			int zone = front ? node->Zone0 : node->Zone1;
			UZoneInfo* zoneInfo = engine->GetZoneActor(zone);
			if (zoneInfo->SkyZone())
			{
				Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
				if (!spans.empty())
				{
					USkyZoneInfo* skyZone = zoneInfo->SkyZone();
					for (auto& p : Portals)
					{
						if (p.SkyZone == skyZone)
						{
							p.Nodes.push_back(info);
							p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
							return;
						}
					}
					VisiblePortal portal;
					portal.Nodes.push_back(info);
					portal.SkyZone = skyZone;
					portal.Spans = std::move(spans);
					Portals.push_back(std::move(portal));
				}
				return;
			}
		}
		else if (PolyFlags & PF_Portal)
		{
			// The zone beyond the portal as seen from the frame's eye: a BSP mirror's frame keeps the real camera as
			// ViewLocation, on the other side of a portal the reflection looks through (Lev5_Final's Mirror of Erised
			// looks back into a warp zone)
			vec3 eye = EyeLocation();
			bool eyeBehind = dot(vec4(eye, 1.0f), vec4(node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW)) < 0.0f;
			int portalZone = eyeBehind ? node->Zone1 : node->Zone0;
			if (portalZone > 0)
			{
				// The other side is transient, so a loaded save has none: generate it when it's missing, as HP1's
				// renderer does. A warp zone still without one (no OtherSideURL, like Lev5_Final's
				// MirroredRoomForStoner) is drawn as a plain zone portal.
				UWarpZoneInfo* warpZone = UObject::TryCast<UWarpZoneInfo>(engine->GetZoneActor(portalZone));
				if (warpZone && (!warpZone->OtherSideActor() || !warpZone->OtherSideLevel()))
					CallEvent(warpZone, "Generate");
				if (warpZone && warpZone->OtherSideActor() && warpZone->OtherSideLevel())
				{
					// The clipper only fills polygons wound towards the eye (a mirror frame's are already reversed): a portal
					// seen from behind (a two-sided one, from the reflected camera) is checked with its vertices reversed
					if (eyeBehind)
						std::reverse(points, points + numverts);
					Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
					if (!spans.empty())
					{
						for (auto& p : Portals)
						{
							if (p.WarpZone == warpZone)
							{
								p.Nodes.push_back(info);
								p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
								return;
							}
						}
						VisiblePortal portal;
						portal.Nodes.push_back(info);
						portal.WarpZone = warpZone;
						portal.Spans = std::move(spans);
						Portals.push_back(std::move(portal));
					}
					return;
				}
			}
		}
		else if (PolyFlags & PF_Mirrored)
		{
			// HP1's renderer reflects below portal depth 3 (Lev5_Final's Mirror of Erised is a mirror seen through a
			// warp zone). Inside a portal only a mirror facing the frame's eye: a reflection or warp can see the back
			// of one.
			if (PortalDepth >= 3)
				return;
			if (PortalDepth > 0)
			{
				UModel* model = engine->Level->Model;
				const BspSurface& surface = model->Surfaces[node->Surf];
				if (dot(EyeLocation() - model->Points[surface.pBase], model->Vectors[surface.vNormal]) <= 0.0f)
					return;
			}

			Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
			if (!spans.empty())
			{
				for (auto& p : Portals)
				{
					if (!p.WarpZone && !p.SkyZone && !p.Mover && !p.Nodes.empty())
					{
						const auto* plane = p.Nodes.front().Node;
						if (!KW::SameMirrorPlane(node->PlaneX, node->PlaneY, node->PlaneZ, node->PlaneW,
							plane->PlaneX, plane->PlaneY, plane->PlaneZ, plane->PlaneW))
							continue;
						p.Nodes.push_back(info);
						p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
						return;
					}
				}

				VisiblePortal portal;
				portal.Nodes.push_back(info);
				portal.Spans = std::move(spans);
				Portals.push_back(std::move(portal));
			}
			return;
		}
	}

	if (!Clipper.CheckSurface(points, numverts, (PolyFlags & PF_NoOcclude) == 0))
		return;

	/*if (PolyFlags & PF_Portal)
	{
		ViewZoneMask |= 1ULL << node->Zone0;
		ViewZoneMask |= 1ULL << node->Zone1;
	}*/

	if (PolyFlags & PF_Invisible)
		return;

	if ((PolyFlags & (PF_Translucent | PF_Modulated)) == 0 && (PolyFlags & PF_Occlude) != PF_Occlude)
	{
		OpaqueNodes.push_back(info);
	}
	else
	{
		UModel* model = engine->Level->Model;
		const BspSurface& surface = model->Surfaces[node->Surf];
		vec3 v = model->Points[surface.pBase] - ViewLocation.xyz();
		Translucents.emplace_back(info, dot(v, v));
	}
}

void VisibleFrame::SortTranslucent()
{
	// HP1 draws translucents back to front, so a modulated sprite behind a translucent particle system doesn't
	// darken it (the spell lesson's SpellBlackboard turned the template spiral red).
	if (engine->LaunchInfo.IsKnowWonder())
	{
		std::sort(Translucents.begin(), Translucents.end(), [](const VisibleTranslucent& a, const VisibleTranslucent& b) { return a.DistSqr > b.DistSqr; });
		return;
	}
	std::sort(Translucents.begin(), Translucents.end(), [](const VisibleTranslucent& a, const VisibleTranslucent& b) { return a.DistSqr < b.DistSqr; });
}

int VisibleFrame::FindZoneAt(const vec3& location)
{
	return FindZoneAt(vec4(location, 1.0f), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
}

int VisibleFrame::FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes)
{
	while (true)
	{
		vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
		bool swapFrontAndBack = dot(location, plane) < 0.0f;
		int front = node->Front;
		int back = node->Back;
		if (swapFrontAndBack)
			std::swap(front, back);

		if (front >= 0)
		{
			node = nodes + front;
		}
		else
		{
			return swapFrontAndBack ? node->Zone0 : node->Zone1;
		}
	}
}

void VisibleFrame::Draw()
{
	{ KWProf::Scope prof_(KWProf::Portals); DrawPortals(); }
	{ KWProf::Scope prof_(KWProf::Nodes); DrawOpaqueNodes(); }
	{ KWProf::Scope prof_(KWProf::Actors); DrawOpaqueActors(); }
	{ KWProf::Scope prof_(KWProf::Translucent); SortTranslucent(); DrawTranslucent(); }
}

void VisibleFrame::DrawCoronas()
{
	SceneNode frame2d = Frame;
	frame2d.ObjectToWorld = mat4::identity();
	frame2d.WorldToView = mat4::identity();
	Device->SetSceneNode(&frame2d);

	if (engine->LaunchInfo.IsKnowWonder())
	{
		KW::DrawCoronas(this);
		return;
	}

	for (VisibleCorona &corona : Coronas)
		corona.Draw(this);
}

void VisibleFrame::DrawOpaqueNodes()
{
	Device->SetSceneNode(&Frame);
	for (VisibleNode& node : OpaqueNodes)
		node.Draw(this);
}

void VisibleFrame::DrawOpaqueActors()
{
	Device->SetSceneNode(&Frame);
	for (VisibleActor& actor : Actors)
		actor.DrawOpaque(this);
}

void VisibleFrame::DrawTranslucent()
{
	Device->SetSceneNode(&Frame);
	for (VisibleTranslucent& translucent : Translucents)
		translucent.Draw(this);
}

void VisibleFrame::DrawPortals()
{
	for (VisiblePortal& portal : Portals)
	{
		// BspClipper requires the visible spans list to be sorted
		std::sort(portal.Spans.begin(), portal.Spans.end(), [](const PortalSpan& a, const PortalSpan& b) { return a.y != b.y ? a.y < b.y: a.x0 < b.x0; });

		if (portal.SkyZone)
		{
			mat4 skyToView =
				Coords::ViewToRenderDev().ToMatrix() *
				ViewRotation.Inverse().ToMatrix() *
				Coords::Rotation(portal.SkyZone->Rotation()).ToMatrix() *
				Coords::Location(portal.SkyZone->Location()).ToMatrix();

			VisibleFrame skyframe;
			skyframe.OnlyViewZone = true;
			skyframe.Process(portal.SkyZone->Location(), skyToView, ViewRotation * Coords::Rotation(portal.SkyZone->Rotation()), MirrorFlag, PortalDepth + 1, portal.Spans);
			Device->SetSceneNode(&skyframe.Frame);
			skyframe.Draw();
			Device->ClearZ();
		}
		else if (portal.WarpZone)
		{
			// Warp camera
			// From the frame's eye, the reflected camera in a mirror's frame
			vec3 eye = EyeLocation();
			vec3 newLocation = WarpLocationToOtherSide(portal.WarpZone, eye);
			Coords rotation = WarpRotationToOtherSide(portal.WarpZone, ViewRotation);
			mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * rotation.Inverse().ToMatrix() * mat4::translate(-newLocation);

			// Find clipping plane for portal and warp it
			auto node = portal.Nodes.front().Node;
			vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
			bool isFrontfacing = dot(vec4(eye, 1.0f), plane) < 0.0f;
			vec3 p = WarpLocationToOtherSide(portal.WarpZone, plane.xyz() * -plane.w);
			vec3 n = WarpNormalToOtherSide(portal.WarpZone, vec3(node->PlaneX, node->PlaneY, node->PlaneZ));
			vec4 portalPlane(n, -dot(p,n));
			if (!isFrontfacing)
				portalPlane = -portalPlane;

			VisibleFrame portalframe;
			portalframe.Process(newLocation, worldToView, rotation, MirrorFlag, PortalDepth + 1, portal.Spans, portalPlane);
			Device->SetSceneNode(&portalframe.Frame);
			portalframe.Draw();
			Device->ClearZ();
		}
		else // Mirror
		{
			vec3 v, n;
			vec4 mirrorPlane(0.0f, 0.0f, 0.0f, 1.0f);
			if (portal.Mover)
			{
				// Keep only what is in front of the mirror: the room behind a mover mirror isn't reflected
				v = portal.MirrorBase;
				n = portal.MirrorNormal;
				mirrorPlane = vec4(n, -dot(v, n));
			}
			else
			{
				UModel* model = engine->Level->Model;
				const BspSurface& surface = model->Surfaces[portal.Nodes.front().Node->Surf];
				v = model->Points[surface.pBase];
				n = model->Vectors[surface.vNormal];
				// A free-standing mirror has solid behind it too (Lev4_Sneak2's Mirror of Erised: its wooden back
				// panel covered the whole reflection, James and Lily in it included)
				mirrorPlane = vec4(n, -dot(v, n));
			}
			mat4 mirrorRotation = mat4::mirror(n);
			mat4 mirrorToView = Frame.WorldToView * mat4::translate(v) * mirrorRotation * mat4::translate(-v);

			// The reflection is seen from the camera mirrored behind the mirror plane: the BSP is walked front to back
			// from there (from the real camera, the walls between it and the mirror culled the reflected room)
			vec3 mirrorViewLocation = ViewLocation.xyz();
			mirrorViewLocation -= n * (2.0f * dot(mirrorViewLocation - v, n));

			VisibleFrame mirrorframe;
			mirrorframe.Process(mirrorViewLocation, mirrorToView, ViewRotation * Coords::FromMatrix(mirrorRotation).Inverse(), !MirrorFlag, PortalDepth + 1, portal.Spans, mirrorPlane);
			Device->SetSceneNode(&mirrorframe.Frame);
			mirrorframe.Draw();
			Device->ClearZ();
		}
	}

	// Seal the portals
	Device->SetSceneNode(&Frame);
	for (VisiblePortal& portal : Portals)
	{
		if (portal.Mover)
			VisibleBrush::DrawMirrorSeal(this, portal.Mover, portal.MoverPoly);
		for (const auto& info : portal.Nodes)
		{
			VisibleNode visnode(info);
			visnode.PolyFlags |= PF_Occlude | PF_Invisible;
			visnode.Front = true;
			visnode.Draw(this);

			if (info.PolyFlags & PF_NoOcclude)
			{
				if ((info.PolyFlags & (PF_Translucent | PF_Modulated)) == 0)
				{
					OpaqueNodes.push_back(info);
				}
				else
				{
					UModel* model = engine->Level->Model;
					const BspSurface& surface = model->Surfaces[info.Node->Surf];
					vec3 v = model->Points[surface.pBase] - ViewLocation.xyz();
					Translucents.emplace_back(info, dot(v, v));
				}
			}
		}
	}
}

vec3 VisibleFrame::WarpLocationToOtherSide(UWarpZoneInfo* warpZone, vec3 p)
{
	// Transform to warp space:
	{
		vec3 origin = warpZone->WarpCoords().Origin;
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		p = rotate * (p - origin);
	}

	// Transform from warp space:
	{
		vec3 origin = warpZone->OtherSideActor()->WarpCoords().Origin;
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		p = (invrotate * p) + origin;
	}

	return p;
}

vec3 VisibleFrame::WarpNormalToOtherSide(UWarpZoneInfo* warpZone, vec3 n)
{
	// Transform to warp space:
	{
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		n = invrotate * n;
	}

	// Transform from warp space:
	{
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		n = rotate * n;
	}

	return n;
}

Coords VisibleFrame::WarpRotationToOtherSide(UWarpZoneInfo* warpZone, Coords rotation)
{
	mat3 newRotation = rotation.ToMatrix();

	// Transform to warp space:
	{
		vec3 origin = warpZone->WarpCoords().Origin;
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		newRotation = rotate * newRotation;
	}

	// Transform from warp space:
	{
		vec3 origin = warpZone->OtherSideActor()->WarpCoords().Origin;
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		newRotation = invrotate * newRotation;
	}

	rotation.XAxis = vec3(newRotation[0], newRotation[1], newRotation[2]);
	rotation.YAxis = vec3(newRotation[3], newRotation[4], newRotation[5]);
	rotation.ZAxis = vec3(newRotation[6], newRotation[7], newRotation[8]);
	return rotation;
}
