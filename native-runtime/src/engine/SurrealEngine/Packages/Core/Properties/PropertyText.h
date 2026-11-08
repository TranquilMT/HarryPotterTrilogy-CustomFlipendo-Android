#pragma once
#include <vector>
#include <utility>
#include <string>
#include <stdexcept>

namespace PropertyText
{
inline std::string Trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    return first == std::string::npos ? std::string() : s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}
inline std::vector<std::string> List(const std::string& text)
{
    const std::string s = Trim(text);
    if (s.empty() || s == "None" || s == "null" || s == "null struct") return {};
    if (s.size() < 2 || s.front() != '(' || s.back() != ')') throw std::runtime_error("Invalid struct property text");
    std::vector<std::string> result;
    size_t start = 1;
    int depth = 0;
    char quote = 0;
    bool escaped = false;
    auto add = [&](size_t end)
    {
        const std::string item = Trim(s.substr(start, end - start));
        if (item.empty()) throw std::runtime_error("Empty struct property assignment");
        result.push_back(item);
    };
    for (size_t i = 1; i + 1 < s.size(); ++i)
    {
        const char c = s[i];
        if (quote)
        {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == quote) quote = 0;
        }
        else if (c == '"' || c == '\'') quote = c;
        else if (c == '(') { if (++depth > 64) throw std::runtime_error("Struct property nesting too deep"); }
        else if (c == ')') { if (--depth < 0) throw std::runtime_error("Unmatched struct property parenthesis"); }
        else if (c == ',' && depth == 0) { add(i); start = i + 1; }
    }
    if (quote || depth != 0) throw std::runtime_error("Unterminated struct property value");
    if (!Trim(s.substr(start, s.size() - 1 - start)).empty()) add(s.size() - 1);
    return result;
}
inline std::vector<std::pair<std::string, std::string>> Parse(const std::string& text)
{
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& item : List(text))
    {
        const size_t equal = item.find('=');
        if (equal == std::string::npos) throw std::runtime_error("Missing struct property assignment");
        const std::string name = Trim(item.substr(0, equal));
        if (name.empty()) throw std::runtime_error("Empty struct property name");
        result.emplace_back(name, Trim(item.substr(equal + 1)));
    }
    return result;
}
inline size_t FieldOffset(size_t base, size_t pitch, size_t index, size_t blockSize, size_t fieldSize)
{
    if (fieldSize > blockSize || base > blockSize - fieldSize ||
        (index && (!pitch || index > (blockSize - fieldSize - base) / pitch)))
        throw std::runtime_error("Struct text field is outside its data block");
    return base + index * pitch;
}
inline std::string Quote(const std::string& value)
{
    std::string result = "\"";
    for (char c : value) { if (c == '\\' || c == '"') result += '\\'; result += c; }
    return result + '"';
}
inline std::string Unquote(const std::string& value)
{
    if (value.size() < 2 || value.front() != '"' || value.back() != '"') return value;
    std::string result;
    for (size_t i = 1; i + 1 < value.size(); ++i)
    {
        if (value[i] == '\\' && i + 2 < value.size() && (value[i+1] == '\\' || value[i+1] == '"')) ++i;
        result += value[i];
    }
    return result;
}
}
