
#include "Precomp.h"
#include "USurrealAudioDevice.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Package/IniProperty.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Pawn/UPawn.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include <chrono>
#include "Packages/Engine/Resources/UMusic.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "KW.h"
#include "HP2.h"
#include "HP1.h"

static float square(float x) { return x * x; }

// HP2's scripts read and write the volumes as floats 0..1, as its ALAudio driver keeps them (src/hp2/HP2Music.cpp)
std::string USurrealAudioDevice::VolumeToString(uint8_t volume)
{
	if (engine->LaunchInfo.IsHarryPotter2())
		return HP2::VolumeToIni(volume);
	return IniPropertyConverter<uint8_t>::ToString(volume);
}

uint8_t USurrealAudioDevice::VolumeFromString(const std::string& value, uint8_t defaultValue)
{
	if (engine->LaunchInfo.IsHarryPotter2())
		return HP2::VolumeFromIni(value, defaultValue);
	return IniPropertyConverter<uint8_t>::FromString(value);
}

std::string USurrealAudioDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "UseFilter")
		return IniPropertyConverter<bool>::ToString(UseFilter);
	else if (propertyName == "UseSurround")
		return IniPropertyConverter<bool>::ToString(UseSurround);
	else if (propertyName == "UseStereo")
		return IniPropertyConverter<bool>::ToString(UseStereo);
	else if (propertyName == "UseCDMusic")
		return IniPropertyConverter<bool>::ToString(UseCDMusic);
	else if (propertyName == "UseDigitalMusic")
		return IniPropertyConverter<bool>::ToString(UseDigitalMusic);
	else if (propertyName == "UseSpatial")
		return IniPropertyConverter<bool>::ToString(UseSpatial);
	else if (propertyName == "UseReverb")
		return IniPropertyConverter<bool>::ToString(UseReverb);
	else if (propertyName == "Use3dHardware")
		return IniPropertyConverter<bool>::ToString(Use3dHardware);
	else if (propertyName == "LowSoundQuality")
		return IniPropertyConverter<bool>::ToString(LowSoundQuality);
	else if (propertyName == "ReverseStereo")
		return IniPropertyConverter<bool>::ToString(ReverseStereo);
	else if (propertyName == "Latency")
		return IniPropertyConverter<int>::ToString(Latency);
	else if (propertyName == "OutputRate")
		return IniPropertyConverter<AudioFrequency>::ToString(OutputRate);
	else if (propertyName == "Channels")
		return IniPropertyConverter<int>::ToString(Channels);
	else if (propertyName == "MusicVolume")
		return VolumeToString(MusicVolume);
	else if (propertyName == "SoundVolume")
		return VolumeToString(SoundVolume);
	else if (propertyName == "SpeechVolume")
		return IniPropertyConverter<uint8_t>::ToString(SpeechVolume);
	else if (propertyName == "AmbientFactor")
		return IniPropertyConverter<float>::ToString(AmbientFactor);

	LogMessage("Queried unknown property for SurrealAudioDevice: " + propertyName.ToString());
	return {};
}

void USurrealAudioDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "UseFilter")
		UseFilter = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSurround")
		UseSurround = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseStereo")
		UseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseCDMusic")
		UseCDMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseDigitalMusic")
		UseDigitalMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSpatial")
		UseSpatial = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseReverb")
		UseReverb = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Use3dHardware")
		Use3dHardware = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "LowSoundQuality")
		LowSoundQuality = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "ReverseStereo")
		ReverseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Latency")
		Latency = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "OutputRate")
		OutputRate = IniPropertyConverter<AudioFrequency>::FromString(value);
	else if (propertyName == "Channels")
		Channels = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "MusicVolume")
		MusicVolume = VolumeFromString(value, MusicVolume);
	else if (propertyName == "SoundVolume")
		SoundVolume = VolumeFromString(value, SoundVolume);
	else if (propertyName == "SpeechVolume")
		SpeechVolume = IniPropertyConverter<uint8_t>::FromString(value);
	else if (propertyName == "AmbientFactor")
		AmbientFactor = IniPropertyConverter<float>::FromString(value);
	else
		LogMessage("Setting unknown property for SurrealAudioDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealAudioDevice::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	UseFilter = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseFilter", UseFilter);
	UseSurround = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSurround", UseSurround);
	UseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseStereo", UseStereo);
	UseCDMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseCDMusic", UseCDMusic);
	UseDigitalMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDigitalMusic", UseDigitalMusic);
	UseSpatial = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSpatial", UseSpatial);
	UseReverb = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseReverb", UseReverb);
	Use3dHardware = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Use3dHardware", Use3dHardware);
	LowSoundQuality = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "LowSoundQuality", LowSoundQuality);
	ReverseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ReverseStereo", ReverseStereo);
	Latency = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Latency", Latency);
	OutputRate = IniPropertyConverter<AudioFrequency>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "OutputRate", OutputRate);
	Channels = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Channels", Channels);
	MusicVolume = VolumeFromString(engine->packages->GetIniFile("System")->GetValue(name_from, "MusicVolume", VolumeToString(MusicVolume)), MusicVolume);
	SoundVolume = VolumeFromString(engine->packages->GetIniFile("System")->GetValue(name_from, "SoundVolume", VolumeToString(SoundVolume)), SoundVolume);
	if (engine->LaunchInfo.IsDeusEx())
		SpeechVolume = IniPropertyConverter<uint8_t>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SpeechVolume", SpeechVolume);
	AmbientFactor = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "AmbientFactor", AmbientFactor);
}

