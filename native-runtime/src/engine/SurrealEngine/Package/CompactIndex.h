#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace PackageIndices
{
template<class Byte>
int32_t Read(Byte byte)
{
	uint8_t b = byte();
	const bool negative = (b & 0x80) != 0;
	bool more = (b & 0x40) != 0;
	uint32_t value = b & 0x3f;
	unsigned shift = 6;
	for (unsigned i = 0; more && i < 4; ++i)
	{
		b = byte();
		if (i == 3 && (b & 0xe0)) throw std::runtime_error("Invalid compact package index");
		value |= uint32_t(b & (i == 3 ? 31 : 127)) << shift;
		shift += 7;
		more = (b & 0x80) != 0;
	}
	if (more || value > uint32_t(std::numeric_limits<int32_t>::max()) + uint32_t(negative))
		throw std::runtime_error("Compact package index overflow");
	return int32_t(negative ? -int64_t(value) : int64_t(value));
}
}
