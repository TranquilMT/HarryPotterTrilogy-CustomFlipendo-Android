// HP3 PC bytecode deserialization, GPL-3.0. This does not execute scripts.
#pragma once
#include "ObjectReader.h"
#include <set>
#include <utility>
namespace hp3 {
struct Instruction { uint32_t offset=0,serializedOffset=0,end=0,depth=0;uint8_t opcode=0; };
struct Script {
 std::vector<uint8_t> memory;
 std::vector<Instruction> instructions;
 std::vector<std::pair<uint32_t,uint32_t>> jumps;
};
class BytecodeReader {
public:
 BytecodeReader(Reader& source,const std::vector<std::string>& names,uint32_t imports,uint32_t exports,uint32_t size)
 : source_(source),names_(names),imports_(imports),exports_(exports),limit_(size),start_(source.position()) {
  if(size>64*1024*1024) throw std::runtime_error("Bytecode memory limit exceeded");
 }
 Script read() {
  while(position()<limit_) expression(0);
  if(position()!=limit_)throw std::runtime_error("Incorrect bytecode memory size");
  std::set<uint32_t> statements={limit_};
  for(const auto& i:script_.instructions)if(!i.depth)statements.insert(i.offset);
  for(const auto& jump:script_.jumps)if(!statements.count(jump.second))throw std::runtime_error("Jump target is not a statement boundary");
  return std::move(script_);
 }
private:
 uint32_t position()const{return uint32_t(script_.memory.size());}
 void put(uint8_t b){if(position()>=limit_)throw std::runtime_error("Operand exceeds bytecode memory size");script_.memory.push_back(b);}
 uint8_t byte(){uint8_t b=source_.byte();put(b);return b;}
 uint16_t word(){uint16_t n=byte();return n|(uint16_t(byte())<<8);}
 uint32_t dword(){uint32_t n=word();return n|(uint32_t(word())<<16);}
 void raw(unsigned count){for(unsigned i=0;i<count;++i)byte();}
 int32_t reference(bool name=false){
  int32_t n=source_.index();
  if(name){if(n<0||size_t(n)>=names_.size())throw std::runtime_error("Name reference outside table");}
  else if(int64_t(n)>exports_||int64_t(n)<-int64_t(imports_))throw std::runtime_error("Object reference outside table");
  for(unsigned i=0;i<4;++i)put(uint8_t(uint32_t(n)>>(i*8)));
  return n;
 }
 void parameters(unsigned depth){while(expression(depth+1)!=0x16){}}
 uint8_t expression(unsigned depth){
  if(depth>=128||script_.instructions.size()>=1000000)throw std::runtime_error("Expression limit exceeded");
  Instruction instruction;instruction.offset=position();instruction.serializedOffset=uint32_t(source_.position()-start_);instruction.depth=depth;instruction.opcode=byte();
  size_t index=script_.instructions.size();script_.instructions.push_back(instruction);uint8_t op=instruction.opcode;
  auto sub=[&]{return expression(depth+1);};
  if(op>=0x60){if(op<0x70)byte();parameters(depth);}
  else switch(op){
   case 0:case 1:case 2:case 0x20:case 0x29:reference();break;
   case 4:sub();break;
   case 5:byte();sub();break;
   case 6:case 7:{uint16_t target=word();script_.jumps.emplace_back(instruction.offset,target);if(op==7)sub();break;}
   case 9:word();sub();break;
   case 0x0a:{uint16_t target=word();if(target!=65535){script_.jumps.emplace_back(instruction.offset,target);sub();}break;}
   case 0x0c:{
    bool terminated=false;
    for(unsigned count=0;count<100000;++count){int32_t n=reference(true);uint32_t target=dword();if(names_[n]=="None"){terminated=true;break;}script_.jumps.emplace_back(instruction.offset,target);}
    if(!terminated){throw std::runtime_error("Too many labels");}
    break;
   }
   case 0x0d:case 0x0e:case 0x2d:case 0x37:sub();break;
   case 0x0f:case 0x10:case 0x14:case 0x1a:case 0x45:sub();sub();break;
   case 0x11:for(int i=0;i<4;++i)sub();break;
   case 0x12:case 0x19:sub();word();byte();sub();break;
   case 0x13:case 0x2e:reference();sub();break;
   case 0x18:word();sub();break;
   case 0x1b:case 0x38:reference(true);parameters(depth);break;
   case 0x1c:reference();parameters(depth);break;
   case 0x1d:case 0x1e:raw(4);break;
   case 0x1f:while(byte()){}break;
   case 0x21:reference(true);break;
   case 0x22:case 0x23:raw(12);break;
   case 0x24:case 0x2c:byte();break;
   case 0x2f:{sub();uint16_t target=word();script_.jumps.emplace_back(instruction.offset,target);break;}
   case 0x32:case 0x33:reference();sub();sub();break;
   case 0x34:while(word()){}break;
   case 0x35:raw(8);break;
   case 0x36:reference();sub();break;
   case 0x39:byte();sub();break;
   case 0x3a:reference();break;
   case 0x3b:case 0x3c:case 0x3d:case 0x3e:sub();sub();break;
   case 0x40:case 0x41:sub();sub();sub();break;
   case 0x42:raw(13);break;
   case 0x43:reference();reference(true);parameters(depth);break;
   case 0x44:reference(true);break;
   case 0x48:word();sub();word();sub();sub();break;
   case 8:case 0x0b:case 0x16:case 0x17:case 0x25:case 0x26:case 0x27:case 0x28:case 0x2a:case 0x30:case 0x31:case 0x3f:case 0x47:break;
   default:throw std::runtime_error("Unsupported HP3 opcode");
  }
  script_.instructions[index].end=position();return op;
 }
 Reader& source_;const std::vector<std::string>& names_;
 uint32_t imports_,exports_,limit_;size_t start_;Script script_;
};
} // namespace hp3

