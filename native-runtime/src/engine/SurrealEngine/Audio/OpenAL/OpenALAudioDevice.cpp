
#include "Precomp.h"
#include "OpenALAudioDevice.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/USound.h"
#include "Utils/Exception.h"
#include "Engine.h"
#include <cmath>
#include <chrono>

#define UU_PER_METER 43

OpenALAudioDevice::OpenALAudioDevice(int inFrequency, int numVoices, int inMusicBufferCount, int inMusicBufferSize)
{
	frequency = inFrequency;
	musicBufferCount = inMusicBufferCount;
	musicBufferSize = inMusicBufferSize;

	musicBuffer.resize(musicBufferSize * musicBufferCount);
	musicQueue.Resize(musicBufferCount);

	for (int i = 0; i < musicBufferCount; i++)
	{
		musicQueue.Push(&musicBuffer[musicBufferSize * i]);
		musicQueue.Pop();
	}

	// Init OpenAL
	// TODO: Add device enumeration
	alDevice = alcOpenDevice(nullptr);
	if (alDevice == nullptr)
		Exception::Throw("Failed to initialize OpenAL device");

	const ALCint ctxAttribs[] =
	{
		ALC_FREQUENCY, inFrequency,
		ALC_REFRESH, 60,
		ALC_SYNC, ALC_FALSE
	};

	alContext = alcCreateContext(alDevice, nullptr);
	if (alContext == nullptr)
		Exception::Throw("Failed to initialize OpenAL context");

	if (alcMakeContextCurrent(alContext) == ALC_FALSE)
		Exception::Throw("Failed to make OpenAL context current");

	if (alcIsExtensionPresent(alDevice, "ALC_SOFT_reopen_device"))
		alcReopenDevice = (LPALCREOPENDEVICESOFT)alcGetProcAddress(alDevice, "alcReopenDeviceSOFT");

	// init listener state
	ALfloat listenerOri[] = { 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f };
	alListener3f(AL_POSITION, 0, 0, 0.0f);
	alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alListenerf(AL_METERS_PER_UNIT, 1.f / UU_PER_METER);

	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	alSpeedOfSound(343.3f / (1.0f / UU_PER_METER));

	// Init sound sources
	alcGetIntegerv(alDevice, ALC_MONO_SOURCES, 1, &monoSources);
	alcGetIntegerv(alDevice, ALC_STEREO_SOURCES, 1, &stereoSources);

	sources.resize(std::min(numVoices, monoSources));

	// init music source/buffer
	alGenSources(1, &alMusicSource);
	alSourcei(alMusicSource, AL_SOURCE_SPATIALIZE_SOFT, AL_FALSE);

	alMusicBuffers.resize(musicBufferCount);
	alGenBuffers(musicBufferCount, &alMusicBuffers[0]);

	// init playback thread
	musicThreadData.thread = std::thread([this]() { MusicThreadMain(); });
}

OpenALAudioDevice::~OpenALAudioDevice()
{
	std::unique_lock lock(musicThreadData.mutex);
	musicThreadData.exitFlag = true;
	lock.unlock();
	musicThreadData.thread.join();

	alSourceStop(alMusicSource);
	alDeleteSources(1, &alMusicSource);

	alDeleteBuffers((ALsizei)alMusicBuffers.size(), &alMusicBuffers[0]);

	sources.clear();

	for (USound* sound : sounds)
	{
		alDeleteBuffers(1, (ALuint*)&sound->handle);
	}

	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
}

int OpenALAudioDevice::GetTotalChannels()
{
	return (int)sources.size();
}

void OpenALAudioDevice::AddSound(USound* sound)
{
	std::lock_guard alLock(alMutex);
	sounds.push_back(sound);

	ALenum format = AL_FORMAT_MONO_FLOAT32;
	if (sound->channels == 2)
		format = AL_FORMAT_STEREO_FLOAT32;

	ALuint id;
	alGenBuffers(1, &id);
	alBufferData(id, format, sound->samples.data(), (ALsizei)(sound->samples.size() * sizeof(sound->samples[0])), sound->frequency);
	alError = alGetError();
	if (alError != AL_NO_ERROR)
		Exception::Throw("Failed to buffer sound data for " + sound->Name.ToString());

	sound->handle = (void*)(ptrdiff_t)id;
}

