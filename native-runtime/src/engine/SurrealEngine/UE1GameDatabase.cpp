
#include "Precomp.h"
#include "UE1GameDatabase.h"
#include "Utils/SHA1Sum.h"
#include "Utils/File.h"
#include <iostream>

std::pair<KnownUE1Games, std::string> FindUE1GameInPath(const std::string& ue1_game_root_folder_path, std::string* problem)
{
	// report why a folder isn't recognised instead of skipping it silently
	auto notFound = [&](const std::string& why) {
		if (problem)
			*problem = why;
		return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, std::string());
	};

	if (ue1_game_root_folder_path.empty())
		return notFound("no folder given");

	if (!fs::exists(ue1_game_root_folder_path) || !fs::is_directory(ue1_game_root_folder_path))
		return notFound(ue1_game_root_folder_path + ": folder does not exist");

	const auto UE1GameSystemPath = File::find_ignoring_case(fs::path(ue1_game_root_folder_path) / "System");
	const auto UE1GameSystem64Path = fs::path(ue1_game_root_folder_path) / "System64";

	for (auto& executable_name : knownUE1ExecutableNames)
	{
		auto executablePath = UE1GameSystemPath / executable_name;
		auto executablePath64 = UE1GameSystem64Path / executable_name;

		std::string sha1sum = SHA1Sum::of_file(executablePath);
		std::string sha1sum64 = SHA1Sum::of_file(executablePath64);

		// some releases rename the executable (HP1: "Harry Potter and the Philosophers Stone.exe")
		for (auto& alias : knownUE1ExecutableAliases)
		{
			if (sha1sum.empty() && alias.second == executable_name)
			{
				executablePath = UE1GameSystemPath / alias.first;
				sha1sum = SHA1Sum::of_file(executablePath);
			}
		}

		// If sha1sum is empty this means that the file doesn't exist
		if (sha1sum.empty() && sha1sum64.empty())
			continue;

		// Now check whether there is a match within the database or not
		const auto it = !sha1sum64.empty() ? SHA1Database.find(sha1sum64) : SHA1Database.find(sha1sum);
		
		if (it == SHA1Database.end())
			return notFound((!sha1sum64.empty() ? executablePath64 : executablePath).string() + " is not a recognised version (SHA-1 " +
				(!sha1sum64.empty() ? sha1sum64 : sha1sum) + "). Please report it with the game's language and release.");

		// Hack: Tactical-Ops has a version that contains the exact same UTv469d executable. Handle that here
		if (it->second == KnownUE1Games::UT99_469d && executable_name == "TacticalOps.exe")
			return std::make_pair(KnownUE1Games::TACTICAL_OPS_469, executable_name);
		return std::make_pair(it->second, executable_name);
	}

	// We got nothing here
	// a common mistake is pointing at the System folder itself instead of the game folder
	if (fs::path(ue1_game_root_folder_path).filename() == "System" || fs::exists(fs::path(ue1_game_root_folder_path) / "Core.u"))
		return notFound(ue1_game_root_folder_path + " looks like the game's System folder; use the folder above it (the one that contains System, Maps, Textures)");
	return notFound(ue1_game_root_folder_path + ": no known game executable in its System folder (for Harry Potter: System/HP.exe). "
		"Use the game's install folder, the one that contains System, Maps, Textures");
}