void USurrealAudioDevice::SaveConfig()
{
	engine->packages->SetIniValue("System", Class, "UseFilter", IniPropertyConverter<bool>::ToString(UseFilter));
	engine->packages->SetIniValue("System", Class, "UseSurround", IniPropertyConverter<bool>::ToString(UseSurround));
	engine->packages->SetIniValue("System", Class, "UseStereo", IniPropertyConverter<bool>::ToString(UseStereo));
	engine->packages->SetIniValue("System", Class, "UseCDMusic", IniPropertyConverter<bool>::ToString(UseCDMusic));
	engine->packages->SetIniValue("System", Class, "UseDigitalMusic", IniPropertyConverter<bool>::ToString(UseDigitalMusic));
	engine->packages->SetIniValue("System", Class, "UseSpatial", IniPropertyConverter<bool>::ToString(UseSpatial));
	engine->packages->SetIniValue("System", Class, "UseReverb", IniPropertyConverter<bool>::ToString(UseReverb));
	engine->packages->SetIniValue("System", Class, "Use3dHardware", IniPropertyConverter<bool>::ToString(Use3dHardware));
	engine->packages->SetIniValue("System", Class, "LowSoundQuality", IniPropertyConverter<bool>::ToString(LowSoundQuality));
	engine->packages->SetIniValue("System", Class, "ReverseStereo", IniPropertyConverter<bool>::ToString(ReverseStereo));
	engine->packages->SetIniValue("System", Class, "Latency", IniPropertyConverter<int>::ToString(Latency));
	engine->packages->SetIniValue("System", Class, "OutputRate", IniPropertyConverter<AudioFrequency>::ToString(OutputRate));
	engine->packages->SetIniValue("System", Class, "Channels", IniPropertyConverter<int>::ToString(Channels));
	engine->packages->SetIniValue("System", Class, "MusicVolume", VolumeToString(MusicVolume));
	engine->packages->SetIniValue("System", Class, "SoundVolume", VolumeToString(SoundVolume));
	if (engine->LaunchInfo.IsDeusEx())
		engine->packages->SetIniValue("System", Class, "SpeechVolume", IniPropertyConverter<uint8_t>::ToString(SpeechVolume));
	engine->packages->SetIniValue("System", Class, "AmbientFactor", IniPropertyConverter<float>::ToString(AmbientFactor));
}

void USurrealAudioDevice::InitDevice()
{
	// TODO: Add configurable option for audio device
	// TODO: Add configurable option for audio output frequency
	// TODO: Add option for number of sound channels
	// TODO: Add option for music buffer count
	// TODO: Add option for music buffer size
	// The games' ini says OutputRate=22050Hz, the rate their sound mixer ran at in 2002: played back at that rate everything above 11 kHz
	// is lost (muffled sounds, dull music). OpenAL resamples anything to the real output.
	// HP2's ALAudio driver has up to 32 channels of its own (HP2::AudioChannels)
	m_Device = AudioDevice::Create(engine->LaunchInfo.IsKnowWonder() ? std::max(OutputRate.frequency, 48000) : OutputRate.frequency, engine->LaunchInfo.IsHarryPotter2() ? std::max(Channels, HP2::AudioChannels()) : Channels, 16, 256);
	// HP2's ALAudio: OpenAL's inverse distance clamped model. HP1's Galaxy driver's linear falloff is set by UpdateSounds.
	if (engine->LaunchInfo.IsHarryPotter2())
		HP2::InitAudio(m_Device.get());
	LogMessage("Audio device initialized");
}

