
#include "Precomp.h"
#include "Utils/UTF16.h"
#include "Utils/Exception.h"

#ifdef WIN32

std::string from_utf16(const std::wstring& str)
{
	if (str.empty()) return {};
	int needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
	if (needed == 0)
		Exception::Throw("WideCharToMultiByte failed");
	std::string result;
	result.resize(needed);
	needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size(), nullptr, nullptr);
	if (needed == 0)
		Exception::Throw("WideCharToMultiByte failed");
	return result;
}

std::wstring to_utf16(const std::string& str)
{
	if (str.empty()) return {};
	int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
	if (needed == 0)
		Exception::Throw("MultiByteToWideChar failed");
	std::wstring result;
	result.resize(needed);
	needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
	if (needed == 0)
		Exception::Throw("MultiByteToWideChar failed");
	return result;
	}

#else

// wchar_t is 32 bits here, but callers fill and read it as UTF-16 code units (as on Windows), so convert those
std::string from_utf16(const std::wstring& str)
{
	std::string result;
	for (size_t i = 0; i < str.size(); i++)
	{
		uint32_t c = (uint32_t)str[i] & 0xffff;
		if (c >= 0xd800 && c < 0xdc00 && i + 1 < str.size())
		{
			uint32_t lo = (uint32_t)str[i + 1] & 0xffff;
			if (lo >= 0xdc00 && lo < 0xe000)
			{
				c = 0x10000 + ((c - 0xd800) << 10) + (lo - 0xdc00);
				i++;
			}
		}
		if (c < 0x80)
		{
			result.push_back((char)c);
		}
		else if (c < 0x800)
		{
			result.push_back((char)(0xc0 | (c >> 6)));
			result.push_back((char)(0x80 | (c & 0x3f)));
		}
		else if (c < 0x10000)
		{
			result.push_back((char)(0xe0 | (c >> 12)));
			result.push_back((char)(0x80 | ((c >> 6) & 0x3f)));
			result.push_back((char)(0x80 | (c & 0x3f)));
		}
		else
		{
			result.push_back((char)(0xf0 | (c >> 18)));
			result.push_back((char)(0x80 | ((c >> 12) & 0x3f)));
			result.push_back((char)(0x80 | ((c >> 6) & 0x3f)));
			result.push_back((char)(0x80 | (c & 0x3f)));
		}
	}
	return result;
}

std::wstring to_utf16(const std::string& str)
{
	std::wstring result;
	size_t i = 0;
	while (i < str.size())
	{
		uint8_t b = (uint8_t)str[i];
		uint32_t c;
		int extra;
		if (b < 0x80) { c = b; extra = 0; }
		else if ((b & 0xe0) == 0xc0) { c = b & 0x1f; extra = 1; }
		else if ((b & 0xf0) == 0xe0) { c = b & 0x0f; extra = 2; }
		else if ((b & 0xf8) == 0xf0) { c = b & 0x07; extra = 3; }
		else { c = 0xfffd; extra = 0; }
		i++;
		for (int j = 0; j < extra; j++)
		{
			if (i >= str.size() || ((uint8_t)str[i] & 0xc0) != 0x80)
			{
				c = 0xfffd;
				break;
			}
			c = (c << 6) | ((uint8_t)str[i++] & 0x3f);
		}
		if (c >= 0x10000)
		{
			c -= 0x10000;
			result.push_back((wchar_t)(0xd800 + (c >> 10)));
			result.push_back((wchar_t)(0xdc00 + (c & 0x3ff)));
		}
		else
		{
			result.push_back((wchar_t)c);
		}
	}
	return result;
}

#endif
