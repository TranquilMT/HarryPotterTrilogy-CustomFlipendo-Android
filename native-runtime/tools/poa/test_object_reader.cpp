#include "../../src/compat/ue2/ObjectReader.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <iterator>
using namespace hp3;
static void require(bool b){if(!b)throw std::runtime_error("Test failed");}
static void rejects(std::function<void()> f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}require(rejected);}
int main(int argc,char** argv){try{
 require(argc==2);std::ifstream file(argv[1],std::ios::binary);require(bool(file));
 std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),{});Reader input(data);size_t passed=0;
 uint32_t groups=input.dword();require(groups<10000);
 for(uint32_t g=0;g<groups;++g){
  uint32_t count=input.dword();require(count<1000000);std::vector<std::string> names;
  for(uint32_t i=0;i<count;++i){uint32_t length=input.dword();require(length<1048576);std::string s;for(uint32_t j=0;j<length;++j)s+=char(input.byte());names.push_back(s);}
  uint32_t cases=input.dword();require(cases<1000000);
  for(uint32_t i=0;i<cases;++i){
   uint32_t kind=input.dword(),length=input.dword();require(length<67108864);std::vector<uint8_t> raw;raw.reserve(length);for(uint32_t j=0;j<length;++j)raw.push_back(input.byte());
   uint32_t x=input.dword(),y=input.dword();Reader r(raw);
   if(kind==1){auto t=readTags(r,names);require(r.position()==x && t.size()==y);}
   else{require(kind==2);auto h=readFunctionHeader(r,names);auto n=readNativeStub(r,h);require(n.index==x && n.parameters.size()==y);}
   ++passed;
  }
 }
 require(input.remaining()==0);
 for(const auto& raw:std::vector<std::vector<uint8_t>>{{0x40},{0x7f,0xff,0xff,0xff,0xff}})rejects([&]{Reader r(raw);r.index();});
 std::vector<std::string> names={"None","Value"};
 for(const auto& raw:std::vector<std::vector<uint8_t>>{{2},{1,0},{1,0x72,0xff,0xff,0xff,0x7f}})rejects([&]{Reader r(raw);readTags(r,names);});
 std::vector<uint8_t> boolean={1,0x83,0};Reader br(boolean);auto t=readTags(br,names);require(t.size()==1 && t[0].boolean && t[0].payloadBytes==0 && br.remaining()==0);
 std::cout<<"PASS "<<passed<<" real object fixtures, malformed inputs and boolean encoding\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
