
#include "Precomp.h"
#include "UPawn.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Inventory/UWeapon.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "KW.h"

void UPawn::PollLatentAction(float elapsed)
{
	if (StateFrame)
	{
		LatentRunState latentState = StateFrame->GetLatentState();
		if (latentState == LatentRunState::MoveTo)
		{
			TickRotateTo(Focus());
			if (TickMoveTo(Destination()))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::MoveToward && engine->LaunchInfo.IsKnowWonder()) // APawn::execPollMoveToward (AlterDestination)
		{
			if (KW::PawnPollMoveToward(this))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::MoveToward)
		{
			if (MoveTarget())
			{
				Focus() = MoveTarget()->Location();
				TickRotateTo(Focus());
				if (TickMoveTo(MoveTarget()->Location()))
					StateFrame->SetLatentState(LatentRunState::Continue);
			}
			else
			{
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
		}
		else if (latentState == LatentRunState::StrafeTo)
		{
			TickRotateTo(Focus());
			if (TickMoveTo(Destination()))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::StrafeFacing && engine->LaunchInfo.IsKnowWonder()) // APawn::execPollStrafeFacing (AlterDestination)
		{
			if (KW::PawnPollStrafeFacing(this))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::StrafeFacing)
		{
			if (engine->LaunchInfo.ue1Version > 219 && FaceTarget())
			{
				Focus() = FaceTarget()->Location();
				TickRotateTo(Focus());
				vec3 oldDest = Destination();
				if (TickMoveTo(Destination()))
					StateFrame->SetLatentState(LatentRunState::Continue);
				Destination() = oldDest;
			}
			else if (engine->LaunchInfo.ue1Version <= 219)
			{
				// How did StrafeFacing work in this version? We don't have FaceTarget.
				LogUnimplemented("StrafeFacing not implemented for old versions");
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
			else
			{
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
		}
		else if (latentState == LatentRunState::TurnTo)
		{
			if (engine->LaunchInfo.IsKnowWonder() ? KW::PawnTurnStep(this, Focus()) : TickRotateTo(Focus()))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::TurnToward)
		{
			if (engine->LaunchInfo.ue1Version > 219 && FaceTarget())
			{
				Focus() = FaceTarget()->Location();
				if (engine->LaunchInfo.IsKnowWonder() ? KW::PawnTurnStep(this, Focus()) : TickRotateTo(FaceTarget()->Location()))
					StateFrame->SetLatentState(LatentRunState::Continue);
			}
			else if (engine->LaunchInfo.ue1Version <= 219)
			{
				// How did StrafeFacing work in this version? We don't have FaceTarget.
				LogUnimplemented("TurnToward not implemented for old versions");
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
			else
			{
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
		}
		else if (latentState == LatentRunState::WaitForLanding && engine->LaunchInfo.IsKnowWonder()) // APawn::execPollWaitForLanding (LongFall)
		{
			if (KW::PawnPollWaitForLanding(this, elapsed))
				StateFrame->SetLatentState(LatentRunState::Continue);
		}
		else if (latentState == LatentRunState::WaitForLanding)
		{
			if (Physics() != PHYS_Falling)
			{
				StateFrame->SetLatentState(LatentRunState::Continue);
			}
			else
			{
				// To do: need to send a LongFall event if the fall state lasts long enough
			}
		}
	}
}

void UPawn::Tick(float elapsed)
{
	// KnowWonder's performPhysics (KW::PerformPhysics) counts MoveTimer down and keeps AvgPhysicsTime itself; doing it
	// here as well ran every latent move's timer at double speed (gnomes gave up halfway to their hide spot)
	if (!engine->LaunchInfo.IsKnowWonder() || !KW::UseKWPhysics())
		MoveTimer() -= elapsed;
	if (engine->LaunchInfo.IsKnowWonder() && !KW::UseKWPhysics()) // AvgPhysicsTime for moveToward
		KW::PawnPhysicsTime(this, elapsed);

	// HP1 polls the latent action in ProcessState, after the Tick event (AActor::Tick): a latent move or turn sees
	// where the actor's own Tick put it. BaseCam's RotateAroundHarry moves the camera in Tick and turns it with
	// turntoward; polled before Tick, every other frame aimed from last frame's spot and the view shook.
	if (!engine->LaunchInfo.IsKnowWonder())
		PollLatentAction(elapsed);

	UActor::Tick(elapsed);

	if (bIsPlayer() && Role() >= ROLE_AutonomousProxy)
	{
		if (engine->LaunchInfo.ue1Version < 400 || bViewTarget())
			CallEvent(this, EventName::UpdateEyeHeight, { ExpressionValue::FloatValue(elapsed) });
		else
			ViewRotation() = Rotation();
	}

	if (Weapon())
	{
		Weapon()->Location() = Location();
		Weapon()->UpdateActorZone();
	}

	if (Role() == ROLE_Authority)
	{
		if (PainTime() > 0.0f)
		{
			PainTime() = std::max(PainTime() - elapsed, 0.0f);
			if (PainTime() == 0.0f)
				CallEvent(this, EventName::PainTimer);
		}
		if (SpeechTime() > 0.0f)
		{
			SpeechTime() = std::max(SpeechTime() - elapsed, 0.0f);
			if (SpeechTime() == 0.0f)
				CallEvent(this, EventName::SpeechTimer);
		}
		if (engine->LaunchInfo.ue1Version >= 436 && bAdvancedTactics())
			CallEvent(this, EventName::UpdateTactics, { ExpressionValue::FloatValue(elapsed) });


		SightCounter() -= elapsed;
		if (SightCounter() <= 0.0f)
		{
			//apply random offest to pawns sight counter so not all
			//pawns get evaluated at the same frame 
			//numbers are arbitrarily picked -> if it causes perf. problems, 
			//maybe switch to using some limit of how many pawns can be evaluated 
			//every frame
			SightCounter() += 0.4f - 0.2f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

			//search for target, if the pawn has an event to see them
			if (IsEventEnabled(EventName::SeePlayer))
			{
				//search every pawn in the map, if they are visible fire the related event
				for (UPawn* other = Level()->PawnList(); other; other = other->nextPawn())
				{
					//skip self, dead pawns and the currenty chosen enemy (handeld by the if(Enemy()) branch)
					if (other == this || other == Enemy() || other->Health() <= 0)
						continue;
					//if the enemy pawn is visible, notify the Unreal scrip about it
					if (CanSee(other))
						CallEvent(this, EventName::SeePlayer, { ExpressionValue::ObjectValue(other) });
				}
			}

			//if the pawn already has an enemy, update its tracking data
			if (Enemy())
			{
				if (CanSee(Enemy()))
				{
					LastSeenPos() = Enemy()->Location();
					LastSeeingPos() = Location();
					if (engine->LaunchInfo.ue1Version > 219)
						LastSeenTime() = Level()->TimeSeconds();
				}
				else if (IsEventEnabled(EventName::EnemyNotVisible))
				{
					CallEvent(this, EventName::EnemyNotVisible);
				}
			}
		}
	}
}

void UPawn::TickRotating(float elapsed)
{
	if (engine->LaunchInfo.IsKnowWonder()) // APawn::physicsRotation
	{
		KW::PawnPhysicsRotation(this, elapsed);
		return;
	}

	if (PhysicsMode() == PHYS_Spider)
		return;

	bRotateToDesired() = true;
	bFixedRotationDir() = false;

	if (Rotation() == DesiredRotation())
		return;

	Rotator rot = Rotation();

	if ((DesiredRotation().Yaw & 0xffff) != (rot.Yaw & 0xffff))
	{
		rot.Yaw = Rotator::TurnToShortest(rot.Yaw, DesiredRotation().Yaw, (int)std::abs(RotationRate().Yaw * elapsed));
	}

	if ((DesiredRotation().Pitch & 0xffff) != (rot.Pitch & 0xffff))
	{
		rot.Pitch = DesiredRotation().Pitch & 0xffff;
		if (rot.Pitch < 0x8000)
		{
			rot.Pitch = std::max(rot.Pitch, RotationRate().Pitch);
		}
		else if (rot.Pitch < 0x10000 - RotationRate().Pitch)
		{
			rot.Pitch = 0x10000 - RotationRate().Pitch;
		}
	}

	// To do: apply RotationRate().Roll

	Rotation() = rot;

	if (Rotation() == DesiredRotation())
	{
		CallEvent(this, EventName::EndedRotation);
	}
}

bool UPawn::TickRotateTo(const vec3& target)
{
	if (engine->LaunchInfo.IsKnowWonder()) // APawn::rotateToward
		return KW::PawnRotateToward(this, target);

	if (PhysicsMode() == PHYS_Spider)
		return true;

	DesiredRotation() = Rotator::FromVector(target - Location());

	if (Physics() == PHYS_Walking && (!MoveTarget() || !MoveTarget()->IsA("Pawn")))
	{
		DesiredRotation().Pitch = 0;
	}

	int doneAngle = 2000;
	return (std::abs(DesiredRotation().Yaw - (Rotation().Yaw & 0xffff)) < doneAngle) || (std::abs(DesiredRotation().Yaw - (Rotation().Yaw & 0xffff)) > 0xffff - doneAngle);
}

bool UPawn::TickMoveTo(const vec3& target)
{
	if (engine->LaunchInfo.IsKnowWonder()) // APawn::moveToward
		return KW::PawnMoveToward(this, target);

	if (MoveTimer() < 0.0f)
		return true;

	if (Physics() == PHYS_Walking)
	{
		vec2 delta = target.xy() - Location().xy();
		float distSqr = dot(delta, delta);
		float velocitySqr = dot(Velocity(), Velocity());
		if (distSqr < 1.0f || distSqr < velocitySqr * 0.05f)
			return true;

		Acceleration() = vec3(normalize(delta) * AccelRate(), 0.0f);
	}
	else
	{
		vec3 delta = target - Location();
		float distSqr = dot(delta, delta);
		float velocitySqr = dot(Velocity(), Velocity());
		if (distSqr < 1.0f || distSqr < velocitySqr * 0.05f)
			return true;

		Acceleration() = normalize(delta) * AccelRate();
	}

	return false;
}

void UPawn::MoveTo(const vec3& newDestination, float speed)
{
	MoveTarget() = nullptr;
	bReducedSpeed() = false;
	DesiredSpeed() = clamp(speed, 0.0f, MaxDesiredSpeed());
	Destination() = newDestination;
	Focus() = newDestination;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::MoveTo);
	if (engine->LaunchInfo.IsKnowWonder()) // execMoveTo takes the first step at once
		KW::PawnStartMove(this);
}

void UPawn::MoveToward(UActor* newTarget, float speed)
{
	if (!newTarget)
		return;

	MoveTarget() = newTarget;
	Destination() = newTarget->Location();
	Focus() = newTarget->Location();
	bReducedSpeed() = false;
	DesiredSpeed() = clamp(speed, 0.0f, MaxDesiredSpeed());
	if (UObject::TryCast<UPawn>(newTarget))
		MoveTimer() = engine->LaunchInfo.IsKnowWonder() ? 1.2f : 1.0f;
	else
		SetMoveDuration(newTarget->Location() - Location());
	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::MoveToward);
	if (engine->LaunchInfo.IsKnowWonder()) // execMoveToward takes the first step at once
		KW::PawnStartMove(this);
}

void UPawn::StrafeFacing(const vec3& newDestination, UActor* newTarget)
{
	if (!newTarget)
		return;

	Destination() = newDestination;
	if (engine->LaunchInfo.ue1Version > 219)
		FaceTarget() = newTarget;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::StrafeFacing);
	if (engine->LaunchInfo.IsKnowWonder()) // execStrafeFacing: StrafeTo's speed, faces the target, first step at once
	{
		bReducedSpeed() = false;
		DesiredSpeed() = bIsPlayer() ? MaxDesiredSpeed() : 0.8f * MaxDesiredSpeed();
		Focus() = newTarget->Location();
		KW::PawnStartMove(this);
	}
}

void UPawn::StrafeTo(const vec3& newDestination, const vec3& newFocus)
{
	MoveTarget() = nullptr;
	bReducedSpeed() = false;
	DesiredSpeed() = bIsPlayer() ? MaxDesiredSpeed() : 0.8f * MaxDesiredSpeed();
	Destination() = newDestination;
	Focus() = newFocus;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::StrafeTo);
	if (engine->LaunchInfo.IsKnowWonder()) // execStrafeTo takes the first step at once
		KW::PawnStartMove(this);
}

void UPawn::TurnTo(const vec3& newFocus)
{
	MoveTarget() = nullptr;
	Focus() = newFocus;
	if (engine->LaunchInfo.IsKnowWonder()) // execTurnTo starts turning at once (HP2: Focus flattened while walking)
		KW::PawnStartTurnTo(this);
	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::TurnTo);
}

