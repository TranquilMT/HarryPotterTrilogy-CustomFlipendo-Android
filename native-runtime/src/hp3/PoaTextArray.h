#pragma once
#include "Utils/UTF16.h"
#include <string>
#include <cstdint>
#include <vector>
#include <stdexcept>
namespace PoaTextArray
{
inline std::string Decode(const std::string& bytes)
{
    std::wstring wide;
    const bool le=bytes.size()>=2&&uint8_t(bytes[0])==255&&uint8_t(bytes[1])==254;
    const bool be=bytes.size()>=2&&uint8_t(bytes[0])==254&&uint8_t(bytes[1])==255;
    if(le||be)
    {
        if(bytes.size()%2) throw std::runtime_error("Truncated UTF-16 string-array file");
        for(size_t i=2;i<bytes.size();i+=2)
        {
            uint16_t a=uint8_t(bytes[i]),b=uint8_t(bytes[i+1]);
            wide.push_back(wchar_t(le?a|(b<<8):(a<<8)|b));
        }
        return from_utf16(wide);
    }
    if(bytes.size()>=3&&bytes.compare(0,3,"\xef\xbb\xbf")==0) return bytes.substr(3);
    // The original ANSI loader widens unsigned bytes, not UTF-8 sequences.
    for(unsigned char c:bytes) wide.push_back(wchar_t(c));
    return from_utf16(wide);
}
inline std::vector<std::string> Lines(std::string text)
{
    text.resize(text.find('\0')==std::string::npos?text.size():text.find('\0'));
    std::vector<std::string> lines;size_t start=0;
    while(true)
    {
        auto end=text.find("\r\n",start);
        if(end==std::string::npos) break;
        lines.push_back(text.substr(start,end-start));start=end+2;
    }
    // EU Core's helper only emits CRLF-terminated records, including blanks.
    return lines;
}
inline std::string Encode(const std::vector<std::string>& lines)
{
    std::string joined;for(const auto& line:lines) joined+=line+"\r\n";
    const auto wide=to_utf16(joined);
    bool ansi=true;for(wchar_t c:wide) if(uint32_t(c)>255){ansi=false;break;}
    std::string bytes;
    if(ansi) for(wchar_t c:wide) bytes.push_back(char(c));
    else
    {
        bytes="\xff\xfe";
        for(wchar_t c:wide){bytes.push_back(char(uint32_t(c)&255));bytes.push_back(char((uint32_t(c)>>8)&255));}
    }
    return bytes;
}
}
