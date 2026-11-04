#pragma once
#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace PackageReferences
{
// References stay local to their owning package. Callbacks validate table bounds.
template<class Outer, class Name>
std::string OuterPath(int32_t ref, Outer outer, Name name, bool omitRoot = false)
{
	std::set<int32_t> visited;
	std::vector<std::string> parts;
	while (ref != 0)
	{
		if (visited.size() >= 1024 || !visited.insert(ref).second)
			throw std::runtime_error("Cyclic or excessive package outer chain at reference " + std::to_string(ref));
		const int32_t parent = outer(ref);
		if (!omitRoot || parent != 0)
			parts.push_back(name(ref));
		ref = parent;
	}
	std::string path;
	for (auto it = parts.rbegin(); it != parts.rend(); ++it)
	{
		if (!path.empty()) path += '.';
		path += *it;
	}
	return path;
}
}
