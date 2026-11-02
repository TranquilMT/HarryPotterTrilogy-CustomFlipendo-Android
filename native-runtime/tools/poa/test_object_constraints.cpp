#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Object constraint assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected constraint rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size){Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Object,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,0,{}});resolver.addClass({3,2,{}});resolver.addClass({4,0,{}});resolver.addClass({5,1,{}});
 auto setter=image(1,{0x0f,1,5,0,6,4,1,5},17);setter.arguments={{6,ScalarType::Object,false,0}};p.add(setter);p.add(image(2,{4,1,5},6));p.add(image(3,{0x0f,1,5,0x17,4,1,5},13));p.add(image(4,{0x0f,1,5,0x2a,4,1,5},13));
 auto nested=image(5,{4,0x1c,1,0,6,0x16},12);nested.arguments=setter.arguments;nested.finalTargets[1]=1;p.add(nested);
 ReceiverProgram receivers(p,resolver);for(uint32_t id=1;id<=5;++id){receivers.bindFunction(id,1,{"None"});}receivers.bindObjectProperty(1,1,2);for(auto id:{1u,2u,3u,4u})receivers.bindPropertyReference(id,5,1);
 rejects([&]{receivers.bindObjectProperty(2,1,0);},"target class");rejects([&]{receivers.bindObjectProperty(2,1,999);},"Missing dispatch");rejects([&]{receivers.bindProperty(2,1,ScalarType::Int,int32_t(0),2);},"requires an object");
 receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);receivers.create(4,4);receivers.create(5,5);receivers.create(6,1);
 auto read=[&](uint32_t id){return std::get<ObjectRef>(receivers.invoke(id,2)).identity;};auto write=[&](uint32_t id,uint32_t value){return std::get<ObjectRef>(receivers.invoke(id,1,{ObjectRef{value}})).identity;};
 check(read(1)==0&&read(5)==0);check(write(1,2)==2);check(write(1,3)==3);rejects([&]{write(1,4);},"class constraint mismatch");check(read(1)==3&&read(6)==0);rejects([&]{write(1,999);},"identity unavailable");check(read(1)==3);
 check(std::get<ObjectRef>(receivers.invoke(5,5,{ObjectRef{3}})).identity==3);rejects([&]{receivers.invoke(5,5,{ObjectRef{4}});},"class constraint mismatch");check(read(5)==3);rejects([&]{receivers.invoke(1,3);},"class constraint mismatch");check(read(1)==3);check(std::get<ObjectRef>(receivers.invoke(1,4)).identity==0);check(write(5,0)==0);
 rejects([&]{receivers.bindObjectProperty(2,1,2);},"precede receiver");
 // Existing unconstrained ObjectRef fields retain their explicit generic semantics.
 ReceiverProgram generic(p,resolver);generic.bindFunction(1,1,{"None"});generic.bindProperty(1,1,ScalarType::Object,ObjectRef{});generic.bindPropertyReference(1,5,1);generic.create(1,1);generic.create(4,4);check(std::get<ObjectRef>(generic.invoke(1,1,{ObjectRef{4}})).identity==4);
 std::cout<<"PASS object property constraints: exact class, subclass, inherited owner, null, nested writes, rejected unrelated/unknown handles, preserved values and generic compatibility\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
