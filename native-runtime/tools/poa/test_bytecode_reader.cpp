#include "../../src/compat/ue2/ScalarVM.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <iterator>
using namespace hp3;
static void require(bool b){if(!b)throw std::runtime_error("Bytecode fixture mismatch");}
static std::vector<uint8_t> blob(Reader& r){uint32_t n=r.dword();if(n>64*1024*1024)throw std::runtime_error("Invalid fixture size");std::vector<uint8_t> b;for(uint32_t i=0;i<n;++i)b.push_back(r.byte());return b;}
static void rejects(std::function<void()> f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed);}
int main(int argc,char** argv){try{
 require(argc==2);std::ifstream f(argv[1],std::ios::binary);require(bool(f));std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),{});Reader input(data);size_t passed=0,tokens=0,jumps=0,constantReturns=0,voidReturns=0;
 uint32_t groups=input.dword();require(groups<10000);
 for(uint32_t g=0;g<groups;++g){
  uint32_t n=input.dword();require(n<1000000);std::vector<std::string> names;
  for(uint32_t i=0;i<n;++i){auto b=blob(input);names.emplace_back(b.begin(),b.end());}
  uint32_t imports=input.dword(),exports=input.dword(),cases=input.dword();require(cases<1000000);
  for(uint32_t c=0;c<cases;++c){
   auto raw=blob(input),expected=blob(input);Reader source(raw);auto h=readFunctionHeader(source,names);BytecodeReader decoder(source,names,imports,exports,h.bytecodeMemoryBytes);auto script=decoder.read();require(script.memory==expected);
   uint32_t count=input.dword();require(script.instructions.size()==count);
   for(const auto& i:script.instructions){require(input.dword()==i.offset);require(input.dword()==i.serializedOffset);require(input.dword()==i.end);require(input.dword()==i.depth);require(input.dword()==i.opcode);}
   require(input.dword()==script.jumps.size());uint16_t native=source.word();uint8_t precedence=source.byte();uint32_t flags=source.dword();if(flags&0x40)source.word();require(!source.remaining());
   require(input.dword()==native);require(input.dword()==precedence);require(input.dword()==flags);++passed;tokens+=count;jumps+=script.jumps.size();
   if(!(flags&0x400) && script.memory.size()>1 && script.memory[0]==4){
    uint8_t op=script.memory[1];bool selected=op==0x0b||op==0x1d||op==0x1e||op==0x24||op==0x25||op==0x26||op==0x27||op==0x28||op==0x2c;
    if(selected){ScalarFrame frame;ScalarVM vm(script);Scalar result=vm.run(frame);Reader expected(script.memory);expected.byte();expected.byte();
     if(op==0x0b){require(std::holds_alternative<std::monostate>(result));++voidReturns;}
     else{++constantReturns;
      if(op==0x27||op==0x28)require(std::get<bool>(result)==(op==0x27));
      else if(op==0x1e){uint32_t bits=expected.dword(),actual;float value=std::get<float>(result);std::memcpy(&actual,&value,4);require(actual==bits);}
      else{int32_t value=op==0x1d?int32_t(expected.dword()):(op==0x25?0:(op==0x26?1:expected.byte()));require(std::get<int32_t>(result)==value);}
     }
    }
   }
  }
 }
 require(!input.remaining());std::vector<std::string> names={"None"};
 for(const auto& bad:std::vector<std::vector<uint8_t>>{{0x03},{0x04},{0x06,1,0,0x04,0x0b},{0x00,2},{0x1b,0}}){rejects([&]{Reader r(bad);BytecodeReader d(r,names,0,0,uint32_t(bad.size()));d.read();});}
 auto decode=[&](std::vector<uint8_t> bytes,uint32_t memory){Reader r(bytes);BytecodeReader d(r,names,0,1,memory);return d.read();};
 {auto s=decode({0x0f,0,1,0x26,4,0,1},13);ScalarFrame f;ScalarVM vm(s);require(std::get<int32_t>(vm.run(f))==1);}
 {auto s=decode({7,6,0,0x28,4,0x26,4,0x25},8);ScalarFrame f;ScalarVM vm(s);require(std::get<int32_t>(vm.run(f))==0);}
 {auto s=decode({7,6,0,0x27,4,0x26,4,0x25},8);ScalarFrame f;ScalarVM vm(s);require(std::get<int32_t>(vm.run(f))==1);}
 for(const auto& item:std::vector<std::pair<std::vector<uint8_t>,uint32_t>>{{{6,0,0},3},{{4,0,1},6},{{4,0x17},2},{{0x70,0x16,4,0x0b},4},{{7,6,0,0x26,4,0x26,4,0x25},8}}){auto s=decode(item.first,item.second);rejects([&]{ScalarFrame f;ScalarVM vm(s,16);vm.run(f);});}
 std::cout<<"Executed "<<constantReturns<<" real constant-return functions and "<<voidReturns<<" original empty functions; local/branch/error tests passed\n";
 std::cout<<"PASS "<<passed<<" full functions, "<<tokens<<" tokens, "<<jumps<<" branch targets, malformed bytecode\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
