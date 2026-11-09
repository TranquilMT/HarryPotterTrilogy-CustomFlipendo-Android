
#include "Precomp.h"
#include "hp3/PoaBones.h"
#include "UActor.h"
#include "Packages/Core/UClass.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Actors/USpawnNotify.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Info/UGameInfo.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "KW.h"
#include "HP2.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "Collision/BottomLevel/PoaBoxCollision.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

UActor* UActor::Spawn(UClass* SpawnClass, std::optional<UActor*> SpawnOwner, std::optional<NameString> SpawnTag, std::optional<vec3> SpawnLocation, std::optional<Rotator> SpawnRotation)
{
	if (!SpawnClass || SpawnClass->ClsFlags & ClassFlags::Abstract)
	{
		LogMessage("Could not spawn class: " + (SpawnClass ? SpawnClass->Name.ToString() : std::string("null")));
		return nullptr;
	}

	vec3 location = SpawnLocation ? *SpawnLocation : Location();
	Rotator rotation = SpawnRotation ? *SpawnRotation : Rotation();

	float radius = SpawnClass->GetDefaultObject<UActor>()->CollisionRadius();
	float height = SpawnClass->GetDefaultObject<UActor>()->CollisionHeight();
	bool bCollideWorld = SpawnClass->GetDefaultObject<UActor>()->bCollideWorld();
	bool bCollideWhenPlacing = SpawnClass->GetDefaultObject<UActor>()->bCollideWhenPlacing();
	if (bCollideWorld || bCollideWhenPlacing)
	{
		auto defaults=SpawnClass->GetDefaultObject<UActor>();
        const bool box=engine->LaunchInfo.IsHarryPotter3() && PoaCollision::BoxActor(defaults);
        auto extent=PoaCollision::ActorExtent(defaults);if(box&&!extent)return nullptr;
        auto result = box?CheckLocationBox(location,vec3(*extent),true):CheckLocation(location, radius, height, bCollideWorld || bCollideWhenPlacing);
		if (!result.first)
		{
			LogMessage("Could not find usable location when trying to spawn: " + SpawnClass->Name.ToString());
			return nullptr;
		}
		location = result.second;
	}

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	// To do: find unique new name in the package
	static std::map<NameString, int> nextIndex;
	NameString name = SpawnClass->Name.ToString() + std::to_string(nextIndex[SpawnClass->Name]++);
	// The original's names are unique: a map placed Jellybean0 next to a spawned Jellybean0 breaks HP2's persistent
	// actors, which are matched by name
	if (engine->LaunchInfo.IsKnowWonder())
	{
		while (std::any_of(XLevel()->Actors.begin(), XLevel()->Actors.end(), [&](UActor* a) { return a && a->Name == name; }))
			name = SpawnClass->Name.ToString() + std::to_string(nextIndex[SpawnClass->Name]++);
	}
	UActor* actor = UObject::Cast<UActor>(engine->LevelPackage->NewObject(name, UObject::Cast<UClass>(SpawnClass), ObjectFlags::Transient, true));
	if (engine->LaunchInfo.IsKnowWonder()) // the native class constructor runs on the copied defaults
		KW::ActorConstructed(actor);

	actor->Outer() = XLevel()->Outer();
	actor->XLevel() = XLevel();
	actor->Level() = Level();
	actor->Tag() = (SpawnTag && !SpawnTag->IsNone()) ? *SpawnTag : SpawnClass->Name;
	actor->bTicked() = bTicked(); // To do: should it tick in the same world tick it was spawned in or wait until the next one?
	actor->Instigator() = Instigator();
	actor->Brush() = nullptr;
	actor->Location() = location;
	actor->OldLocation() = location;
	actor->Rotation() = rotation;
	actor->Region().Zone = actor->Level();
	actor->Index = (int)XLevel()->Actors.size();
	XLevel()->Actors.push_back(actor);
	XLevel()->Collision.AddToCollision(actor);

	actor->SetOwner(SpawnOwner.has_value() && SpawnOwner.value() ? *SpawnOwner : nullptr);

	if (Level()->bBegunPlay())
	{
		CallEvent(actor, EventName::Spawned);
		CallEvent(actor, EventName::PreBeginPlay);
		CallEvent(actor, EventName::BeginPlay);

		if (actor->bDeleteMe())
		{
			//LogMessage("Object destroyed itself during Spawn!");
			return nullptr;
		}

		// To do: we need to call EventName::EncroachingOn events here?

		actor->InitActorZone();

		CallEvent(actor, EventName::PostBeginPlay);
		CallEvent(actor, EventName::SetInitialState);
		if (engine->LaunchInfo.IsDeusEx())
			CallEvent(actor, "PostPostBeginPlay");

		actor->InitBase();

		if (engine->LaunchInfo.ue1Version >= 400)
		{
			static bool spawnNotificationLocked = false;
			if (!spawnNotificationLocked)
			{
				struct NotificationLockGuard
				{
					NotificationLockGuard() { spawnNotificationLocked = true; }
					~NotificationLockGuard() { spawnNotificationLocked = false; }
				} lockGuard;

				for (USpawnNotify* notifyObj = Level()->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
				{
					UClass* cls = notifyObj->ActorClass();
					if (cls && actor->IsA(cls->Name))
						actor = UObject::Cast<UGameInfo>(CallEvent(notifyObj, EventName::SpawnNotification, { ExpressionValue::ObjectValue(actor) }).ToObject());
				}
			}
		}
	}

	return actor;
}

bool UActor::Destroy()
{
	//engine->LogMessage("UActor.Destroy(" + Class->FriendlyName.ToString() + ")");

	if (bStatic() || bNoDelete())
		return false;
	if (bDeleteMe())
		return true;

	// HP1's ULevel::DestroyActor calls EndState first when the actor is in a state, before the actor is marked deleted
	// (CallEvent skips a deleted actor). Harry's Target draws the spell symbol on a creature
	// (baseChar.bGestureOnTargeting false: imps, Peeves, gnomes) from its EndState when the cast destroys it.
	if (engine->LaunchInfo.IsKnowWonder() && StateFrame && StateFrame->GetFunc() && IsEventEnabled(EventName::EndState))
	{
		InEndState = true; // a GotoState made from this EndState must not send EndState again (UObject::GotoState)
		CallEvent(this, EventName::EndState);
		InEndState = false;
		if (bDeleteMe()) // EndState destroyed it
			return true;
	}

	bDeleteMe() = true;

	if (engine->LaunchInfo.IsKnowWonder()) // free the ParticleFX particle list
		KW::ParticleFXDestroyed(this);

	//GotoState({}, {}); // What should happen to function calls after Destroy() has been called? Razor2 calls SetRoll afterwards!
	SetBase(nullptr, true);

	engine->audiodev->ActorDestroyed(this);

	ULevel* level = XLevel();

	RemoveFromBspNode();
	level->Collision.RemoveFromCollision(this);

	CallEvent(this, EventName::Destroyed);

	if (engine->LaunchInfo.IsUnrealTournament_469())
	{
		for (const auto actor : Touching_UT469())
			if (actor)
				UnTouch(actor);
	}
	else
	{
		for (const auto actor : Touching())
			if (actor)
				UnTouch(actor);
	}


	SetOwner(nullptr);

	if (engine->LaunchInfo.IsHarryPotter2()) // no actor keeps this one as its TickParent / TickParent2
		HP2::ClearTickParents(this);

	while (!ChildActors.empty())
	{
		ChildActors.back()->SetOwner(nullptr);
	}
	while (!BasedActors.empty())
	{
		BasedActors.back()->SetBase(nullptr, true);
	}

	if (Index == -1)
		throw std::runtime_error("Actor index was never set!");
	level->Actors[Index] = nullptr;

	return true;
}

void UActor::Tick(float elapsed)
{
	if (engine->LaunchInfo.IsKnowWonder()) // AParticleFX::Tick, AWind::Tick
	{
		KW::TickNativeActor(this, elapsed);
		if (bDeleteMe())
			return;
	}

	if (engine->LaunchInfo.IsDeusEx())
	{
		DistanceFromPlayer() = length(engine->viewport->Actor()->Location() - Location());
	}

	TickAnimation(elapsed);
	if (engine->LaunchInfo.IsDeusEx())
		TickBlendAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(EventName::Tick))
	{
		CallEvent(this, EventName::Tick, { ExpressionValue::FloatValue(elapsed) });
	}

	if (engine->LaunchInfo.IsKnowWonder()) // HP1's ProcessState: the latent action is polled after the Tick event
		PollLatentAction(elapsed);

	if (StateFrame)
	{
		LatentRunState latentState = StateFrame->GetLatentState();
		if (latentState == LatentRunState::Sleep)
		{
			// the time left lives in LatentFloat (saved with the actor); done once under half a tick
			LatentFloat() -= elapsed;
			if (LatentFloat() < elapsed * 0.5f)
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::FinishInterpolation)
		{
			if (!bInterpolating())
				StateFrame->SetLatentState(LatentRunState::Continue);
		}

		if (Role() >= ROLE_SimulatedProxy && StateFrame->GetLatentState() == LatentRunState::Continue)
		{
			StateFrame->Tick();
		}
	}

	bool timerFirst = engine->LaunchInfo.IsKnowWonder() && KW::UseKWPhysics(); // HP1's AActor::Tick: Timer, then physics
	if (!timerFirst)
		TickPhysics(elapsed);

	if (TimerRate() > 0.0f) // Role() == ROLE_Authority && RemoteRole() == ROLE_AutonomousProxy
	{
		TimerCounter() += elapsed;
		while (TimerRate() > 0.0f && TimerCounter() > TimerRate())
		{
			TimerCounter() -= TimerRate();
			if (!bTimerLoop())
				TimerRate() = 0.0f;
			CallEvent(this, EventName::Timer);
		}
	}

	if (timerFirst && !bDeleteMe())
		TickPhysics(elapsed);

	if(engine->LaunchInfo.IsHarryPotter3()&&!bDeleteMe())PoaBones::UpdateAttachments(this);
	if (engine->LaunchInfo.IsKnowWonder()) // bAnimMove root motion (end of AActor::Tick)
		KW::TickRootMotion(this);
}