void USurrealAudioDevice::ShutdownDevice()
{
	m_Device.reset();
	LogMessage("Audio device has been shut down");
}

void USurrealAudioDevice::SetViewport(UViewport* InViewport)
{
	if (m_Viewport != InViewport)
	{
		StopSounds();

		m_Viewport = InViewport;

		if (m_Viewport)
		{
			if (m_Viewport->Actor()->Song() && m_Viewport->Actor()->Transition() == MTRAN_None)
				m_Viewport->Actor()->Transition() = MTRAN_Instant;

			PlayingSounds.resize(std::min(engine->LaunchInfo.IsHarryPotter2() ? HP2::AudioChannels() : Channels, m_Device->GetTotalChannels()));
		}
	}
}

void USurrealAudioDevice::Update()
{
	// HP2's audio driver starts and stops the ambient sounds itself, and pauses the sounds with the game
	if (engine->LaunchInfo.IsHarryPotter2())
	{
		HP2::UpdateAudio(this);
	}
	else
	{
		StartAmbience();
		UpdateAmbience();
	}
	UpdateSounds();

	// HP1's audio driver fades between songs, keeps a song playing when it is set again and scales the music by
	// PlayerPawn.PercentMusicVolume. It also plays its mp2 music although HP1 ships UseDigitalMusic=False. HP2 starts its music
	// from script (Actor.PlayMusic, src/hp2/HP2Natives.cpp): its PlayerPawn has no song properties for the stock code.
	if (engine->LaunchInfo.IsKnowWonder())
		m_Device->SetMusicVolume(KW::UpdateMusic(m_Device.get(), m_Viewport ? m_Viewport->Actor() : nullptr, MusicVolume, Latency));
	else
		UpdateMusic();
	m_Device->Update();
}

void USurrealAudioDevice::SetInstantMusicVolume(uint8_t newMusicVolume)
{
	MusicVolume = newMusicVolume;
	Update();
}

void USurrealAudioDevice::SetInstantSoundVolume(uint8_t newSoundVolume)
{
	SoundVolume = newSoundVolume;
	Update();
}

void USurrealAudioDevice::SetInstantSpeechVolume(uint8_t newSpeechVolume)
{
	SpeechVolume = newSpeechVolume;
	Update();
}

void USurrealAudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
	GetDevice()->PlayMusic(std::move(source));
}

void USurrealAudioDevice::StopMusic()
{
	GetDevice()->PlayMusic({});
}

void USurrealAudioDevice::StartAmbience()
{
	if (!m_Viewport || !m_Viewport->Actor())
		return;

	bool realtime = m_Viewport->IsRealtime() && m_Viewport->Actor()->Level()->Pauser() == "";
	if (realtime)
	{
		UActor* viewActor = m_Viewport->Actor()->ViewTarget() ? m_Viewport->Actor()->ViewTarget() : m_Viewport->Actor();
		for (UActor* Actor : m_Viewport->Actor()->XLevel()->Actors)
		{
			if (Actor && Actor->AmbientSound() && dist_squared(viewActor->Location(), Actor->Location()) <= square(Actor->WorldSoundRadius()))
			{
				int id = CreateSoundId(Actor, SLOT_Ambient, false);
				bool foundSound = false;
				for (size_t j = 0; j < PlayingSounds.size(); j++)
				{
					if (PlayingSounds[j].Id == id)
					{
						foundSound = true;
						break;
					}
				}
				if (!foundSound)
					PlaySound(Actor, id, Actor->AmbientSound(), Actor->Location(), AmbientFactor * Actor->SoundVolume() / 255.0f, Actor->WorldSoundRadius(), Actor->SoundPitch() / 64.0f);
			}
		}
	}
}

