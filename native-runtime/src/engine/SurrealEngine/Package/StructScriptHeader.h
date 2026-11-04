#pragma once
#include <cstdint>
#include <stdexcept>

namespace StructScripts
{
constexpr uint32_t MaximumBytes = 64u * 1024u * 1024u;
struct Header { uint32_t line, textPosition, memoryBytes; };

// The extra byte is verified for PoA v129 Function exports only. Do not infer
// Class/State/Struct layouts, or select a game solely from a package version.
template<class Stream>
Header Read(Stream& stream, bool poaFunction)
{
	if (poaFunction)
	{
		if (stream.GetVersion() != 129) throw std::runtime_error("Unverified PoA function metadata version");
		if (stream.ReadUInt8() != 0) throw std::runtime_error("Unverified PoA function extension byte");
	}
	Header header;
	header.line = stream.ReadUInt32();
	header.textPosition = stream.ReadUInt32();
	header.memoryBytes = stream.ReadUInt32();
	if (header.memoryBytes > MaximumBytes) throw std::runtime_error("Excessive struct bytecode memory size");
	return header;
}

template<class Stream>
void Write(Stream& stream, bool poaFunction, Header header)
{
	if (header.memoryBytes > MaximumBytes) throw std::runtime_error("Excessive struct bytecode memory size");
	if (poaFunction && stream.GetVersion() != 129) throw std::runtime_error("Unverified PoA function save version");
	if (poaFunction) stream.WriteUInt8(0);
	stream.WriteUInt32(header.line);
	stream.WriteUInt32(header.textPosition);
	stream.WriteUInt32(header.memoryBytes);
}
}
