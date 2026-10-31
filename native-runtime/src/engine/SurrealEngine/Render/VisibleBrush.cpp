
#include "Precomp.h"
#include "VisibleBrush.h"
#include "VisibleFrame.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Level/UPolys.h"

bool VisibleBrush::Draw(VisibleFrame* frame, UActor* actor, bool translucentPass)
{
	UMover* mover = UActor::TryCast<UMover>(actor);
	if (!mover)
		return false;

	engine->render->Stats.Actors++;

	UModel* brush = mover->Brush();
	SceneNode brushframe = frame->Frame;

	engine->Level->Light.UpdateLightList(mover);

	brushframe.ObjectToWorld = BrushToWorld(mover);

	frame->Device->SetSceneNode(&brushframe);
	bool needTranslucentPass = false;

	// A brush polygon is one-sided like a BSP surface: only drawn from in front of its plane, unless PF_TwoSided.
	// Drawing both sides showed a mover's back faces through its masked front (Lev_Tut1b's portcullis: the plank back
	// face filled the gate's holes and hid the doors behind it).
	// The reflected camera in a mirror (Lev4_Sneak2's Mirror of Erised: James and Lily are a polygon in front of the
	// glass facing it, seen only in the reflection)
	vec3 viewLocation = frame->EyeLocation();
	auto facesView = [&](const Poly& poly)
	{
		uint32_t flags = poly.PolyFlags | (poly.Texture ? poly.Texture->PolyFlags() : 0);
		if ((flags & PF_TwoSided) || engine->getEditorMode())
			return true;
		vec3 point = (brushframe.ObjectToWorld * vec4(poly.Base, 1.0f)).xyz();
		vec3 normal = (brushframe.ObjectToWorld * vec4(poly.Normal, 0.0f)).xyz();
		return dot(viewLocation - point, normal) > 0.0f;
	};
	// A mirror polygon is a portal of the frame (AddMirrorPortals), drawn by DrawPortals instead. A translucent one is
	// drawn over its reflection too, as a translucent BSP mirror is (Lev5_Final's Mirror0: its MirrorMain2 frost
	// shows it white-grey; without it the mirror showed a clear reflection)
	auto isMirror = [&](const Poly& poly)
	{
		uint32_t flags = poly.PolyFlags | (poly.Texture ? poly.Texture->PolyFlags() : 0);
		return IsMirrorPortal(frame, flags) && (flags & (PF_Translucent | PF_Modulated)) == 0;
	};
	for (const Poly& poly : brush->Polys->Polys)
	{
		if (facesView(poly) && !isMirror(poly) && DrawBrushPoly(frame, brush, poly, 0, mover, translucentPass))
			needTranslucentPass = true;
	}

	for (const Poly& poly : brush->Polys->Polys)
	{
		if (facesView(poly) && !isMirror(poly) && DrawBrushPoly(frame, brush, poly, 1, mover, translucentPass))
			needTranslucentPass = true;
	}

	frame->Device->SetSceneNode(&frame->Frame);
	return needTranslucentPass;
}

mat4 VisibleBrush::BrushToWorld(UMover* mover)
{
	return mat4::translate(mover->Location()) * Coords::Rotation(mover->Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-mover->PrePivot());
}

// Mirrors are only followed from the main view, as VisibleFrame::ProcessNodeSurface does for BSP mirrors
bool VisibleBrush::IsMirrorPortal(VisibleFrame* frame, uint32_t polyFlags)
{
	return (polyFlags & PF_Mirrored) && frame->PortalDepth == 0 && !engine->getEditorMode();
}

// UE1 inserts mover polygons into the level's BSP every frame, so a mirrored polygon on a mover reflects like a
// mirrored wall. Here movers are drawn as actors, so their mirror polygons are added as portals separately (HP1's
// Lev_Tut3: the mirror in the Alohomora room is the Mirror1 mover, which swings open; without this it drew its
// texture, dark under the translucent MirrorBlur pane in front of it).
void VisibleBrush::AddMirrorPortals(VisibleFrame* frame, UMover* mover)
{
	if (frame->PortalDepth != 0 || engine->getEditorMode())
		return;

	UModel* brush = mover->Brush();
	if (!brush || !brush->Polys)
		return;

	mat4 objectToWorld;
	bool transformReady = false;
	vec3 viewLocation = frame->ViewLocation.xyz();
	const Array<Poly>& polys = brush->Polys->Polys;
	for (int i = 0; i < (int)polys.size(); i++)
	{
		const Poly& poly = polys[i];
		if (!IsMirrorPortal(frame, poly.PolyFlags | (poly.Texture ? poly.Texture->PolyFlags() : 0)) || poly.Vertices.size() < 3)
			continue;

		if (!transformReady)
		{
			objectToWorld = BrushToWorld(mover);
			transformReady = true;
		}

		vec3 base = (objectToWorld * vec4(poly.Base, 1.0f)).xyz();
		vec3 normal = normalize((objectToWorld * vec4(poly.Normal, 0.0f)).xyz());
		if (dot(viewLocation - base, normal) <= 0.0f)
			continue;

		int numverts = (int)poly.Vertices.size();
		vec3* points = engine->render->GetTempVertexBuffer(numverts);
		for (int j = 0; j < numverts; j++)
			points[j] = (objectToWorld * vec4(poly.Vertices[j], 1.0f)).xyz();

		Array<PortalSpan> spans = frame->Clipper.CheckPortal(points, numverts);
		if (spans.empty())
			continue;

		VisiblePortal portal;
		portal.Mover = mover;
		portal.MoverPoly = i;
		portal.MirrorBase = base;
		portal.MirrorNormal = normal;
		portal.Spans = std::move(spans);
		frame->Portals.push_back(std::move(portal));
	}
}