void USurrealAudioDevice::UpdateAmbience()
{
	if (!m_Viewport || !m_Viewport->Actor())
		return;

	UActor* ViewActor = m_Viewport->Actor()->ViewTarget() ? m_Viewport->Actor()->ViewTarget() : m_Viewport->Actor();
	bool Realtime = m_Viewport->IsRealtime() && m_Viewport->Actor()->Level()->Pauser() == "";
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];
		if (GetSoundSlot(Playing.Id) == SLOT_Ambient)
		{
			if (Playing.Actor->bDeleteMe() || dist_squared(ViewActor->Location(), Playing.Actor->Location()) > square(Playing.Actor->WorldSoundRadius()) || Playing.Actor->AmbientSound() != Playing.Sound || !Realtime)
			{
				// Ambient sound went out of range
				StopSound(i);
			}
			else
			{
				// Update basic sound properties
				Playing.Volume = 2.0f * (AmbientFactor * Playing.Actor->SoundVolume() / 255.0f);
				Playing.Radius = Playing.Actor->WorldSoundRadius();
				Playing.Pitch = Playing.Actor->SoundPitch() / 64.0f;
			}
		}
	}
}

void USurrealAudioDevice::UpdateLipSync(PlayingSound& Playing)
{
	if (engine->LaunchInfo.IsHarryPotter2())
	{
		HP2::UpdateLipSync(Playing);
		return;
	}
	if (!engine->LaunchInfo.IsDeusEx())
		return;
	if (!Playing.Actor || !engine->LevelInfo)
		return;
	if (GetSoundSlot(Playing.Id) != SLOT_Talk)
		return;

	UPawn* pawn = UObject::TryCast<UPawn>(Playing.Actor);
	if (!pawn)
		return;

	float elapsedTime = engine->LevelInfo->TimeSeconds() - Playing.StartTime;
	uint8_t letter = Playing.Sound->GetLipsyncLetterAt(std::round(elapsedTime * 10.0f) / 10.0f); // Update 10 times per second, DX has a tween time of 0.1 seconds

	if (!letter)
		return;

	pawn->bIsSpeaking() = true;
	pawn->nextPhoneme() = std::string(1, letter);
}

// Called wherever a slot is torn down, so the mouth does not freeze mid-vowel.
void USurrealAudioDevice::ClearLipSync(PlayingSound& Playing)
{
	if (engine->LaunchInfo.IsHarryPotter2())
	{
		HP2::StopLipSync(Playing);
		return;
	}
	if (!engine->LaunchInfo.IsDeusEx())
		return;
	if (!Playing.Actor)
		return;
	if (GetSoundSlot(Playing.Id) != SLOT_Talk)
		return;

	if (UPawn* pawn = UObject::TryCast<UPawn>(Playing.Actor))
	{
		pawn->bIsSpeaking() = false;
		pawn->nextPhoneme() = "X";   // X == MouthClosed
	}
}

void USurrealAudioDevice::UpdateSounds()
{
	if (!m_Viewport || !m_Viewport->Actor())
		return;

	// HP1's Galaxy driver: volume = clamp(1 - distance / radius) * Volume, at most the sound volume
	if (engine->LaunchInfo.IsKnowWonder() && !engine->LaunchInfo.IsHarryPotter2())
		m_Device->SetLinearFalloff(true, SoundVolume / 255.0f * 0.5f);

	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];

		if (Playing.Id != 0)
		{
			bool hp2 = engine->LaunchInfo.IsHarryPotter2();
			if (hp2 && HP2::SoundPaused(Playing))
				continue;

			// Update positioning from actor, if available; HP2 plays a 2D sound at the listener (ALAudio: source relative
			// at 0,0,0), the audio device's listener being the camera actor
			if (Playing.No3D && engine->CameraActor)
				Playing.Location = engine->CameraActor->Location();
			else if (Playing.Actor)
				Playing.Location = Playing.Actor->Location();

			// Global volume for the sound type; HP2 mutes the sound effects while it fast-forwards a skipped cutscene (MUTESOUNDS)
			float globalVolume = SoundVolume / 255.0f * 0.5f;
			if (engine->LaunchInfo.IsDeusEx() && GetSoundSlot(Playing.Id) == SLOT_Talk)
				globalVolume = SpeechVolume / 255.0f * 0.5f;
			if (hp2 && HP2::SoundsMuted())
				globalVolume = 0.0f;

			// HP2's StopSound fade-out (UALAudioSubsystem::Update): linear over FadeOutTime, a step of at most 1 s, then stopped
			float volume = Playing.Volume;
			if (Playing.FadeOutTime > 0.0f)
			{
				double now = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
				float step = (float)(now - Playing.FadeLastTime);
				Playing.FadeLastTime = now;
				if (step < 1.0f)
					Playing.FadeElapsed += step;
				if (Playing.FadeElapsed >= Playing.FadeOutTime)
				{
					StopSound(i);
					continue;
				}
				volume *= 1.0f - Playing.FadeElapsed / Playing.FadeOutTime;
			}

			if (hp2)
				volume = HP2::UpdateSound(this, (int)i, Playing, volume);
			// Update the priority
			if (hp2)
				Playing.Priority = HP2::SoundPriority(m_Viewport, Playing.Location, Playing.Volume, Playing.Radius, Playing.Flags);
			else
				Playing.Priority = SoundPriority(m_Viewport, Playing.Location, Playing.Volume * globalVolume, Playing.Radius);

			UpdateLipSync(Playing);

			// Update the device sound channels to match our configuration
			if (!Playing.IsActive)
			{
				m_Device->PlaySound((int)i, Playing.Sound, Playing.Location, volume * globalVolume, Playing.Radius, Playing.Pitch);
				if (engine->LaunchInfo.IsKnowWonder())
					m_Device->SetLooping((int)i, Playing.Loop);
				if (hp2)
					m_Device->SetSoundVelocity((int)i, vec3(0.0f));
				Playing.IsActive = true;
			}
			else
			{
				m_Device->UpdateSound((int)i, Playing.Location, volume * globalVolume, Playing.Radius, Playing.Pitch);
				if (!m_Device->IsPlaying((int)i))
				{
					ClearLipSync(Playing);
					Playing = {};
				}
			}
		}
	}
}

