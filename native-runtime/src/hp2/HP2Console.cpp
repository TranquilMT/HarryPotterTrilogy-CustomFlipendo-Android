#include "Precomp.h"
#include "HP2.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Utils/StrTools.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

// HP2's console commands that the scripts send through ConsoleCommand and that the engine's own Exec answers
// (UViewport / audio subsystem). NOT reversed: the effect of each is taken from the hgame scripts that send it.
// Audio commands, including cutscene mute/unmute and SETVOLUMES, are handled by HP2Music.cpp.
//   MLOD x                     FESoundVideoPage "object detail" (mesh LOD bias). No LOD change here.
// ISFULLSCREEN uses the engine handler so the Options page sees the actual window state.
namespace HP2
{
	static const char* const AudioSection = "ALAudio.ALAudioSubsystem";

	// HP2 keeps its volumes in [ALAudio.ALAudioSubsystem] as floats (MusicVolume=0.53, SoundVolume=0.9); the stock
	// audio device reads them as bytes (0..255) from another section, and a "0.9" reads as 0.
	void LoadAudioVolumes()
	{
		if (!engine->audiodev || !engine->packages)
			return;
		const std::string music = engine->packages->GetIniValue("System", AudioSection, "MusicVolume", "");
		const std::string sound = engine->packages->GetIniValue("System", AudioSection, "SoundVolume", "");
		if (!music.empty())
			engine->audiodev->SetMusicVolume(VolumeFromIni(music, engine->audiodev->GetMusicVolume()));
		if (!sound.empty())
			engine->audiodev->SetSoundVolume(VolumeFromIni(sound, engine->audiodev->GetSoundVolume()));

	}

	// Keep the engine's device configuration and HP2's original float-volume section
	// together. LoadAudioVolumes reads the latter even when an engine INI exists.
	void SaveAudioVolumes()
	{
		if (!engine || !engine->audiodev || !engine->packages)
			return;
		auto* audio = engine->audiodev;
		engine->packages->SetIniValue("System", AudioSection, "MusicVolume", VolumeToIni(audio->GetMusicVolume()));
		engine->packages->SetIniValue("System", AudioSection, "SoundVolume", VolumeToIni(audio->GetSoundVolume()));
		audio->SaveConfig();
		engine->packages->SaveAllIniFiles();
	}

	bool ConsoleCommand(const std::string& commandline, std::string& result)
	{
		std::istringstream words(commandline);
		std::string name;
		words >> name;
		if (name.empty())
			return false;

		// Audio commands fall through to AudioCommand; do not swallow mute/unmute.
		if (StrTools::equals_ignore_case(name, "mlod"))
		{
			result.clear();
			return true;
		}
		// FESoundVideoPage reads and writes the volumes as "get ini:Engine.Engine.AudioDevice SoundVolume" and "set ini:... SoundVolume 0.9",
		// fractions 0..1. The stock get/set treat the device's byte properties: the page read 255, showed 25500 and wrote back 1.0,
		// which the byte property took as 1 of 255: every sound started after that was silent.
		if (StrTools::equals_ignore_case(name, "get") || StrTools::equals_ignore_case(name, "set"))
		{
			std::string target, key, value;
			words >> target >> key >> value;
			std::string lowerTarget = target;
			std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), [](unsigned char c) { return (char)std::tolower(c); });
			const bool music = StrTools::equals_ignore_case(key, "MusicVolume");
			const bool sound = StrTools::equals_ignore_case(key, "SoundVolume");
			if (lowerTarget.find("audiodevice") == std::string::npos || (!music && !sound) || !engine->audiodev)
				return false;
			uint8_t volume = music ? engine->audiodev->GetMusicVolume() : engine->audiodev->GetSoundVolume();
			if (StrTools::equals_ignore_case(name, "get"))
			{
				result = std::to_string(volume / 255.0f);
				return true;
			}
			if (!value.empty())
			{
				uint8_t updated = VolumeFromIni(value, volume);
				if (updated != volume)
				{
					if (music) engine->audiodev->SetMusicVolume(updated); else engine->audiodev->SetSoundVolume(updated);
					SaveAudioVolumes();
				}
			}
			result.clear();
			return true;
		}

		return false;
	}
}

namespace HP2
{
	// HP2 walks with A/D as strafe and turns with the arrow keys (RotLeft/RotRight). The touch overlay and a gamepad
	// press the arrows for left and right, so on the phone they strafe and the camera is turned by dragging the screen.
	void TweakKeybindings()
	{
#ifdef __ANDROID__
		auto replace = [](std::string& text, const std::string& from, const std::string& to)
		{
			size_t pos = text.find(from);
			if (pos != std::string::npos)
				text.replace(pos, from.size(), to);
		};
		for (const char* key : { "Left", "Right" })
		{
			auto it = engine->keybindings.find(key);
			if (it == engine->keybindings.end())
				continue;
			replace(it->second, "RotLeft", "StrafeLeft");
			replace(it->second, "RotRight", "StrafeRight");
		}
#endif
	}
}
