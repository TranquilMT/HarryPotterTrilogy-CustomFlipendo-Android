#include "../../src/engine/SurrealEngine/Package/SerializedString.h"
#include "../../src/engine/SurrealEngine/Package/PackageReferences.h"
#include "../../src/engine/SurrealEngine/Package/CompactIndex.h"
#include <cassert>
#include <climits>
#include <map>
#include <vector>
#include <iostream>

template<class F> void rejects(F f) {
 bool failed = false;
 try { f(); } catch (const std::runtime_error&) { failed = true; }
 assert(failed);
}
std::string decode(int32_t count, const std::vector<uint8_t>& bytes, size_t expected) {
 size_t pos = 0;
 auto text = PackageStrings::Read(count, [&]() {
  if(pos == bytes.size()) throw std::runtime_error("Truncated input");
  return bytes[pos++];
 });
 assert(pos == expected);
 return text;
}
int32_t index(const std::vector<uint8_t>& bytes) {
 size_t pos=0;
 return PackageIndices::Read([&]() { if(pos==bytes.size()) throw std::runtime_error("Truncated index"); return bytes[pos++]; });
}
int main() {
 assert(index({0})==0);
 assert(index({0x3f})==63);
 assert(index({0x40,1})==64);
 assert(index({0xc0,1})==-64);
 assert(index({0x7f,0xff,0xff,0xff,0x0f})==INT_MAX);
 assert(index({0xc0,0x80,0x80,0x80,0x10})==INT_MIN);
 rejects([] { index({0x40}); });
 rejects([] { index({0x40,0x80,0x80,0x80,0x10}); });
 rejects([] { index({0x40,0x80,0x80,0x80,0x20}); });
 rejects([] { index({0x40,0x80,0x80,0x80,0x80,0}); });

 assert(decode(0, {}, 0).empty());
 assert(decode(1, {0, 99}, 1).empty());
 assert(decode(5, {'C','o','r','e',0,99}, 5) == "Core");
 assert(decode(2, {0xe9,0}, 2) == std::string(1,char(0xe9))); // UE1 ANSI bytes preserved
 assert(decode(-1, {0,0}, 2).empty());
 assert(decode(-4, {'H',0,0xe9,0,0x2d,0x4e,0,0,99}, 8) == u8"Hé中");
 assert(decode(-3, {0x3d,0xd8,0x00,0xde,0,0}, 6) == u8"😀");
 rejects([] { decode(INT_MIN, {}, 0); });
 rejects([] { decode(INT_MAX, {}, 0); });
 rejects([] { decode(3, {'a',0}, 0); });
 rejects([] { decode(2, {'a','b'}, 0); });
 rejects([] { decode(-2, {'a',0,1,0}, 0); });
 rejects([] { decode(-2, {'a',0,0}, 0); });
 rejects([] { decode(-2, {0x00,0xdc,0,0}, 0); });
 rejects([] { decode(-2, {0x00,0xd8,0,0}, 0); });
 rejects([] { decode(-3, {0x00,0xd8,'a',0,0,0}, 0); });
 struct Entry { int32_t outer; std::string name; };
 std::map<int32_t,Entry> entries = {{1,{0,"Group"}},{2,{1,"Nested"}},{3,{2,"Asset"}},
  {-1,{0,"Other"}},{-2,{-1,"Group"}},{-3,{-2,"Nested"}}};
 auto outer = [&](int ref) { auto i=entries.find(ref); if(i==entries.end()) throw std::runtime_error("Out of bounds"); return i->second.outer; };
 auto name = [&](int ref) { return entries.at(ref).name; };
 auto path = [&](int ref, bool omit=false) { return PackageReferences::OuterPath(ref,outer,name,omit); };
 assert(path(0).empty());
 assert(path(1) == "Group");
 assert(path(3) == "Group.Nested.Asset");
 assert(path(3) == "Group.Nested.Asset"); // repeated lookup must not modify entries
 assert(entries.at(3).outer==2 && entries.at(3).name=="Asset");
 assert(path(-3,true) == "Group.Nested");
 assert(path(-1,true).empty());
 rejects([&] { path(4); });
 rejects([&] { path(INT_MIN); });
 entries[1].outer=3;
 rejects([&] { path(3); });
 entries[1].outer=1;
 rejects([&] { path(1); });
 entries[-1].outer=-3;
 rejects([&] { path(-3,true); });
 entries.clear();
 for(int i=1; i<=1025; ++i) entries[i]={i-1,"Deep"};
 rejects([&] { path(1025); });
 std::cout << "PASS package strings and outer paths\n";
}