void USurrealAudioDevice::UpdateMusic()
{
	m_Device->SetMusicVolume(MusicVolume / 255.0f);

	if (m_Viewport && m_Viewport->Actor() && m_Viewport->Actor()->Transition() != MTRAN_None)
	{
		// To do: this needs to fade out the old song before switching

		if (CurrentSong)
		{
			m_Device->PlayMusic({});
			CurrentSong = nullptr;
		}

		CurrentSong = m_Viewport->Actor()->Song();
		CurrentSection = m_Viewport->Actor()->SongSection();

		if (CurrentSong && UseDigitalMusic)
		{
			int subsong = CurrentSection != 255 ? CurrentSection : 0;

			std::unique_ptr<AudioSource> source;
			if (CurrentSong->Format == "mp3" || CurrentSong->Format == "mp2")
				source = AudioSource::CreateMp3(CurrentSong->Data);
			else if (CurrentSong->Format == "ogg" || CurrentSong->Format == "event") // Some ogg files in Unreal 227 have event as format for some reason
				source = AudioSource::CreateOgg(CurrentSong->Data, true);
			else if (CurrentSong->Format == "wav")
				source = AudioSource::CreateWav(CurrentSong->Data);
			else
				source = AudioSource::CreateMod(CurrentSong->Data, true, subsong);

			if (source)
				m_Device->PlayMusic(std::move(source));
		}

		m_Viewport->Actor()->Transition() = MTRAN_None;
	}
}

int USurrealAudioDevice::CreateSoundId(UActor* actor, ESoundSlot slot, bool noOverride)
{
	if (slot == SLOT_None)
		return ((--FreeSlot) << 4) | (int)noOverride;
	else
		return (int)((((uint32_t)(ptrdiff_t)actor) << 4) | ((int)slot << 1) | (int)noOverride);
}

bool USurrealAudioDevice::IsSoundInterruptable(int id)
{
	return (id & 1) == 0;
}

ESoundSlot USurrealAudioDevice::GetSoundSlot(int id)
{
	return (ESoundSlot)((id >> 1) & 7);
}

bool USurrealAudioDevice::IsSameSoundId(int id1, int id2)
{
	return (id1 & ~1) == (id2 & ~1);
}