void OpenALAudioDevice::RemoveSound(USound* sound)
{
	std::lock_guard alLock(alMutex);
	auto it = sounds.begin();
	while (it != sounds.end())
	{
		if (*it == sound)
		{
			// TODO: find sources playing this sound and stop them?
			sounds.erase(it);
			alDeleteBuffers(1, reinterpret_cast<const ALuint*>(&sound->handle));
			return;
		}
	}
}

bool OpenALAudioDevice::IsPlaying(int channel)
{
	std::lock_guard alLock(alMutex);
	ALSoundSource& source = sources[channel];
	return source.IsPlaying();
}

void OpenALAudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
	std::unique_lock lock(musicThreadData.mutex);
	musicThreadData.music = std::move(source);
	musicThreadData.musicUpdate = true;
}

void OpenALAudioDevice::PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
{
	std::lock_guard alLock(alMutex);
	if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
		Exception::Throw("Invalid PlaySound arguments");

	ALSoundSource& source = sources[channel];
	if (source.IsPlaying())
	{
		LogMessage("Attempted to play sound on active channel " + std::to_string(channel));
		return;
	}

	source.SetSound(sound);
	source.SetPosition(location);
	source.SetVolume(volume);
	source.SetRadius(radius);
	source.SetPitch(pitch);
	source.SetSpatial(true);
	source.Play();
}

void OpenALAudioDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
	std::lock_guard alLock(alMutex);
	if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
		Exception::Throw("Invalid PlaySound arguments");

	ALSoundSource& source = sources[channel];

	source.SetPosition(location);
	source.SetVolume(volume);
	source.SetRadius(radius);
	source.SetPitch(pitch);

	source.DoLoop();
}

void OpenALAudioDevice::StopSound(int channel)
{
	std::lock_guard alLock(alMutex);
	if (channel >= sources.size())
		Exception::Throw("Invalid StopSound arguments");

	sources[channel].Stop();
}

void OpenALAudioDevice::SetLooping(int channel, bool loop)
{
	std::lock_guard alLock(alMutex);
	if (channel < (int)sources.size())
		sources[channel].SetLooping(loop);
}

std::string OpenALAudioDevice::getALErrorString(ALenum error)
{
	switch (error)
	{
	case AL_NO_ERROR:
		return "AL_NO_ERROR";
	case AL_INVALID_NAME:
		return "AL_INVALID_NAME";
	case AL_INVALID_ENUM:
		return "AL_INVALID_ENUM";
	case AL_INVALID_VALUE:
		return "AL_INVALID_VALUE";
	case AL_INVALID_OPERATION:
		return "AL_INVALID_OPERATION";
	case AL_OUT_OF_MEMORY:
		return "AL_OUT_OF_MEMORY";
	default:
		return "AL_UNKNOWN_ERROR";
	}
}

