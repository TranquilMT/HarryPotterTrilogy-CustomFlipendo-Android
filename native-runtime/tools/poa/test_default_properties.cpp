#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Default-property assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected default rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType type=ScalarType::Int){Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,type,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,1,{}});resolver.addClass({3,2,{}});resolver.addClass({4,0,{}});
 p.add(image(1,{4,2,5},6));p.add(image(2,{4,1,5},6));auto write=image(3,{0x0f,1,5,0,6,4,1,5},17);write.arguments.push_back({6,ScalarType::Int,false,0});p.add(write);
 p.add(image(4,{4,0x2d,2,5},7,ScalarType::Bool));p.add(image(5,{4,1,5},6,ScalarType::Bool));p.add(image(6,{0x14,0x2d,1,5,0x28,4,0x2d,1,5},15,ScalarType::Bool));
 auto nested=image(7,{4,0x1c,1,0x16},7);nested.finalTargets[1]=1;p.add(nested);
 p.add(image(8,{4,2,0x81},6));p.add(image(9,{0x0f,2,5,0x26,4,2,5},13));p.add(image(10,{0x14,0x2d,2,5,0x28,4,0x2d,2,5},15,ScalarType::Bool));p.add(image(11,{4,0x2d,2,5},7,ScalarType::Bool));p.add(image(12,{4,2,5},6));
 ReceiverProgram receivers(p,resolver);for(uint32_t i=1;i<=12;++i)receivers.bindFunction(i,1,{"None"});
 receivers.bindProperty(1,1,ScalarType::Int,int32_t(3));receivers.bindProperty(2,1,ScalarType::Bool,false);receivers.bindProperty(3,1,ScalarType::Byte,int32_t(0));
 for(auto fn:{1u,2u,3u,7u,9u,11u})receivers.bindPropertyReference(fn,5,1);
 for(auto fn:{4u,5u,6u,10u}){receivers.bindPropertyReference(fn,5,2);}
 receivers.bindPropertyReference(8,-1,1);
 receivers.bindClassDefault(2,1,int32_t(9));receivers.bindClassDefault(3,1,int32_t(12));receivers.bindClassDefault(2,2,true);
 rejects([&]{receivers.bindClassDefault(2,1,int32_t(1));},"Duplicate");rejects([&]{receivers.bindClassDefault(4,1,int32_t(1));},"unrelated");rejects([&]{receivers.bindClassDefault(1,2,int32_t(1));},"type mismatch");rejects([&]{receivers.bindClassDefault(1,3,int32_t(256));},"type mismatch");rejects([&]{receivers.bindClassDefault(1,999,int32_t(1));},"schema unavailable");
 receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);receivers.create(4,2);
 auto n=[&](uint32_t receiver,uint32_t fn){return std::get<int32_t>(receivers.invoke(receiver,fn));};
 check(n(1,1)==3&&n(2,1)==9&&n(3,1)==12);check(n(1,2)==3&&n(2,2)==9&&n(3,2)==12);
 receivers.invoke(2,3,{int32_t(77)});check(n(2,2)==77&&n(2,1)==9&&n(4,2)==9&&n(1,1)==3);check(n(2,7)==9&&n(2,8)==9);
 check(!std::get<bool>(receivers.invoke(1,4)));check(std::get<bool>(receivers.invoke(2,4)));check(std::get<bool>(receivers.invoke(3,4)));check(!std::get<bool>(receivers.invoke(2,6)));check(std::get<bool>(receivers.invoke(2,4)));
 rejects([&]{receivers.invoke(2,9);},"writes are not implemented");rejects([&]{receivers.invoke(2,10);},"writes are not implemented");check(n(2,1)==9&&n(2,2)==77);
 rejects([&]{receivers.invoke(2,11);},"requires bool storage");rejects([&]{receivers.invoke(2,12);},"Unresolved instance property");rejects([&]{p.invoke(1);},"reader unavailable");rejects([&]{receivers.bindClassDefault(2,3,int32_t(1));},"precede receiver");
 std::cout<<"PASS primitive defaults: nearest inherited override, instance/default separation, independent receivers, wrapped bool, nested/imported access and explicit failures\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