// Fills the depth buffer where the mirror polygon is, after its reflection was drawn
void VisibleBrush::DrawMirrorSeal(VisibleFrame* frame, UMover* mover, int polyIndex)
{
	const Poly& poly = mover->Brush()->Polys->Polys[polyIndex];

	SceneNode brushframe = frame->Frame;
	brushframe.ObjectToWorld = BrushToWorld(mover);
	frame->Device->SetSceneNode(&brushframe);

	UTexture* tex = poly.Texture ? poly.Texture->GetAnimTexture() : engine->LevelInfo->DefaultTexture();
	TextureInfo texture;
	engine->render->UpdateTexture(tex);
	engine->render->UpdateTextureInfo(texture, poly, tex, 0.0f, 0.0f);

	SurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PF_Occlude | PF_Invisible;
	surfaceinfo.Texture = &texture;

	SurfaceFacet facet;
	facet.MapCoords.Origin = poly.Base;
	facet.MapCoords.XAxis = poly.TextureU;
	facet.MapCoords.YAxis = poly.TextureV;
	facet.Vertices = const_cast<vec3*>(poly.Vertices.data());
	facet.VertexCount = (int)poly.Vertices.size();
	frame->Device->DrawComplexSurface(&brushframe, surfaceinfo, facet);

	frame->Device->SetSceneNode(&frame->Frame);
}

bool VisibleBrush::DrawBrushPoly(VisibleFrame* frame, UModel* model, const Poly& poly, int pass, UMover* mover, bool translucentPass)
{
	// The texture's own flags count like the polygon's, as for BSP surfaces (VisibleFrame::ProcessNode). Without them
	// a masked texture on a mover drew its holes black (Lev_Tut1b's portcullis in front of a star).
	uint32_t PolyFlags = poly.PolyFlags;
	if (poly.Texture)
		PolyFlags |= poly.Texture->PolyFlags();

	if (!engine->getEditorMode() && PolyFlags & PolyFlags::PF_Invisible)
		return false;

	bool isTranslucent = (PolyFlags & (PF_Translucent | PF_Modulated | PF_Highlighted)) != 0;
	if (isTranslucent && !translucentPass)
		return true;
	else if (!isTranslucent && translucentPass)
		return false;

	//UpdateTexture(poly.Texture);

	auto zoneActor = mover->Region().Zone;
	if (!zoneActor)
		zoneActor = engine->LevelInfo;
	float ZoneUPanSpeed = zoneActor->TexUPanSpeed();
	float ZoneVPanSpeed = zoneActor->TexVPanSpeed();

	SurfaceInfo surfaceinfo;
	UTexture* polyTex = poly.Texture;
	if (!polyTex)
		polyTex = engine->LevelInfo->DefaultTexture();

	TextureInfo texture;
	TextureInfo detailtex;
	TextureInfo macrotex;

	if (polyTex)
	{
		UTexture* tex = polyTex->GetAnimTexture();
		engine->render->UpdateTexture(tex);
		engine->render->UpdateTextureInfo(texture, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
		surfaceinfo.Texture = &texture;

		if (polyTex->DetailTexture())
		{
			UTexture* tex = polyTex->DetailTexture()->GetAnimTexture();
			engine->render->UpdateTexture(tex);
			engine->render->UpdateTextureInfo(detailtex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.DetailTexture = &detailtex;
		}

		if (polyTex->MacroTexture())
		{
			UTexture* tex = poly.Texture->MacroTexture()->GetAnimTexture();
			engine->render->UpdateTexture(tex);
			engine->render->UpdateTextureInfo(macrotex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.MacroTexture = &macrotex;
		}
	}

	int numverts = (int)poly.Vertices.size();
	const vec3* points = poly.Vertices.data();
	// A reflection flips the winding: reverse the vertices as VisibleNode does for BSP surfaces, or the polygon is culled
	if (frame->MirrorFlag)
	{
		vec3* reversed = engine->render->GetTempVertexBuffer(numverts);
		for (int j = 0; j < numverts; j++)
			reversed[numverts - 1 - j] = poly.Vertices[j];
		points = reversed;
	}

	SurfaceFacet facet;
	facet.MapCoords.Origin = poly.Base;
	facet.MapCoords.XAxis = poly.TextureU;
	facet.MapCoords.YAxis = poly.TextureV;
	facet.Vertices = const_cast<vec3*>(points);
	facet.VertexCount = numverts;

	TextureInfo lightmap;
	if ((PolyFlags & PF_Unlit) == 0)
		lightmap = engine->Level->Light.GetMoverLightmap(mover, poly, zoneActor, model);
	TextureInfo fogmap = engine->Level->Light.GetMoverFogmap(mover, poly, engine->CameraActor->Region().Zone, model);

	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	frame->Device->DrawComplexSurface(&frame->Frame, surfaceinfo, facet);
	return false;
}