void OpenALAudioDevice::PlayMusicBuffer(AudioSource* music)
{
	int format = (music->GetChannels() == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
	int freq = music->GetFrequency();

	ALenum error;
	for (int i = 0; i < musicBufferCount; i++)
	{
		alBufferData(alMusicBuffers[i], format, musicQueue.Pop(), musicBufferSize * 4, freq);
		if ((error = alGetError()) != AL_NO_ERROR)
			Exception::Throw("alBufferData failed in PlayMusicBuffer: " + getALErrorString(error));

		alSourceQueueBuffers(alMusicSource, 1, &alMusicBuffers[i]);
		if ((error = alGetError()) != AL_NO_ERROR)
			Exception::Throw("alSourceQueueBuffers failed in PlayMusicBuffer: " + getALErrorString(error));
	}

	alSourcePlay(alMusicSource);
	if ((error = alGetError()) != AL_NO_ERROR)
		Exception::Throw("alSourcePlay failed in PlayMusicBuffer: " + getALErrorString(error));
}

void OpenALAudioDevice::UpdateMusicBuffer(AudioSource* music)
{
	ALenum error;
	ALint status;
	alGetSourcei(alMusicSource, AL_BUFFERS_PROCESSED, &status);

	while (status)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(alMusicSource, 1, &buffer);

		int format = (music->GetChannels() == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
		int freq = music->GetFrequency();

		alBufferData(buffer, format, musicQueue.Pop(), musicBufferSize * 4, freq);
		if ((error = alGetError()) != AL_NO_ERROR)
			Exception::Throw("alBufferData failed in UpdateMusicBuffer: " + getALErrorString(error));

		alSourceQueueBuffers(alMusicSource, 1, &buffer);
		if ((error = alGetError()) != AL_NO_ERROR)
			Exception::Throw("alSourceQueueBuffers failed in UpdateMusicBuffer: " + getALErrorString(error));

		status--;
	}

	alGetSourcei(alMusicSource, AL_SOURCE_STATE, &status);
	if (status == AL_STOPPED)
	{
		alSourcePlay(alMusicSource);
		if ((error = alGetError()) != AL_NO_ERROR)
			Exception::Throw("alSourcePlay failed in PlayMusicBuffer: " + getALErrorString(error));
	}
}

void OpenALAudioDevice::SetMusicVolume(float volume)
{
	std::lock_guard alLock(alMutex);
	alSourcef(alMusicSource, AL_GAIN, volume);
}

void OpenALAudioDevice::SetLinearFalloff(bool enable, float maxVolume)
{
	std::lock_guard alLock(alMutex);
	for (auto& soundSource : sources)
		soundSource.SetLinearFalloff(enable, maxVolume);
}

void OpenALAudioDevice::SetInverseFalloff(float rolloff, float dopplerFactor, float speedOfSound)
{
	std::lock_guard alLock(alMutex);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alDopplerFactor(dopplerFactor);
	alSpeedOfSound(speedOfSound);
	for (auto& soundSource : sources)
		soundSource.SetInverseFalloff(rolloff);
	ownListenerVelocity = true;
}

void OpenALAudioDevice::SetSoundVelocity(int channel, const vec3& velocity)
{
	std::lock_guard alLock(alMutex);
	if (channel < (int)sources.size())
	{
		vec3 v = velocity;
		sources[channel].SetVelocity(v);
	}
}

void OpenALAudioDevice::SetListenerVelocity(const vec3& velocity)
{
	std::lock_guard alLock(alMutex);
	listenerVelocity = velocity;
}

void OpenALAudioDevice::SetPaused(int channel, bool paused)
{
	std::lock_guard alLock(alMutex);
	if (channel < (int)sources.size())
		sources[channel].SetPaused(paused);
}

void OpenALAudioDevice::Update()
{
	std::lock_guard alLock(alMutex);
	UActor* listener = engine->CameraActor;
	if (listener)
	{
		// Update listener properties: where the camera is, as PlayerCalcView left it (what is drawn). The CameraActor
		// itself is only the actor the script named; HP2's third person camera is not at its location, and sounds with a
		// radius (footsteps, spells, pickups) played from a listener that was elsewhere.
		vec3 location = engine->CameraLocation;
		vec3& velocity = ownListenerVelocity ? listenerVelocity : listener->Velocity();

		vec3 at, left, up;
		Coords::Rotation(engine->CameraRotation).GetAxes(at, left, up);

		ALfloat listenerOri[6] = { up.x, up.y, -up.z, -at.x, -at.y, at.z };
		alListener3f(AL_POSITION, location.x, location.y, -location.z);
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, -velocity.z);
		alListenerfv(AL_ORIENTATION, listenerOri);
	}
}

