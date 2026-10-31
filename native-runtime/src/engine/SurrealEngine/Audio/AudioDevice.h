#pragma once

#include "AudioSource.h"

#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include "Math/vec.h"

class AudioSource;
class USound;

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize);

	virtual ~AudioDevice() = default;
	virtual void AddSound(USound* sound) = 0;
	virtual void RemoveSound(USound* sound) = 0;
	virtual bool IsPlaying(int channel) = 0;
	virtual int GetTotalChannels() = 0;
	virtual void PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void UpdateSound(int channel, vec3& location, float volume, float radius, float pitch) = 0;
	virtual void StopSound(int channel) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetLinearFalloff(bool enable, float maxVolume) { } // see ALSoundSource::SetLinearFalloff
	virtual void SetLooping(int channel, bool loop) { } // loop the channel's sound even without loop points (HP2's PlaySound Loop)
	// HP2's ALAudio driver: OpenAL's inverse distance clamped model (see ALSoundSource::SetInverseFalloff), doppler
	// from the sources' and the listener's velocities, and its sounds paused with the game
	virtual void SetInverseFalloff(float rolloff, float dopplerFactor, float speedOfSound) { }
	virtual void SetSoundVelocity(int channel, const vec3& velocity) { }
	virtual void SetListenerVelocity(const vec3& velocity) { }
	virtual void SetPaused(int channel, bool paused) { }
	virtual void Update() = 0;
};
