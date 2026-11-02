#include "../../src/compat/ue2/ReceiverProgram.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Call constraint corpus mismatch");}
int main(int argc,char** argv){try{
 check(argc==2);std::ifstream input(argv[1],std::ios::binary);check(bool(input));std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});Reader r(bytes);auto count=r.dword();check(count>0&&count<1000000);DispatchResolver resolver;std::vector<uint32_t> classes;
 for(uint32_t i=0;i<count;++i){auto id=r.dword(),parent=r.dword();resolver.addClass({id,parent,{}});classes.push_back(id);}
 auto cases=r.dword();check(cases>0&&cases<=1000000);std::map<uint32_t,std::pair<uint32_t,uint32_t>> candidates;size_t exact=0,derived=0,nulls=0,rejected=0,optional=0,arguments=0,results=0;
 for(uint32_t i=0;i<cases;++i){auto owner=r.dword(),target=r.dword(),role=r.dword();check(role==1||role==2||role==5);if(!candidates.count(target)){uint32_t child=0,unrelated=0;for(auto cls:classes){if(resolver.isSubclass(cls,target)){if(cls!=target&&!child)child=cls;}else if(!unrelated)unrelated=cls;if(child&&unrelated)break;}candidates[target]={child,unrelated};}
  std::vector<uint8_t> raw={4,0,5};Reader source(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(source,names,0,100,6);FunctionImage f;f.id=1;f.script=decoder.read();f.arguments={{5,ScalarType::Object,bool(role&4),0,role==2?0:target}};f.result={100,ScalarType::Object,false,0,role==2?target:0};TypedProgram program;program.add(f);ReceiverProgram receivers(program,resolver);receivers.bindFunction(1,owner,names);receivers.create(1,owner);receivers.create(2,target);
  if(role==2)++results;else ++arguments;check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{2}})).identity==2);++exact;auto choices=candidates.at(target);
  if(choices.first){receivers.create(3,choices.first);check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{3}})).identity==3);++derived;}
  if(choices.second){receivers.create(4,choices.second);bool failed=false;try{receivers.invoke(1,1,{ObjectRef{4}});}catch(const std::runtime_error& e){failed=std::string(e.what()).find("class constraint mismatch")!=std::string::npos;}check(failed);++rejected;}
  check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{}})).identity==0);++nulls;if(role&4){check(std::get<ObjectRef>(receivers.invoke(1,1)).identity==0);++optional;}
 }
 check(!r.remaining());std::cout<<"PASS "<<cases<<" retail-metadata object call slots ("<<arguments<<" arguments, "<<results<<" returns): "<<exact<<" exact, "<<derived<<" subclass, "<<nulls<<" null, "<<optional<<" omitted optional; "<<rejected<<" unrelated-class rejections (synthetic identities, not original functions)\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