void OpenALAudioDevice::MusicThreadMain()
{
	std::unique_ptr<AudioSource> currentMusic;
	bool musicPlaying = false;
	int musicFailures = 0;

	while (true)
	{
		// Lock the mutex. Grab the data we need from the main thread. Then unlock.
		std::unique_lock lock(musicThreadData.mutex);
		if (musicThreadData.exitFlag)
			break;
		if (musicThreadData.musicUpdate)
		{
			if (musicPlaying)
			{
				std::lock_guard alLock(alMutex);
				alSourceStop(alMusicSource);
				alSourceUnqueueBuffers(alMusicSource, (ALsizei)alMusicBuffers.size(), &alMusicBuffers[0]);
				alGetError();
				musicPlaying = false;
			}
			currentMusic = std::move(musicThreadData.music);
			musicThreadData.musicUpdate = false;
			musicFailures = 0;
		}
		lock.unlock();
		// Never touch anything from musicThreadData after this point.

		using namespace std::chrono_literals;
		if (currentMusic)
		{
			// An exception leaving this thread ends the game (std::terminate), so a music error only stops the
			// music: the source is reset and the track restarted once a second; a track that keeps failing is
			// dropped.
			try
			{
				while (musicQueue.Size() < musicBufferCount)
				{
					// render a chunk of music
					// silence after the end of the stream (the buffer still holds an older chunk)
					float* chunk = musicQueue.GetNextFree();
					size_t samplesRead = currentMusic->ReadSamples(chunk, musicBufferSize);
					std::fill(chunk + samplesRead, chunk + musicBufferSize, 0.0f);
					musicQueue.Push(chunk);
				}

				std::lock_guard alLock(alMutex);
				alGetError(); // an error left by a main thread call isn't ours
				if (!musicPlaying)
				{
					PlayMusicBuffer(currentMusic.get());
					musicPlaying = true;
				}
				else if (musicQueue.Size() > 0)
				{
					UpdateMusicBuffer(currentMusic.get());
					musicFailures = 0;
				}

				// TODO: music fade in/out
			}
			catch (const std::exception& e)
			{
				std::string message = e.what();
				message = message.substr(0, message.find('\n'));
				{
					std::lock_guard alLock(alMutex);
					alSourceStop(alMusicSource);
					alSourcei(alMusicSource, AL_BUFFER, 0);
					alGetError();
				}
				musicPlaying = false;
				if (++musicFailures >= 5)
				{
					LogMessage("Music stopped, the track keeps failing: " + message);
					currentMusic.reset();
					musicFailures = 0;
				}
				else
				{
					if (musicFailures == 1)
						LogMessage("Music error, restarting the stream: " + message);
					std::this_thread::sleep_for(1s);
				}
			}
		}
		else
		{
			musicPlaying = false;
		}

		{
			std::lock_guard alLock(alMutex);
			FollowDefaultDevice();
			alGetError();
		}

		std::this_thread::sleep_for(5ms);
	}
}

// OpenAL Soft 1.23 stays on the device it opened: unplugging it leaves the game silent and a new system default
// (headphones plugged back in) is never picked up. Once a second, reopen the default device when ours was lost or
// the default changed; alcReopenDeviceSOFT keeps the context, sources and buffers. A default is reopened for once:
// when OpenAL can't open it and stays on another device, the names keep differing, and reopening every second
// for that (seen 2026-10-12, over 1000 times in a row) ended in WASAPI errors. A lost device is retried less and
// less often, up to every 30 s.
void OpenALAudioDevice::FollowDefaultDevice()
{
	auto now = std::chrono::steady_clock::now();
	if (now < nextDeviceCheck)
		return;
	nextDeviceCheck = now + std::chrono::seconds(1);

	if (!alcReopenDevice)
		return;

	ALCint connected = ALC_TRUE;
	alcGetIntegerv(alDevice, ALC_CONNECTED, 1, &connected);
	const ALCchar* current = alcGetString(alDevice, ALC_ALL_DEVICES_SPECIFIER);
	const ALCchar* defaultName = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
	std::string newDefault = defaultName ? defaultName : "";
	bool defaultChanged = current && !newDefault.empty() && newDefault != current && newDefault != reopenedDefault;
	if (connected)
		lostRetrySeconds = 1;
	if (connected && !defaultChanged)
		return;

	reopenedDefault = newDefault;
	if (!connected)
	{
		nextDeviceCheck = now + std::chrono::seconds(lostRetrySeconds);
		lostRetrySeconds = std::min(lostRetrySeconds * 2, 30);
	}
	if (alcReopenDevice(alDevice, nullptr, nullptr))
		LogMessage(std::string("Audio device: now ") + alcGetString(alDevice, ALC_ALL_DEVICES_SPECIFIER) + (connected ? "" : " (the last one was lost)") + ", system default " + newDefault);
}