bool UActor::Move(const vec3& delta)
{
	return TryMove(delta).Fraction == 1.0f;
}

bool UActor::MoveSmooth(const vec3& delta)
{
	CollisionHit hit = TryMoveSmooth(delta);
	return hit.Fraction != 1.0f;
}

void UActor::MakeNoise(float loudness)
{
	UPawn* noisePawn = UObject::Cast<UPawn>(Instigator());

	if (!noisePawn || Level()->NetMode() == NM_Client)
		return;

	float currentTime = Level()->TimeSeconds();
	vec3 delta1 = noisePawn->noise1spot() - Location();
	vec3 delta2 = noisePawn->noise2spot() - Location();
	if ((noisePawn->noise1time() > currentTime - 0.2f && dot(delta1, delta1) < 2500.0f && noisePawn->noise1loudness() >= 0.9f * loudness) ||
		(noisePawn->noise2time() > currentTime - 0.2f && dot(delta2, delta2) < 2500.0f && noisePawn->noise2loudness() >= 0.9f * loudness))
	{
		return;
	}

	if (noisePawn->noise1time() < currentTime - 0.18f)
	{
		noisePawn->noise1time() = currentTime;
		noisePawn->noise1spot() = Location();
		noisePawn->noise1loudness() = loudness;
	}
	else if (noisePawn->noise2time() < currentTime - 0.18f)
	{
		noisePawn->noise2time() = currentTime;
		noisePawn->noise2spot() = Location();
		noisePawn->noise2loudness() = loudness;
	}
	else if (dot(delta1, delta1) < 2500.0f)
	{
		noisePawn->noise1time() = currentTime;
		noisePawn->noise1spot() = Location();
		noisePawn->noise1loudness() = loudness;
	}
	else if (noisePawn->noise2loudness() <= loudness)
	{
		noisePawn->noise2time() = currentTime;
		noisePawn->noise2spot() = Location();
		noisePawn->noise2loudness() = loudness;
	}

	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		if (pawn != noisePawn && pawn->CanHearNoise(this, loudness))
		{
			CallEvent(pawn, EventName::HearNoise, { ExpressionValue::FloatValue(loudness), ExpressionValue::ObjectValue(this) });
		}
	}
}

