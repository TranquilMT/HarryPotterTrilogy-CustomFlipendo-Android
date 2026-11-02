#include "../../src/compat/ue2/ReceiverProgram.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Instance corpus mismatch");}
static std::vector<uint8_t> blob(Reader& r){auto n=r.dword();check(n<=64*1024*1024);std::vector<uint8_t> result;while(n--)result.push_back(r.byte());return result;}
int main(int argc,char** argv){try{
 check(argc==2);std::ifstream file(argv[1],std::ios::binary);check(bool(file));std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});Reader r(bytes);auto cases=r.dword();check(cases>0&&cases<=1000000);
 size_t invocations=0;
 for(uint32_t i=0;i<cases;++i){
  auto raw=blob(r);auto memory=r.dword(),imports=r.dword(),exports=r.dword(),count=r.dword();check(count<=1000000);std::vector<std::string> names;while(count--){auto text=blob(r);names.emplace_back(text.begin(),text.end());}auto reference=r.dword();auto type=ScalarType(r.dword());Reader script(raw);BytecodeReader decoder(script,names,imports,exports,memory);FunctionImage f;f.id=1;f.script=decoder.read();check(!script.remaining());f.result={int32_t(exports+1),type,false,0};
  TypedProgram program;program.add(f);DispatchResolver resolver;resolver.addClass({1,0,{}});
  for(unsigned variant=0;variant<(type==ScalarType::Bool?2u:1u);++variant){
  ReceiverProgram receivers(program,resolver);receivers.bindFunction(1,1,names);
  Scalar supplied;switch(type){case ScalarType::Bool:supplied=bool(variant);break;case ScalarType::Int:supplied=int32_t(13);break;case ScalarType::Byte:supplied=int32_t(7);break;case ScalarType::Float:supplied=2.5f;break;default:throw std::runtime_error("Unexpected property type");}
  receivers.bindProperty(1,1,type,supplied);receivers.bindPropertyReference(1,int32_t(reference),1);receivers.create(1,1);receivers.create(2,1);
  check(receivers.invoke(1,1)==supplied);check(receivers.invoke(2,1)==supplied);invocations+=2;
  }
 }
 check(!r.remaining());std::cout<<"PASS "<<cases<<" retail-derived property cases, "<<invocations<<" invocations (both bool values), against explicit host property values on two isolated receivers; no retail defaults or actor behavior claimed\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
