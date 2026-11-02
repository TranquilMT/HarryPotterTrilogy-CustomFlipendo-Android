#include "../../src/compat/ue2/ReceiverProgram.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Context corpus mismatch");}
static std::vector<uint8_t> blob(Reader& r){auto n=r.dword();check(n<=64*1024*1024);std::vector<uint8_t> result;while(n--)result.push_back(r.byte());return result;}
int main(int argc,char** argv){try{
 check(argc==2);std::ifstream file(argv[1],std::ios::binary);check(bool(file));std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});Reader r(bytes);auto cases=r.dword();check(cases>0&&cases<=1000000);size_t invocations=0;
 for(uint32_t i=0;i<cases;++i){auto raw=blob(r);auto memory=r.dword(),imports=r.dword(),exports=r.dword(),count=r.dword();check(count<=1000000);std::vector<std::string> names;while(count--){auto text=blob(r);names.emplace_back(text.begin(),text.end());}auto root=int32_t(r.dword()),field=int32_t(r.dword());auto type=ScalarType(r.dword());Reader script(raw);BytecodeReader decoder(script,names,imports,exports,memory);FunctionImage f;f.id=1;f.script=decoder.read();check(!script.remaining());f.result={int32_t(exports+1),type,false,0};
  std::vector<uint8_t> setterRaw={0x0f,1,1,0,7,4,1,1};Reader sr(setterRaw);std::vector<std::string> setterNames={"None"};BytecodeReader sd(sr,setterNames,0,100,17);FunctionImage setter;setter.id=2;setter.script=sd.read();setter.arguments={{7,ScalarType::Object,false,0}};setter.result={100,ScalarType::Object,false,0};TypedProgram program;program.add(f);program.add(setter);DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,0,{}});
  for(unsigned variant=0;variant<(type==ScalarType::Bool?2u:1u);++variant){Scalar value;switch(type){case ScalarType::Bool:value=bool(variant);break;case ScalarType::Int:value=int32_t(13);break;case ScalarType::Float:value=2.5f;break;case ScalarType::Byte:value=int32_t(7);break;default:throw std::runtime_error("Unsupported context leaf");}ReceiverProgram receivers(program,resolver);receivers.bindFunction(1,1,names);receivers.bindFunction(2,1,setterNames);receivers.bindObjectProperty(1,1,2);receivers.bindProperty(2,2,type,value);receivers.bindPropertyReference(1,root,1);receivers.bindContextPropertyReference(1,field,2);receivers.bindPropertyReference(2,1,1);receivers.create(1,1);receivers.create(2,2);receivers.create(3,2);for(auto target:{2u,3u}){receivers.invoke(1,2,{ObjectRef{target}});check(receivers.invoke(1,1)==value);++invocations;}}
 }
 check(!r.remaining());std::cout<<"PASS "<<cases<<" retail-derived non-null context fragments, "<<invocations<<" invocations on explicit host receivers; no original actor/default/native behavior claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
