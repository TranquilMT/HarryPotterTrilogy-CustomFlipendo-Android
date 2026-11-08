
#include "Precomp.h"
#include "VisibleActor.h"
#include "VisibleFrame.h"
#include "VisibleBrush.h"
#include "VisibleMesh.h"
#include "VisiblePoaStaticMesh.h"
#include "VisibleSprite.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Actors/Pawn/UPawn.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Engine.h"
#include "KW.h"

void VisibleActor::Process(VisibleFrame* frame, UActor* actor)
{
	// HP1 gathers coronas from the lights around the camera instead (KW::DrawCoronas)
	if (actor->bCorona() && !engine->LaunchInfo.IsKnowWonder())
	{
		if (engine->LaunchInfo.IsDeusEx())
		{
			// There seems to be more to this check than simply the distance. But this will do for now.
			vec3 v = actor->Location() - frame->ViewLocation.xyz();
			float d = 2000.0f;
			if (dot(v, v) < d * d)
				frame->Coronas.push_back(actor);
		}
		else
		{
			frame->Coronas.push_back(actor);
		}
	}

	if (actor->bHidden())
		return;

	UActor* viewportActor = engine->viewport->Actor();
	bool isOwnedByViewport = actor->IsOwnedBy(viewportActor);

	bool behindView = frame->PortalDepth > 0;
	if (UPawn* pawn = UObject::TryCast<UPawn>(viewportActor))
	{
		if (pawn->bBehindView())
			behindView = true;
	}

	if ((behindView || !isOwnedByViewport) && actor->bOnlyOwnerSee())
		return;

	if (engine->LaunchInfo.ue1Version > 219)
	{
		if (!behindView && isOwnedByViewport && actor->bOwnerNoSee())
			return;
	}

	if (!behindView && actor == engine->CameraActor) // Hide CameraActor as if bOwnerNoSee was set for it
		return;

	if (!frame->Clipper.IsAABBVisible(actor->BspInfo.BoundingBox))
		return;

	// A reflection doesn't show what is behind the mirror. The clipper only culls BSP surfaces by the mirror's plane and
	// the GPU doesn't clip, so an actor wholly behind it (the mirror's own mover) would cover the reflection.
	if (frame->MirrorFlag && frame->PortalDepth > 0)
	{
		const BBox& box = actor->BspInfo.BoundingBox;
		const vec4& plane = frame->PortalPlane;
		float maxDist = plane.w + std::max(plane.x * box.min.x, plane.x * box.max.x) + std::max(plane.y * box.min.y, plane.y * box.max.y) + std::max(plane.z * box.min.z, plane.z * box.max.z);
		if (maxDist < 1.0f)
			return;
	}

	EDrawType dt = (EDrawType)actor->DrawType();
	if (engine->LaunchInfo.IsHarryPotter3() && int(dt) == 8 && GetPoaStaticMesh(actor))
    {
        Type = dt; Actor = actor; frame->Actors.push_back(*this);
    }
    else if (dt == DT_Mesh && actor->Mesh())
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
	else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && actor->Texture())
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
	else if (dt == DT_Brush && actor->Brush() && UActor::TryCast<UMover>(actor))
	{
		VisibleBrush::AddMirrorPortals(frame, UActor::Cast<UMover>(actor));
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
	else if (dt == 8 && engine->LaunchInfo.IsKnowWonder()) // DT_Particles (ParticleFX)
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
}

void VisibleActor::DrawOpaque(VisibleFrame* frame)
{
	if (engine->LaunchInfo.IsHarryPotter3() && int(Type) == 8)
    {
        if (DrawPoaStaticMesh(frame, Actor, false))
        {
            vec3 v = Actor->Location() - frame->ViewLocation.xyz();
            frame->Translucents.emplace_back(*this, dot(v, v));
        }
    }
    else if (Type == DT_Mesh)
	{
		VisibleMesh vismesh;
		if (vismesh.DrawMesh(frame, Actor, false, false))
		{
			vec3 v = Actor->Location() - frame->ViewLocation.xyz();
			frame->Translucents.emplace_back(*this, dot(v, v));
		}
	}
	else if (Type == DT_Sprite || Type == DT_SpriteAnimOnce)
	{
		// Assume all sprites are translucent for now
		vec3 v = Actor->Location() - frame->ViewLocation.xyz();
		frame->Translucents.emplace_back(*this, dot(v, v));
	}
	else if (Type == DT_Brush)
	{
		VisibleBrush visbrush;
		if (visbrush.Draw(frame, Actor, false))
		{
			vec3 v = Actor->Location() - frame->ViewLocation.xyz();
			frame->Translucents.emplace_back(*this, dot(v, v));
		}
	}
	else if (Type == 8) // DT_Particles are drawn in the translucent pass
	{
		vec3 v = Actor->Location() - frame->ViewLocation.xyz();
		frame->Translucents.emplace_back(*this, dot(v, v));
	}
}

void VisibleActor::DrawTranslucent(VisibleFrame* frame)
{
	if (engine->LaunchInfo.IsHarryPotter3() && int(Type) == 8)
        DrawPoaStaticMesh(frame, Actor, true);
    else if (Type == DT_Mesh)
	{
		VisibleMesh vismesh;
		vismesh.DrawMesh(frame, Actor, false, true);
	}
	else if (Type == DT_Sprite || Type == DT_SpriteAnimOnce)
	{
		VisibleSprite vissprite;
		vissprite.Draw(frame, Actor);
	}
	else if (Type == DT_Brush)
	{
		VisibleBrush visbrush;
		visbrush.Draw(frame, Actor, true);
	}
	else if (Type == 8) // DT_Particles (ParticleFX)
	{
		KW::DrawParticleSystem(frame, Actor);
	}
}
