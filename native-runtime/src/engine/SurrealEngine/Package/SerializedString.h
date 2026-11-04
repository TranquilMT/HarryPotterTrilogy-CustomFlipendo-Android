#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace PackageStrings
{
inline void AppendUTF8(std::string& text, uint32_t code)
{
	if (code < 0x80) text += char(code);
	else if (code < 0x800)
	{
		text += char(0xc0 | (code >> 6)); text += char(0x80 | (code & 63));
	}
	else if (code < 0x10000)
	{
		text += char(0xe0 | (code >> 12)); text += char(0x80 | ((code >> 6) & 63)); text += char(0x80 | (code & 63));
	}
	else
	{
		text += char(0xf0 | (code >> 18)); text += char(0x80 | ((code >> 12) & 63));
		text += char(0x80 | ((code >> 6) & 63)); text += char(0x80 | (code & 63));
	}
}

// Unreal FString counts include a trailing NUL. Negative counts are UTF-16LE
// code units, independent of the host wchar_t width (Android/Linux use 4 bytes).
template<class Byte>
std::string Read(int32_t length, Byte byte)
{
	const uint64_t count = length < 0 ? uint64_t(-int64_t(length)) : uint64_t(length);
	if (count > 1024 * 1024) throw std::runtime_error("Package string exceeds limit");
	std::string text;
	if (!count) return text;
	if (length > 0)
	{
		for (uint64_t i = 0; i < count; ++i)
		{
			const uint8_t ch = byte();
			if (i + 1 == count)
			{
				if (ch) throw std::runtime_error("Unterminated package string");
			}
			else text += char(ch);
		}
		return text;
	}
	auto word = [&]() { uint16_t lo = byte(); return uint16_t(lo | (uint16_t(byte()) << 8)); };
	for (uint64_t i = 0; i < count; ++i)
	{
		uint32_t code = word();
		if (i + 1 == count)
		{
			if (code) throw std::runtime_error("Unterminated UTF-16 package string");
			break;
		}
		if (code >= 0xd800 && code <= 0xdbff)
		{
			if (i + 2 >= count) throw std::runtime_error("Truncated UTF-16 surrogate pair");
			uint32_t low = word(); ++i;
			if (low < 0xdc00 || low > 0xdfff) throw std::runtime_error("Invalid UTF-16 surrogate pair");
			code = 0x10000 + ((code - 0xd800) << 10) + low - 0xdc00;
		}
		else if (code >= 0xdc00 && code <= 0xdfff) throw std::runtime_error("Unexpected UTF-16 low surrogate");
		AppendUTF8(text, code);
	}
	return text;
}
}
