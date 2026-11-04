#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace PackageValidation
{
struct Region { uint64_t begin, end; };

inline void Range(uint64_t offset, uint64_t bytes, uint64_t total, const std::string& label)
{
	if (offset > total || bytes > total - offset)
		throw std::runtime_error(label + ": range outside package");
}

inline void Count(int32_t count, uint64_t remaining, uint32_t minimumBytes, const std::string& label)
{
	if (!minimumBytes || count < 0 || count > 1000000 || uint64_t(count) > remaining / minimumBytes)
		throw std::runtime_error(label + ": invalid or excessive count");
}

inline void Table(int32_t count, int32_t offset, uint64_t total, uint64_t summaryEnd,
	uint32_t minimumBytes, const std::string& label)
{
	if (count < 0 || count > 1000000) throw std::runtime_error(label + ": invalid count");
	if (!count) return; // Empty legacy tables may use a zero offset.
	if (offset < 0 || uint64_t(offset) < summaryEnd || uint64_t(offset) >= total)
		throw std::runtime_error(label + ": invalid table offset");
	Count(count, total - uint64_t(offset), minimumBytes, label);
}

inline void Reference(int32_t ref, uint32_t imports, uint32_t exports, const std::string& label)
{
	if (int64_t(ref) < -int64_t(imports) || int64_t(ref) > exports)
		throw std::runtime_error(label + ": object reference outside tables (" + std::to_string(ref) + ")");
}

inline void Name(int32_t index, uint32_t count, const std::string& label)
{
	if (index < 0 || uint32_t(index) >= count)
		throw std::runtime_error(label + ": name index outside table");
}

inline Region Payload(int32_t size, int32_t offset, uint64_t total, const std::string& label)
{
	if (size < 0 || (size && offset < 0)) throw std::runtime_error(label + ": negative export range");
	if (!size) return {0, 0};
	Range(uint64_t(offset), uint64_t(size), total, label);
	return {uint64_t(offset), uint64_t(offset) + uint64_t(size)};
}

inline void Disjoint(Region first, Region second, const std::string& label)
{
	if (first.begin < first.end && second.begin < second.end && first.begin < second.end && second.begin < first.end)
		throw std::runtime_error(label + ": overlapping serialized regions");
}

// Each local outer link is visited once. Imported/exported identities remain distinct.
template<class Outer>
void Outers(uint32_t exports, uint32_t imports, Outer outer, const std::string& label)
{
	if (exports > 1000000 || imports > 1000000) throw std::runtime_error(label + ": excessive reference tables");
	const uint32_t count = exports + imports;
	std::vector<uint8_t> state(count, 0);
	std::vector<uint32_t> depth(count, 0);
	std::vector<uint32_t> chain;
	for (uint32_t start = 0; start < count; ++start)
	{
		if (state[start] == 2) continue;
		chain.clear();
		int32_t ref = start < exports ? int32_t(start + 1) : -int32_t(start - exports + 1);
		uint32_t tailDepth = 0;
		while (ref != 0)
		{
			Reference(ref, imports, exports, label);
			uint32_t index = ref > 0 ? uint32_t(ref - 1) : exports + uint32_t(-int64_t(ref) - 1);
			if (state[index] == 2) { tailDepth = depth[index]; break; }
			if (state[index] == 1 || chain.size() >= 1024)
				throw std::runtime_error(label + ": cyclic or excessive outer chain at reference " + std::to_string(ref));
			state[index] = 1;
			chain.push_back(index);
			ref = outer(ref);
		}
		for (auto it = chain.rbegin(); it != chain.rend(); ++it)
		{
			if (++tailDepth > 1024) throw std::runtime_error(label + ": excessive outer chain");
			depth[*it] = tailDepth;
			state[*it] = 2;
		}
	}
}
}
