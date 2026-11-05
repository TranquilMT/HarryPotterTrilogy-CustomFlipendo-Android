#pragma once
#include <cstdint>
#include <stdexcept>

namespace StructScripts
{
constexpr uint32_t MaximumBytes = 64u * 1024u * 1024u;
struct Header { uint32_t line, textPosition, memoryBytes; };

// PoA v129 stores a compact CppText reference before the script header.
template<class Stream, class ReadCppText>
Header Read(Stream& stream, bool poaStruct, ReadCppText readCppText)
{
    if (poaStruct)
    {
        if (stream.GetVersion() != 129) throw std::runtime_error("Unverified PoA struct metadata version");
        readCppText();
    }
    Header header;
    header.line = stream.ReadUInt32();
    header.textPosition = stream.ReadUInt32();
    header.memoryBytes = stream.ReadUInt32();
    if (header.memoryBytes > MaximumBytes) throw std::runtime_error("Excessive struct bytecode memory size");
    return header;
}

template<class Stream, class WriteCppText>
void Write(Stream& stream, bool poaStruct, Header header, WriteCppText writeCppText)
{
    if (header.memoryBytes > MaximumBytes) throw std::runtime_error("Excessive struct bytecode memory size");
    if (poaStruct)
    {
        if (stream.GetVersion() != 129) throw std::runtime_error("Unverified PoA struct save version");
        writeCppText();
    }
    stream.WriteUInt32(header.line);
    stream.WriteUInt32(header.textPosition);
    stream.WriteUInt32(header.memoryBytes);
}
}
