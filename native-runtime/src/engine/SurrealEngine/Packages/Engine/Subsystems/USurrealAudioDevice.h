#pragma once

#include "UAudioSubsystem.h"
#include "Packages/Engine/Resources/USound.h"
#include "Audio/AudioDevice.h"
#include "Packages/Engine/Actors/UActor.h" // ESoundSlot (an enum without a fixed type can't be declared ahead in standard C++)

class UActor;
class UMusic;
class UViewport;
class AudioDevice;

struct PlayingSound
{
	PlayingSound() = default;
	PlayingSound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, float Priority) : Actor(Actor), Id(Id), Sound(Sound), Location(Location), Volume(Volume), Radius(Radius), Pitch(Pitch), Priority(Priority) {}

	int Id = 0;
	bool IsActive = false;
	float Priority = 0.0f;
	UActor* Actor = nullptr;
	USound* Sound = nullptr;
	vec3 Location = { 0.0f, 0.0f, 0.0f };
	float Volume = 1.0f;
	float Radius = 1.0f;
	float Pitch = 1.0f;
	float StartTime = 0.0f;
	// HP2 (ALAudio.dll): played at the listener instead of the actor (Disable3D, or the player's own sound), looped
	// (PlaySound's Loop), and a StopSound fade-out in progress (FadeOutTime > 0)
	bool No3D = false;
	bool Loop = false;
	float FadeOutTime = 0.0f;
	float FadeElapsed = 0.0f;
	double FadeLastTime = 0.0;
	// HP2 (ALAudio.dll): the actor's mouth channel while a sound with lip sync data plays on it
	UActor* MouthChannel = nullptr;
	double LipSyncStart = -1.0;
	float LipSyncLast = 0.0f;
	// HP2 (ALAudio.dll): its flags (HP2::SoundFlags) and whether the game paused it
	int Flags = 0;
	bool Paused = false;
};

class USurrealAudioDevice : public UAudioSubsystem
{
public:
	using UAudioSubsystem::UAudioSubsystem;

	void LoadProperties(const NameString& from = "") override;
	void SaveConfig() override;

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;

	std::string Class = "Engine.SurrealAudioDevice";

	void InitDevice();
	void ShutdownDevice();
	void SetViewport(UViewport* InViewport);
	void Update();

	bool PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, bool no3D = false, bool loop = false);
	void StopSound(UActor* Actor, int Id);
	// HP1's ModifySound / StopSound(Sound, Slot): first sound whose Id matches (ignoring bit 0) and, if given, Sound
	bool ModifySoundHP1(int Id, USound* Sound, int Param, float Value);
	bool StopSoundHP1(int Id, USound* Sound);
	// HP2's StopSound (ALAudio.dll): every sound of the actor whose Id matches (ignoring bit 0; slot None: every
	// sound of the actor in slot None) and, if given, Sound; faded out over FadeOutTime when it is above 0
	bool StopSoundHP2(UActor* Actor, int Id, USound* Sound, float FadeOutTime);
	void ActorDestroyed(UActor* Actor);
	void StopSounds();

	void BreakpointTriggered();
	void AddStats(Array<std::string>& lines);

	void SetInstantMusicVolume(uint8_t newMusicVolume);
	void SetInstantSoundVolume(uint8_t newSoundVolume);
	void SetInstantSpeechVolume(uint8_t newSpeechVolume);

	void PlayMusic(std::unique_ptr<AudioSource> source);
	void StopMusic();

	int CreateSoundId(UActor* actor, ESoundSlot slot, bool noOverride);
	static bool IsSoundInterruptable(int id);
	static ESoundSlot GetSoundSlot(int id);
	static bool IsSameSoundId(int id1, int id2);

	// For HP2's audio driver (src/hp2/HP2Audio.cpp) and its front end's volume settings
	UViewport* GetCurrentViewport() { return m_Viewport; }
	Array<PlayingSound>& GetPlayingSounds() { return PlayingSounds; }
	void StopSoundAt(size_t index) { StopSound(index); }
	void SetSoundPaused(int channel, bool paused) { m_Device->SetPaused(channel, paused); }
	void SetSoundVelocity(int channel, const vec3& velocity) { m_Device->SetSoundVelocity(channel, velocity); }
	void SetListenerVelocity(const vec3& velocity) { m_Device->SetListenerVelocity(velocity); }
	uint8_t GetMusicVolume() const { return MusicVolume; }
	uint8_t GetSoundVolume() const { return SoundVolume; }
	void SetMusicVolume(uint8_t volume) { MusicVolume = volume; }
	void SetSoundVolume(uint8_t volume) { SoundVolume = volume; }

private:
	AudioDevice* GetDevice() { return m_Device.get(); } // DO NOT MAKE THIS PUBLIC!!!!
	friend class USound; // To do: figure out what USound::GetSound() is doing and why it wants to talk directly to the device (it shouldn't)

	UViewport* GetViewport() { return m_Viewport; }

	void StartAmbience();
	void UpdateAmbience();
	void UpdateSounds();

	void UpdateLipSync(PlayingSound& Playing);
	void ClearLipSync(PlayingSound& Playing);
	void UpdateMusic();
	void StopSound(size_t index);

	static std::string VolumeToString(uint8_t volume);
	static uint8_t VolumeFromString(const std::string& value, uint8_t defaultValue);
	static float SoundPriority(UViewport* Viewport, vec3 Location, float Volume, float Radius);

	UViewport* m_Viewport = nullptr;

	std::unique_ptr<AudioDevice> m_Device;
	Array<PlayingSound> PlayingSounds;
	UMusic* CurrentSong = nullptr;
	int CurrentSection = 255;
	int FreeSlot = 0x07ffffff;

	bool UseFilter = true;
	bool UseSurround = true;
	bool UseStereo = true;
	bool UseCDMusic = false;
	bool UseDigitalMusic = true;
	bool UseSpatial = true;
	bool UseReverb = true;
	bool Use3dHardware = true;
	bool LowSoundQuality = false;
	bool ReverseStereo = false;
	int Latency = 40;
	AudioFrequency OutputRate = 44100;
	int Channels = 16;
	uint8_t MusicVolume = 160;
	uint8_t SoundVolume = 200;
	uint8_t SpeechVolume = 200;
	float AmbientFactor = 0.7f;
};