void UPawn::TurnToward(UActor* newTarget)
{
	if (!newTarget)
		return;

	if (engine->LaunchInfo.ue1Version > 219)
		FaceTarget() = newTarget;
	Focus() = newTarget->Location();
	if (engine->LaunchInfo.IsKnowWonder()) // execTurnToward starts turning at once
		KW::PawnTurnStep(this, Focus());
	// a state frame without code (stopped, or a state with no labels) is never polled in UE1, so the latent
	// turn does nothing. HP1's spell lesson calls Teacher.TurnToward on Tut1Quirrell in his empty 'idle' state; resuming
	// that frame ran off the end ("Unexpected end of code statements").
	if (StateFrame && !(engine->LaunchInfo.IsKnowWonder() && StateFrame->GetLatentState() == LatentRunState::Stop))
		StateFrame->SetLatentState(LatentRunState::TurnToward);
}

void UPawn::WaitForLanding()
{
	if (engine->LaunchInfo.IsKnowWonder()) // APawn::execWaitForLanding: 2.5 s until LongFall, no wait unless falling
	{
		LatentFloat() = 2.5f;
		if (StateFrame && Physics() == PHYS_Falling)
			StateFrame->SetLatentState(LatentRunState::WaitForLanding);
		return;
	}

	if (StateFrame)
		StateFrame->SetLatentState(LatentRunState::WaitForLanding);
}

void UPawn::SetMoveDuration(const vec3& deltaMove)
{
	float scale = DesiredSpeed() * GetSpeed();
	MoveTimer() = scale > 0.0f ? 1.0f + 1.3f * length(deltaMove) / scale : 0.5f;
}

float UPawn::GetSpeed()
{
	switch (PhysicsMode())
	{
	case PHYS_Walking:
	case PHYS_Falling:
	case PHYS_Spider:
		return GroundSpeed();
	case PHYS_Flying:
		return AirSpeed();
	case PHYS_Swimming:
		return WaterSpeed();
	default:
		return 0.0f;
	}
}