bool USurrealAudioDevice::PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, bool no3D, bool loop)
{
	if (engine->LaunchInfo.IsHarryPotter1() && HP1::ModsMuteCutsceneSpeech() &&
		(GetSoundSlot(Id) == SLOT_Interact || GetSoundSlot(Id) == SLOT_Talk))
		return false;

	// HP1's Galaxy driver asserts a radius (Actor.PlaySound already made a zero radius 1600, KW::PlaySound)
	if (Radius <= 0.0 && !engine->LaunchInfo.IsKnowWonder()) // Seems we have zero radius values. Lovely.
		Radius = 1500.0f;

	if (engine->LaunchInfo.IsHarryPotter2())
	{
		// ALAudio: the volume from 0.01 to full, the pitch from 0.1 to 2
		Volume = std::clamp(Volume, 0.01f, 1.0f);
		Pitch = std::clamp(Pitch, 0.1f, 2.0f);
	}
	else if (engine->LaunchInfo.IsKnowWonder())
	{
		// Galaxy keeps the volume as given; the linear falloff caps volume times falloff at full volume and a
		// negative volume plays silent
		Volume = std::max(Volume, 0.0f);
	}
	else if (GetSoundSlot(Id) == SLOT_Talk && engine->LaunchInfo.IsDeusEx())
	{
		// Should this still be directional?
		Volume *= 2.0f;
	}
	else
	{
		// Attempt to normalize volume around 1.0 as the values used by the original games are just really broken in general.
		if (Volume >= 8.0f)
			Volume = 0.8f; // Special check for announcer garbage
		else
			Volume = (Volume - 1.0f) * 0.25f + 1.0f;
	}

	if (!m_Viewport || !Sound)
		return false;

	// A sound that asked not to override itself and is already playing (HP2's Lumos_glow_loop is started every frame by every sparkle,
	// the original loops it once): not started again, or the copies fill every channel and the rest is dropped.
	if (engine->LaunchInfo.IsHarryPotter2() && (Id & 1))
	{
		for (const PlayingSound& Playing : PlayingSounds)
		{
			if (Playing.Id != 0 && Playing.Sound == Sound)
				return 0;
		}
	}

	// Allocate a new slot if requested
	if (GetSoundSlot(Id) == SLOT_None)
		Id = CreateSoundId(Actor, SLOT_None, false);

	int hp2Flags = 0;
	float Priority;
	size_t Index = PlayingSounds.size();
	if (engine->LaunchInfo.IsHarryPotter2())
	{
		hp2Flags = HP2::SoundFlags(Sound, Id, loop, no3D);
		Priority = HP2::SoundPriority(m_Viewport, Location, Volume, Radius, hp2Flags);
		int channel = HP2::ChooseSoundChannel(this, Id, Priority);
		if (channel < 0)
			return false;
		Index = (size_t)channel;
	}
	else
	{
		Priority = SoundPriority(m_Viewport, Location, Volume, Radius);
	}

	// If already playing, stop it
	float BestPriority = Priority;
	for (size_t i = 0; i < PlayingSounds.size() && !engine->LaunchInfo.IsHarryPotter2(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];
		if (IsSameSoundId(Playing.Id, Id))
		{
			// Skip if not interruptable.
			if (!IsSoundInterruptable(Id))
				return 0;

			// Stop the sound.
			Index = i;
			break;
		}
		else if (Playing.Priority <= BestPriority)
		{
			Index = i;
			BestPriority = Playing.Priority;
		}
	}

	// If no sound, or its priority is overruled, stop it
	if (Index == PlayingSounds.size())
	{
		if (engine->LaunchInfo.IsHarryPotter2())
			LogMessage("PlaySound dropped, every channel is playing something louder: " + Sound->Name.ToString() + " priority " + std::to_string(Priority) + " channels " + std::to_string(PlayingSounds.size()));
		return 0;
	}

	Sound->CheckSoundSamples();

	// Put the sound on the play-list
	StopSound(Index);
	PlayingSounds[Index] = PlayingSound(Actor, Id, Sound, Location, Volume, Radius, Pitch, Priority);
	PlayingSounds[Index].StartTime = engine->LevelInfo->TimeSeconds();
	PlayingSounds[Index].No3D = no3D;
	PlayingSounds[Index].Loop = loop;
	PlayingSounds[Index].Flags = hp2Flags;

	return true;
}

void USurrealAudioDevice::ActorDestroyed(UActor* Actor)
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		if (PlayingSounds[i].Actor == Actor)
		{
			if ((PlayingSounds[i].Id & 14) == SLOT_Ambient * 2)
			{
				// Stop ambient sound when actor dies
				StopSound(i);
			}
			else
			{
				// Unbind regular sounds from actors
				PlayingSounds[i].Actor = nullptr;
			}
		}
	}
}

