#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Call class assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected call rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size){Reader r(raw);std::vector<std::string> names={"None","Constrained"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Object,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{{"Constrained",1}}});resolver.addClass({2,0,{}});resolver.addClass({3,2,{}});resolver.addClass({4,0,{}});
 auto f=image(1,{0x0f,1,5,0x26,4,0,6},13);f.arguments={{6,ScalarType::Object,false,0,2}};f.result.objectClass=2;p.add(f);
 auto result=image(2,{4,0,6},6);result.arguments={{6,ScalarType::Object,false,0}};result.result.objectClass=2;p.add(result);
 auto optional=result;optional.id=3;optional.arguments[0].optional=true;optional.arguments[0].objectClass=2;p.add(optional);
 auto final=image(4,{4,0x1c,1,0,6,0x16},12);final.arguments=result.arguments;final.finalTargets[1]=1;p.add(final);
 auto named=image(5,{4,0x1b,1,0,6,0x16},12);named.arguments=result.arguments;p.add(named);
 auto self=image(6,{4,0x17},2);self.result.objectClass=2;p.add(self);
 auto nothing=image(7,{4,0x2a},2);nothing.result.objectClass=2;p.add(nothing);
 auto badClass=result;badClass.id=8;badClass.result.objectClass=999;p.add(badClass);
 auto read=image(9,{4,1,5},6);read.result.type=ScalarType::Int;p.add(read);
 ReceiverProgram receivers(p,resolver);for(uint32_t id=1;id<=9;++id){receivers.bindFunction(id,1,{"None","Constrained"});}receivers.bindProperty(1,1,ScalarType::Int,int32_t(0));receivers.bindPropertyReference(1,5,1);receivers.bindPropertyReference(9,5,1);receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);receivers.create(4,4);
 auto invoke=[&](uint32_t function,uint32_t object){return std::get<ObjectRef>(receivers.invoke(1,function,{ObjectRef{object}})).identity;};
 rejects([&]{invoke(1,4);},"class constraint mismatch");check(std::get<int32_t>(receivers.invoke(1,9))==0);check(invoke(1,2)==2);check(invoke(1,3)==3);check(invoke(1,0)==0);
 check(invoke(2,2)==2&&invoke(2,3)==3&&invoke(2,0)==0);rejects([&]{invoke(2,4);},"class constraint mismatch");check(std::get<ObjectRef>(receivers.invoke(1,3)).identity==0);check(std::get<ObjectRef>(receivers.invoke(1,3,{std::monostate{}})).identity==0);
 check(invoke(4,3)==3&&invoke(5,3)==3);rejects([&]{invoke(4,4);},"class constraint mismatch");rejects([&]{invoke(5,4);},"class constraint mismatch");rejects([&]{invoke(1,999);},"identity unavailable");
 rejects([&]{receivers.invoke(1,6);},"class constraint mismatch");check(std::get<ObjectRef>(receivers.invoke(1,7)).identity==0);rejects([&]{invoke(8,0);},"Missing dispatch");rejects([&]{p.invoke(2,{ObjectRef{2}});},"validator unavailable");
 auto nonObject=image(10,{4,0x26},2);nonObject.result.type=ScalarType::Int;nonObject.result.objectClass=2;rejects([&]{p.add(nonObject);},"requires an object");
 auto local=result;local.id=11;local.locals={{7,ScalarType::Object,false,0,2}};rejects([&]{p.add(local);},"Local object");
 std::cout<<"PASS object call constraints: exact/subclass/null arguments and results, optional null defaults, pre-body rejection, final/named calls, Self rejection and invalid schemas\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
