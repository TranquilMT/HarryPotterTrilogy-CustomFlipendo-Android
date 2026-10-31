// HP3 metadata deserialization. GPL-3.0, as part of the Flipendo fork.
// Not connected to the UE1 object loader or VM until UE2 object layouts exist.
#pragma once
#include <cstdint>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace hp3 {
class Reader {
public:
 Reader(const std::vector<uint8_t>& bytes) : bytes_(bytes) {}
 size_t position() const { return position_; }
 size_t remaining() const { return bytes_.size() - position_; }
 void skip(size_t n) { if (n > remaining()) throw std::runtime_error("Read outside object"); position_ += n; }
 uint8_t byte() { if (!remaining()) throw std::runtime_error("Truncated object"); return bytes_[position_++]; }
 uint16_t word() { uint16_t n=byte(); return n | (uint16_t(byte())<<8); }
 uint32_t dword() { uint32_t n=word(); return n | (uint32_t(word())<<16); }
 int32_t index() {
  uint8_t b=byte(); bool negative=(b&128)!=0; bool more=(b&64)!=0;
  uint32_t value=b&63; unsigned shift=6;
  for (int i=0; more && i<4; ++i) {
   b=byte(); if (i==3 && (b&0xe0)) throw std::runtime_error("Invalid compact index");
   value |= uint32_t(b & (i==3?31:127))<<shift; shift+=7; more=(b&128)!=0;
  }
  if (more || value>uint32_t(std::numeric_limits<int32_t>::max())+uint32_t(negative)) throw std::runtime_error("Compact index overflow");
  return int32_t(negative ? -int64_t(value) : int64_t(value));
 }
 std::string name(const std::vector<std::string>& names) {
  int32_t n=index(); if(n<0 || size_t(n)>=names.size()) throw std::runtime_error("Name outside table"); return names[n];
 }
private:
 const std::vector<uint8_t>& bytes_; size_t position_=0;
};
struct Tag {
 std::string name, structName; uint8_t type=0; uint32_t arrayIndex=0;
 size_t payloadOffset=0, payloadBytes=0; bool boolean=false;
};
inline std::vector<Tag> readTags(Reader& r,const std::vector<std::string>& names) {
 std::vector<Tag> tags;
 for(size_t count=0;count<100000;++count) {
  Tag t; t.name=r.name(names); if(t.name=="None") return tags;
  uint8_t info=r.byte(); t.type=info&15;
  if(!t.type) throw std::runtime_error("Invalid property type");
  if(t.type==10) t.structName=r.name(names);
  const uint32_t fixed[]={1,2,4,12,16}; unsigned code=(info>>4)&7;
  uint32_t size=code<5?fixed[code]:(code==5?r.byte():(code==6?r.word():r.dword()));
  if((info&128) && t.type!=3) {
   uint8_t b=r.byte();
   if(b<128) t.arrayIndex=b;
   else if(b&64) { uint32_t n=uint32_t(b)<<24; n|=uint32_t(r.byte())<<16; n|=uint32_t(r.byte())<<8; n|=r.byte(); t.arrayIndex=n&0x3fffffff; }
   else t.arrayIndex=((uint32_t(b)<<8)|r.byte())&0x3fff;
  }
  t.payloadOffset=r.position(); t.boolean=(info&128)!=0;
  t.payloadBytes=t.type==3?0:size; r.skip(t.payloadBytes); tags.push_back(t);
 }
 throw std::runtime_error("Too many tagged properties");
}
struct FunctionHeader {
 int32_t base=0,next=0,scriptText=0,children=0;
 std::string friendlyName; uint8_t extensionByte=0;
 uint32_t line=0,textPosition=0,bytecodeMemoryBytes=0; size_t bytecodeOffset=0;
};
inline FunctionHeader readFunctionHeader(Reader& r,const std::vector<std::string>& names) {
 readTags(r,names); FunctionHeader h;
 h.base=r.index();h.next=r.index();h.scriptText=r.index();h.children=r.index();h.friendlyName=r.name(names);
 h.extensionByte=r.byte(); if(h.extensionByte) throw std::runtime_error("Unverified HP3 struct extension byte");
 h.line=r.dword();h.textPosition=r.dword();h.bytecodeMemoryBytes=r.dword();
 if(h.bytecodeMemoryBytes>64*1024*1024) throw std::runtime_error("Excessive bytecode size");
 h.bytecodeOffset=r.position();return h;
}
struct NativeStub { uint16_t index=0; uint8_t precedence=0; uint32_t flags=0; uint16_t replicationOffset=0; std::vector<int32_t> parameters; };
inline NativeStub readNativeStub(Reader& r,const FunctionHeader& h) {
 uint64_t memory=0;NativeStub n;
 while(memory<h.bytecodeMemoryBytes) {
  uint8_t op=r.byte();++memory;
  if(op==0x29) { n.parameters.push_back(r.index());memory+=4; }
  else if(op!=0x0b) throw std::runtime_error("Script execution not implemented");
 }
 if(memory!=h.bytecodeMemoryBytes) throw std::runtime_error("Bytecode size mismatch");
 n.index=r.word();n.precedence=r.byte();n.flags=r.dword();
 if(n.flags&0x40) n.replicationOffset=r.word();
 if(r.remaining()) throw std::runtime_error("Unconsumed function bytes");
 if(!(n.flags&0x400)) throw std::runtime_error("Not a native function stub");
 return n;
}
} // namespace hp3