void USurrealAudioDevice::StopSound(UActor* Actor, int Id)
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		if (PlayingSounds[i].Actor == Actor && PlayingSounds[i].Id == Id)
		{
			StopSound(i);
			break;
		}
	}
}

// Galaxy.dll UGalaxyAudioSubsystem::ModifySound / StopSound (HP1's audio driver). Param 0 = volume,
// 1 = radius, 2 = pitch, stored as given.
bool USurrealAudioDevice::ModifySoundHP1(int Id, USound* Sound, int Param, float Value)
{
	for (PlayingSound& Playing : PlayingSounds)
	{
		if (Playing.Id != 0 && ((Playing.Id ^ Id) & ~1) == 0 && (!Sound || Playing.Sound == Sound))
		{
			if (Param == 0)
				Playing.Volume = std::max(Value, 0.0f); // Galaxy plays a negative volume silent (BroomHarry's fade-out goes below 0)
			else if (Param == 1)
				Playing.Radius = Value;
			else if (Param == 2)
				Playing.Pitch = Value;
			return true;
		}
	}
	return false;
}

bool USurrealAudioDevice::StopSoundHP1(int Id, USound* Sound)
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		if (PlayingSounds[i].Id != 0 && ((PlayingSounds[i].Id ^ Id) & ~1) == 0 && (!Sound || PlayingSounds[i].Sound == Sound))
		{
			StopSound(i);
			return true;
		}
	}
	return false;
}

bool USurrealAudioDevice::StopSoundHP2(UActor* Actor, int Id, USound* Sound, float FadeOutTime)
{
	bool stopped = false;
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];
		bool idMatches = ((Playing.Id ^ Id) & ~1) == 0 || ((Id & 14) == 0 && (Playing.Id & 14) == 0);
		if (Playing.Id == 0 || Playing.Actor != Actor || !idMatches || (Sound && Playing.Sound != Sound))
			continue;
		stopped = true;
		if (FadeOutTime <= 0.0f)
		{
			StopSound(i);
		}
		else
		{
			Playing.FadeOutTime = FadeOutTime;
			Playing.FadeElapsed = 0.0f;
			Playing.FadeLastTime = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}
	}
	return stopped;
}

void USurrealAudioDevice::StopSound(size_t index)
{
	PlayingSound& Playing = PlayingSounds[index];

	if (Playing.IsActive)
	{
		m_Device->StopSound((int)index);
		Playing.IsActive = false;
	}

	ClearLipSync(Playing);
	PlayingSounds[index] = {};
}

void USurrealAudioDevice::StopSounds()
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
		StopSound(i);

	// HP1's audio driver keeps the music playing through a level change or a loaded save (its LoadMap only stops the
	// sounds); the next Transition decides what plays
	if (engine->LaunchInfo.IsKnowWonder())
		return;

	m_Device->PlayMusic(nullptr);
	m_Viewport = nullptr;
}

void USurrealAudioDevice::BreakpointTriggered()
{
	if (m_Device)
	{
		uint8_t oldSound = SoundVolume;
		uint8_t oldSpeech = SpeechVolume;
		SoundVolume = 0;
		UpdateSounds();
		m_Device->Update();
		SoundVolume = oldSound;
		SpeechVolume = oldSpeech;
	}
}

void USurrealAudioDevice::AddStats(Array<std::string>& lines)
{
	const int bufsize = 1024;
	char buffer[bufsize];
	int index = 0;
	for (const PlayingSound& sound : PlayingSounds)
	{
		if (sound.IsActive)
		{
			std::snprintf(buffer, bufsize - 1, "Channel %2i: Vol: %05.2f %s", index, sound.Volume, sound.Sound->Name.ToString().c_str());
		}
		else
		{
			if (index >= 10)
				std::snprintf(buffer, bufsize - 1, "Channel %i:  None", index);
			else
				std::snprintf(buffer, bufsize - 1, "Channel %i: None", index);
		}
		buffer[bufsize - 1] = 0;
		lines.push_back(buffer);
		index++;
	}
}

float USurrealAudioDevice::SoundPriority(UViewport* Viewport, vec3 Location, float Volume, float Radius)
{
	UActor* target = Viewport->Actor();
	if (target && Viewport->Actor()->ViewTarget())
		target = Viewport->Actor()->ViewTarget();
	return target ? std::max(Volume * (1.0f - length(Location - target->Location()) / Radius), 0.0f) : 0.0f;
}
