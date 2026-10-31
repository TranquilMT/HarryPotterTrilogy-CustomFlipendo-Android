
#include "Precomp.h"
#include "ALSoundSource.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/USound.h"
#include <cfloat>

ALSoundSource::ALSoundSource()
{
	alGenSources(1, &id);
	if (alGetError() != AL_NO_ERROR)
		Exception::Throw("Failed to generate AL source");

	alSourcef(id, AL_ROLLOFF_FACTOR, 1.1f);
}

ALSoundSource::~ALSoundSource()
{
	Stop();
	alSourcei(id, AL_BUFFER, 0);
	alDeleteSources(1, &id);
}

void ALSoundSource::Play()
{
	alSourcePlay(id);
	if (alGetError() != AL_NO_ERROR)
		Exception::Throw("Failed to play AL source");
}

void ALSoundSource::Stop()
{
	alSourceStop(id);
}

bool ALSoundSource::IsPlaying()
{
	if (!alIsSource(id))
		return false;

	ALint state;
	alGetSourcei(id, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING || state == AL_PAUSED; // HP2 pauses its sounds with the game (SetPaused)
}

USound* ALSoundSource::GetSound()
{
	return sound;
}

void ALSoundSource::SetDopplerFactor(float newDopplerFactor)
{
	if (dopplerFactor != newDopplerFactor)
	{
		dopplerFactor = newDopplerFactor;
		alSourcef(id, AL_DOPPLER_FACTOR, dopplerFactor);
	}
}

void ALSoundSource::SetPosition(vec3& newPosition)
{
	position.x = newPosition.x;
	position.y = newPosition.y;
	position.z = -newPosition.z;
	alSourcefv(id, AL_POSITION, &position[0]);
}

void ALSoundSource::SetRadius(float newRadius)
{
	if (radius != newRadius)
	{
		radius = newRadius;
		if (inverseFalloff)
		{
			alSourcef(id, AL_MAX_DISTANCE, FLT_MAX);
			alSourcef(id, AL_REFERENCE_DISTANCE, radius);
			return;
		}
		alSourcef(id, AL_MAX_DISTANCE, radius);
		alSourcef(id, AL_REFERENCE_DISTANCE, linearFalloff ? 0.0f : 0.1f * radius);
	}
}

void ALSoundSource::SetVelocity(vec3& newVelocity)
{
	velocity.x = newVelocity.x;
	velocity.y = newVelocity.y;
	velocity.z = -newVelocity.z;
	alSourcefv(id, AL_VELOCITY, &velocity[0]);
}

void ALSoundSource::SetSound(USound* newSound)
{
	if (sound != newSound)
	{
		sound = newSound;
		alSourcei(id, AL_BUFFER, 0);
		alGetError();
		alSourcei(id, AL_BUFFER, (ALint)(ptrdiff_t)sound->handle);

		if (sound->loopInfo.Looped)
			alSourcei(id, AL_LOOPING, AL_TRUE);
		else
			alSourcei(id, AL_LOOPING, AL_FALSE);
	}
}

// Loops the whole sound when asked, or between its loop points when it has them (SetSound)
void ALSoundSource::SetLooping(bool loop)
{
	alSourcei(id, AL_LOOPING, (loop || (sound && sound->loopInfo.Looped)) ? AL_TRUE : AL_FALSE);
}

void ALSoundSource::SetSpatial(bool bSpatial)
{
	if (bIs3d != bSpatial)
	{
		bIs3d = bSpatial;
		alSourcei(id, AL_SOURCE_SPATIALIZE_SOFT, bIs3d);
	}
}

void ALSoundSource::SetVolume(float newVolume)
{
	if (volume != newVolume)
	{
		volume = newVolume;
		alSourcef(id, AL_GAIN, volume);
		alSourcef(id, AL_MAX_GAIN, linearFalloff ? maxVolume : volume);
	}
}

// Linear falloff: the volume falls in a straight line from the source to the radius, and the volume times
// the falloff is capped at maxVolume (a loud sound stays at full volume further out). Otherwise the
// sound plays at its volume within a tenth of the radius and fades out a little before the radius.
void ALSoundSource::SetLinearFalloff(bool enable, float newMaxVolume)
{
	if (linearFalloff != enable || maxVolume != newMaxVolume)
	{
		linearFalloff = enable;
		maxVolume = newMaxVolume;
		alSourcef(id, AL_ROLLOFF_FACTOR, linearFalloff ? 1.0f : 1.1f);
		radius = -1.0f; // applied again by the next SetRadius and SetVolume
		volume = -1.0f;
	}
}

// HP2's ALAudio driver: full volume within the radius, then radius / (radius + rolloff * (distance - radius)),
// with no distance where it ends (OpenAL's inverse distance clamped model, set for the context)
void ALSoundSource::SetInverseFalloff(float rolloff)
{
	inverseFalloff = true;
	alSourcef(id, AL_ROLLOFF_FACTOR, rolloff);
	radius = -1.0f; // applied again by the next SetRadius
}

void ALSoundSource::SetPaused(bool paused)
{
	ALint state;
	alGetSourcei(id, AL_SOURCE_STATE, &state);
	if (paused && state == AL_PLAYING)
		alSourcePause(id);
	else if (!paused && state == AL_PAUSED)
		alSourcePlay(id);
}

// OpenAL rejects a negative pitch (AL_INVALID_VALUE, which the next checked call then reported as its own
// failure): HP1's BroomHarry.UpdateBroomSound fades the broom loop's pitch below 0 along with its volume.
void ALSoundSource::SetPitch(float newPitch)
{
	newPitch = std::max(newPitch, 0.0f);
	if (pitch != newPitch)
	{
		pitch = newPitch;
		alSourcef(id, AL_PITCH, pitch);
	}
}

void ALSoundSource::DoLoop()
{
	if (sound->loopInfo.Looped)
	{
		ALint offset;
		alGetSourcei(id, AL_SAMPLE_OFFSET, &offset);
		if (offset >= sound->loopInfo.LoopEnd)
			alSourcei(id, AL_SAMPLE_OFFSET, (ALint)sound->loopInfo.LoopStart);
	}
}
