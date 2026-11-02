// Cross-check the C++ resolver against independently generated graph lookups.
#include "../../src/compat/ue2/DispatchResolver.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Dispatch corpus mismatch");}
static uint32_t count(Reader& r){auto n=r.dword();check(n<=1000000);return n;}
static std::string text(Reader& r){auto n=count(r);check(n<=4096);std::string s;while(n--)s+=char(r.byte());return s;}
int main(int argc,char** argv){try{
 check(argc==2);std::ifstream input(argv[1],std::ios::binary);check(bool(input));std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});Reader r(bytes);DispatchResolver resolver;
 auto scopes=count(r);for(uint32_t i=0;i<scopes;++i){bool state=r.dword()!=0;DispatchScope s;s.id=r.dword();s.parent=r.dword();auto n=count(r);for(uint32_t j=0;j<n;++j){auto name=text(r);auto id=r.dword();check(s.functions.emplace(name,id).second);}if(state)resolver.addState(s);else resolver.addClass(s);}
 auto cases=r.dword();check(cases<=4000000);size_t resolved=0,rejected=0;
 for(uint32_t i=0;i<cases;++i){auto cls=r.dword(),state=r.dword();auto name=text(r);auto virt=r.dword(),glob=r.dword();
  for(auto opcode:{uint8_t(0x1b),uint8_t(0x38)}){
   std::vector<uint8_t> raw={4,opcode,1,0x16};std::vector<std::string> names={"None",name};Reader source(raw);BytecodeReader decoder(source,names,0,0,7);FunctionImage f;f.script=decoder.read();auto expected=opcode==0x1b?virt:glob;
   if(expected){resolver.link(f,names,cls,state);const auto& links=opcode==0x1b?f.virtualTargets:f.globalTargets;check(links.at(1)==expected);++resolved;}
   else{bool failed=false;try{resolver.link(f,names,cls,state);}catch(const std::runtime_error& e){failed=std::string(e.what()).find("Unresolved metadata")!=std::string::npos;}check(failed);++rejected;}
  }
 }
 check(!r.remaining());std::cout<<"PASS retail-table resolver corpus: "<<scopes<<" scopes, "<<resolved<<" resolved lookups, "<<rejected<<" absent lookups rejected (metadata agreement, not gameplay equivalence)\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
