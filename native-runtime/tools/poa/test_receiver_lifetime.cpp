#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Receiver lifetime assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected lifetime rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType type=ScalarType::Object){Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,type,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,1,{}});
 auto setter=image(1,{0x0f,1,5,0,6,4,1,5},17);setter.arguments={{6,ScalarType::Object,false,0,1}};p.add(setter);p.add(image(2,{4,1,5},6));p.add(image(3,{4,0x17},2));p.add(image(4,{0x0f,1,5,0x2a,4,1,5},13));p.add(image(5,{4,0x19,1,5,1,0,4,0x17},11));p.add(image(6,{4,0x36,7,1,8},11,ScalarType::Int));
 ReceiverProgram receivers(p,resolver);for(uint32_t i=1;i<=6;++i)receivers.bindFunction(i,1,{"None"});receivers.bindObjectProperty(1,1,1);for(auto i:{1u,2u,4u,5u})receivers.bindPropertyReference(i,5,1);receivers.bindStructProperty(20,1,{{{1},ScalarType::Int,int32_t(12)}});receivers.bindStructReference(6,8,20);receivers.bindStructMemberReference(6,7,1);
 receivers.create(1,1);receivers.create(2,2);receivers.create(3,1);check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{2}})).identity==2);check(std::get<int32_t>(receivers.invoke(2,6))==12);
 receivers.retire(2);rejects([&]{receivers.invoke(2,3);},"identity unavailable");rejects([&]{receivers.invoke(1,1,{ObjectRef{2}});},"identity unavailable");rejects([&]{receivers.invoke(1,2);},"identity unavailable");rejects([&]{receivers.invoke(1,5);},"identity unavailable");rejects([&]{receivers.create(2,2);},"cannot be reused");rejects([&]{receivers.retire(2);},"identity unavailable");rejects([&]{receivers.retire(99);},"identity unavailable");
 check(std::get<ObjectRef>(receivers.invoke(1,4)).identity==0);check(std::get<ObjectRef>(receivers.invoke(1,1,{ObjectRef{3}})).identity==3);check(std::get<ObjectRef>(receivers.invoke(3,3)).identity==3);
 receivers.retire(1);receivers.retire(3);rejects([&]{receivers.bindProperty(2,1,ScalarType::Int,int32_t(0));},"precede receiver");rejects([&]{receivers.bindClassDefault(1,1,ObjectRef{});},"precede receiver");rejects([&]{receivers.bindStructProperty(21,1,{{{1},ScalarType::Int,int32_t(0)}});},"precede receiver");rejects([&]{receivers.bindStructDefault(1,20,{1},int32_t(0));},"precede receiver");
 receivers.create(4,1);check(std::get<int32_t>(receivers.invoke(4,6))==12);check(std::get<ObjectRef>(receivers.invoke(4,2)).identity==0);rejects([&]{receivers.create(1,1);},"cannot be reused");
 StructProperties store(resolver);store.define(1,1,{{{1},ScalarType::Int,int32_t(9)}});store.create(1,1);store.retire(1);rejects([&]{store.read(1,1,{1});},"receiver unavailable");rejects([&]{store.define(2,1,{{{1},ScalarType::Int,int32_t(0)}});},"precede receiver");rejects([&]{store.bindDefault(1,1,{1},int32_t(0));},"precede receiver");
 std::cout<<"PASS registry lifetime: retired invocation/arguments/field reads/context rejected, identity reuse blocked, explicit stale-field clearing, struct cleanup and permanent schema freeze\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
