
#include "Precomp.h"
#include "UActor.h"
#include "Engine.h"
#include "KW.h"

void UActor::TickTrailer(float elapsed)
{
	if (engine->LaunchInfo.IsKnowWonder()) // physTrailer with AnimBone (AttachToOwner) and HP1's rotation rules
	{
		KW::PhysTrailer(this);
		return;
	}

	if (!Owner())
		return;

	vec3 newLocation = Owner()->Location();

	if (engine->LaunchInfo.ue1Version >= 400 && bTrailerPrePivot())
	{
		newLocation += PrePivot();
	}

	SetLocation(newLocation);

	if ((engine->LaunchInfo.ue1Version < 400 || bTrailerSameRotation()) && DrawType() != DT_Sprite)
	{
		SetRotation(Owner()->Rotation());
	}
}