bool UActor::PlayerCanSeeMe()
{
	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		if (pawn == this)
			continue;

		vec3 L = Location() - pawn->Location();
		float dist2 = dot(L, L);

		// Too far away
		if (dist2 > 500 * 500)
			continue;

		// Without behind view the pawn can only see in a 75 degree cone in front of them
		if (!pawn->bBehindView())
		{
			vec3 viewDirection = Coords::Rotation(pawn->ViewRotation()).XAxis;
			if (dot(viewDirection, L) < 0.2588190451f * dist2)
				continue;
		}

		// Try check for line of sight
		vec3 eyePos = pawn->Location();
		eyePos.z += pawn->BaseEyeHeight();
		if (pawn->FastTrace(Location(), eyePos))
			return true;
	}
	return false;
}

UTexture* UActor::CreateTextureFromScreenShot(UViewport* vport)
{
	LogUnimplemented("Actor.CreateTextureFromScreenShot");
	return nullptr;
}

UTexture* UActor::CreateTextureFromBMP(const std::string& name, const std::string& filename)
{
	LogUnimplemented("Actor.CreateTextureFromBMP");
	return nullptr;
}

bool UActor::SaveObjectAsFile(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.SaveObjectAsFile");
	return false;
}

bool UActor::LoadObjectAsFile(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.LoadObjectAsFile");
	return false;
}

