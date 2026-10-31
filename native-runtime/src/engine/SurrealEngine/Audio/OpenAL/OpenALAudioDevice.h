#pragma once

#include "Audio/AudioDevice.h"
#include "ALSoundSource.h"
#include "RingQueue.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <mutex>
#include <thread>
#include <memory>
#include <chrono>
#include <string>

class OpenALAudioDevice : public AudioDevice
{
public:
	OpenALAudioDevice(int inFrequency, int numVoices, int inMusicBufferCount, int inMusicBufferSize);
	~OpenALAudioDevice();

	int GetTotalChannels() override;
	void AddSound(USound* sound) override;
	void RemoveSound(USound* sound) override;
	bool IsPlaying(int channel) override;
	void PlayMusic(std::unique_ptr<AudioSource> source) override;
	void PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) override;
	void UpdateSound(int channel, vec3& location, float volume, float radius, float pitch) override;
	void StopSound(int channel) override;
	void SetMusicVolume(float volume) override;
	void SetLooping(int channel, bool loop) override;
	void SetLinearFalloff(bool enable, float maxVolume) override;
	void SetInverseFalloff(float rolloff, float dopplerFactor, float speedOfSound) override;
	void SetSoundVelocity(int channel, const vec3& velocity) override;
	void SetListenerVelocity(const vec3& velocity) override;
	void SetPaused(int channel, bool paused) override;
	void Update() override;

private:
	std::string getALErrorString(ALenum error);

	void PlayMusicBuffer(AudioSource* music);
	void UpdateMusicBuffer(AudioSource* music);

	void MusicThreadMain();
	void FollowDefaultDevice();

	ALCdevice* alDevice = nullptr;
	LPALCREOPENDEVICESOFT alcReopenDevice = nullptr;
	std::chrono::steady_clock::time_point nextDeviceCheck;
	std::string reopenedDefault; // the system default the device was last reopened for
	int lostRetrySeconds = 1;
	ALCcontext* alContext = nullptr;
	ALenum alError = 0;
	ALuint alMusicSource = 0;
	Array<ALuint> alMusicBuffers;
	Array<ALSoundSource> sources;
	ALint monoSources = 0;
	ALint stereoSources = 0;

	int frequency = 48000;
	Array<USound*> sounds;

	// OpenAL keeps one error state per context (alGetError), shared by the game and music threads: without this
	// lock each thread read the other's errors as its own and aborted the game, "alBufferData failed in
	// UpdateMusicBuffer: AL_NO_ERROR" on the music thread or "Failed to play AL source" in PlaySound (HP1's
	// Quidditch match, 2026-10-18). Every OpenAL call holds it; the music thread clears the error state each time.
	std::mutex alMutex;

	// Note: variables changed in musicThreadData *must* be done within a mutex lock to be thread safe
	struct
	{
		std::mutex mutex;
		std::thread thread;
		bool exitFlag = false;
		std::unique_ptr<AudioSource> music;
		bool musicUpdate = false;
	} musicThreadData;

	RingQueue<float*> musicQueue;
	int musicBufferCount = 0;
	int musicBufferSize = 0;
	Array<float> musicBuffer;
	float currentMusicVolume = 0.0f;
	float targetMusicVolume = 0.0f;
	float fadeRate = 0.0f;
	// HP2: the listener velocity comes from the audio subsystem (SetListenerVelocity), not the camera actor
	bool ownListenerVelocity = false;
	vec3 listenerVelocity = vec3(0.0f);
};
