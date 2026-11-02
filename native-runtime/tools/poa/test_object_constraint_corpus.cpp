#include "../../src/compat/ue2/ReceiverProgram.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Object constraint corpus mismatch");}
int main(int argc,char** argv){try{
 check(argc==2);std::ifstream input(argv[1],std::ios::binary);check(bool(input));std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});Reader r(bytes);auto count=r.dword();check(count>0&&count<1000000);DispatchResolver resolver;std::vector<uint32_t> classes;
 for(uint32_t i=0;i<count;++i){auto id=r.dword(),parent=r.dword();resolver.addClass({id,parent,{}});classes.push_back(id);}
 std::vector<uint8_t> raw={0x0f,1,5,0,6,4,1,5};Reader source(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(source,names,0,100,17);FunctionImage setter;setter.id=1;setter.script=decoder.read();setter.arguments={{6,ScalarType::Object,false,0}};setter.result={100,ScalarType::Object,false,0};TypedProgram program;program.add(setter);
 std::vector<uint8_t> getterRaw={4,1,5};Reader getterSource(getterRaw);BytecodeReader getterDecoder(getterSource,names,0,100,6);FunctionImage getter;getter.id=2;getter.script=getterDecoder.read();getter.result={100,ScalarType::Object,false,0};program.add(getter);
 auto cases=r.dword();check(cases>0&&cases<=1000000);std::map<uint32_t,std::pair<uint32_t,uint32_t>> candidates;size_t exact=0,derived=0,nulls=0,rejected=0,retiredChecks=0;
 for(uint32_t i=0;i<cases;++i){auto owner=r.dword(),target=r.dword();if(!candidates.count(target)){uint32_t child=0,unrelated=0;for(auto cls:classes){if(resolver.isSubclass(cls,target)){if(cls!=target&&!child)child=cls;}else if(!unrelated)unrelated=cls;if(child&&unrelated)break;}candidates[target]={child,unrelated};}
  ReceiverProgram receivers(program,resolver);receivers.bindFunction(1,owner,names);receivers.bindFunction(2,owner,names);receivers.bindObjectProperty(1,owner,target);receivers.bindPropertyReference(1,5,1);receivers.bindPropertyReference(2,5,1);receivers.create(1,owner);receivers.create(2,target);
  check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{2}})).identity==2);++exact;auto choices=candidates.at(target);
  if(choices.first){receivers.create(3,choices.first);check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{3}})).identity==3);++derived;}
  if(choices.second){receivers.create(4,choices.second);bool failed=false;try{receivers.invoke(1,1,{ObjectRef{4}});}catch(const std::runtime_error& e){failed=std::string(e.what()).find("class constraint mismatch")!=std::string::npos;}check(failed);++rejected;}
  check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{}})).identity==0);++nulls;
  receivers.invoke(1,1,{ObjectRef{2}});receivers.retire(2);bool stale=false;try{receivers.invoke(1,2);}catch(const std::runtime_error& e){stale=std::string(e.what()).find("identity unavailable")!=std::string::npos;}check(stale);++retiredChecks;
  check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{}})).identity==0);
 }
 check(!r.remaining());std::cout<<"PASS "<<cases<<" retail-metadata ObjectProperty schemas: "<<exact<<" exact, "<<derived<<" subclass, "<<nulls<<" null assignments; "<<rejected<<" unrelated-class rejections, "<<retiredChecks<<" stale-field rejections (synthetic setters, not retail gameplay)\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