bool UActor::SaveGameSaveInfo(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.SaveGameSaveInfo");
	return false;
}

bool UActor::LoadGameSaveInfo(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.LoadGameSaveInfo");
	return false;
}

bool UActor::IsOSVer2kOrXP()
{
	return true;
}

void UActor::AIClearEvent(const NameString& eventName)
{
	LogUnimplemented("Actor.AIClearEvent");
}

void UActor::AIClearEventCallback(const NameString& eventName)
{
	LogUnimplemented("Actor.AIClearEventCallback");
}

void UActor::AIEndEvent(const NameString& eventName, uint8_t eventType)
{
	LogUnimplemented("Actor.AIEndEvent");
}

float UActor::AIGetLightLevel(const vec3& Location)
{
	LogUnimplemented("Actor.AIGetLightLevel");
	return 1.0f;
}

void UActor::AISendEvent(const NameString& eventName, uint8_t eventType, std::optional<float> Value, std::optional<float> Radius)
{
	LogUnimplemented("Actor.AISendEvent");
	//LogUnimplemented(Name.ToString() + ": AISendEvent('" + eventName.ToString() + "')");
}

void UActor::AISetEventCallback(const NameString& eventName, const NameString& callback, std::optional<NameString> scoreCallback, std::optional<bool> bCheckVisibility, std::optional<bool> bCheckDir, std::optional<bool> bCheckCylinder, std::optional<bool> bCheckLOS)
{
	LogUnimplemented("Actor.AISetEventCallback");
	//LogMessage(Name.ToString() + ": AISetEventCallback('" + eventName.ToString() + "')");
}

void UActor::AIStartEvent(const NameString& eventName, uint8_t eventType, std::optional<float> Value, std::optional<float> Radius)
{
	LogUnimplemented("Actor.AIStartEvent");
	//LogUnimplemented(Name.ToString() + ": AIStartEvent('" + eventName.ToString() + "')");
}

float UActor::AIVisibility(std::optional<bool> bIncludeVelocity)
{
	LogUnimplemented("Actor.AIVisibility");
	//LogMessage(Name.ToString() + ": AIVisibility");
	return 0.0f;
}

// KnowWonder's AActor::WorldLightRadius is max(DrawScale, 1) * (LightRadius + 1) * 25 (the Quidditch suns,
// DrawScale 5, reach 32000 units). docs/re/engine/lighting.md.
float UActor::WorldLightRadius()
{
	float radius = ((int)LightRadius() + 1) * 25.0f;
	if (engine->LaunchInfo.IsKnowWonder())
		radius *= std::max(DrawScale(), 1.0f);
	return radius;
}
