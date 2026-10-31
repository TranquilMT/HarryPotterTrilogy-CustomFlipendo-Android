
#include "Precomp.h"
#include "UActor.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Decoration/UDecoration.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Inventory/UInventory.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "VM/ScriptCall.h"
#include "KW.h"
#include "KWMove.h"

void UActor::InitBase()
{
	if (engine->LaunchInfo.ue1Version > 219)
	{
		NameString attachTag = AttachTag();
		if (!attachTag.IsNone())
		{
			// HP2's AttachTag names the actor this one stands on (a statue with AttachTag "StatueBase01" on the pedestal mover tagged
			// StatueBase01): the stock meaning is the reverse (the tagged actors attach to this one), which made the statue the base of its
			// pedestal, so the pedestal slid away alone. NOT checked against HP2's Engine.dll (no IDA database here); read from the map data.
			if (engine->LaunchInfo.IsHarryPotter2())
			{
				for (UActor* levelActor : XLevel()->Actors)
				{
					if (levelActor && levelActor != this && levelActor->Tag() == attachTag)
					{
						SetBase(levelActor, false);
						break;
					}
				}
				return;
			}
			for (UActor* levelActor : XLevel()->Actors)
			{
				if (levelActor && levelActor->Tag() == attachTag)
				{
					SetBase(levelActor, false);
					break;
				}
			}
			return;
		}
	}

	// Find base for certain types
	bool isDecorationInventoryOrPawn = UObject::TryCast<UDecoration>(this) || UObject::TryCast<UInventory>(this) || UObject::TryCast<UPawn>(this);
	if (isDecorationInventoryOrPawn && !ActorBase() && bCollideWorld() && (Physics() == PHYS_None || Physics() == PHYS_Rotating))
	{
		if (engine->LaunchInfo.IsKnowWonder() && KW::UseKWPhysics())
		{
			KW::FindBase(this);
		}
		else
		{
			CollisionHitList hits = XLevel()->Collision.OverlapTest(this);
			if (!hits.empty())
			{
				SetBase(hits.front().Actor, true);
			}
		}
	}

	if (engine->LaunchInfo.ue1Version < 400 && !ActorBase()) // Unreal expects a base to always exist. What about UT? TournamentPlayer seems to indicate not.
	{
		SetBase(Level(), false);
	}
}

void UActor::AddBasedActor(UActor* actor)
{
	if (actor)
		BasedActors.push_back(actor);
}

void UActor::RemoveBasedActor(UActor* actor)
{
	if (!actor)
		return;

	auto it = BasedActors.begin();

	while (it != BasedActors.end())
	{
		if (*it == actor)
		{
			BasedActors.erase(it);
			return;
		}
		it++;
	}
}

void UActor::SetBase(UActor* newBase, bool sendBaseChangeEvent)
{
	if (ActorBase() != newBase)
	{
		if (this->IsBasedOn(newBase))
			return; // don't allow any cycles in the tree

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->RemoveBasedActor(this);
			ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
			CallEvent(ActorBase(), EventName::Detach, { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->AddBasedActor(this);
			ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
			// Note: in the unlikely case of an actor having > 255 bases, StandingCount() won't be an accurate number.
			CallEvent(ActorBase(), EventName::Attach, { ExpressionValue::ObjectValue(this) });
		}

		if (sendBaseChangeEvent)
			CallEvent(this, EventName::BaseChange);
	}
}

void UActor::RelinkBasedActor()
{
	if (ActorBase() && ActorBase() != Level())
	{
		ActorBase()->AddBasedActor(this);
		ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
	}
}

bool UActor::IsBasedOn(UActor* other)
{
	for (UActor* cur = other; cur; cur = cur->ActorBase())
	{
		if (cur == this)
		{
			return true;
		}
	}
	return false;
}
