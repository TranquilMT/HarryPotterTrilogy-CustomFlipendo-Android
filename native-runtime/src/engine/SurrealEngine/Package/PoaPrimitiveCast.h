#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace PoaPrimitiveCasts
{
// UE2 EX_PrimitiveCast (0x39) is followed by a cast code and one expression.
// Enable scalar casts and the additional casts observed in PoA packages
// that already have matching production evaluators.
inline bool Supported(uint8_t code)
{
	return (code >= 0x39 && code <= 0x45) ||
		(code >= 0x49 && code <= 0x4c) ||
		code == 0x50 ||
		(code >= 0x52 && code <= 0x59);
}

template<class Stream>
uint8_t ReadCode(Stream& stream)
{
	const uint8_t code = stream.ReadUInt8();
	if (!Supported(code)) throw std::runtime_error("Unsupported PoA primitive cast code " + std::to_string(code));
	return code;
}

// Preserve the prefix in serialized/decoded bytes. Normalize only when building
// the expression tree, keeping statement/jump offsets in the original dialect.
template<class Stream>
uint8_t ExpressionToken(Stream& stream, uint8_t token, bool poa)
{
	if (!poa) return token;
	if (token == 0x39) return ReadCode(stream);
	if (token >= 0x3a && token < 0x60)
		throw std::runtime_error("Unsupported production PoA bytecode token " + std::to_string(token));
	return token;
}
}
