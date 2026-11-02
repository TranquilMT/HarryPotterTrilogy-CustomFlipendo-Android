#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("BoolVariable assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected Boolean rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size){
 Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Bool,false,0};return f;
}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});
 auto local=image(1,{0x14,0x2d,0,5,0x27,4,0x2d,0,5},15);local.locals.push_back({5,ScalarType::Bool,false,0});p.add(local);
 p.add(image(2,{4,0x2d,1,5},7));
 p.add(image(3,{0x14,0x2d,1,5,0x27,4,0x2d,1,5},15));
 p.add(image(4,{0x0f,0x2d,1,5,0x28,4,0x2d,1,5},15));
 p.add(image(5,{4,0x2d,1,6},7));
 p.add(image(6,{0x14,0x2d,1,5,0x26,4,0x2d,1,5},15));
 p.add(image(7,{4,0x2d,0x27},3));
 auto wrongLocal=image(8,{4,0x2d,0,5},7);wrongLocal.locals.push_back({5,ScalarType::Int,false,0});p.add(wrongLocal);
 p.add(image(9,{4,0x2d,1,0x81},7));
 p.add(image(10,{0x14,0x2d,1,6,0x27,4,0x2d,1,5},15));
 // A wrapped Boolean controls a real VM branch; false => false, true => true.
 p.add(image(11,{7,11,0,0x2d,1,5,4,0x27,4,0x28},13));
 ReceiverProgram receivers(p,resolver);
 for(uint32_t i=1;i<=11;++i)receivers.bindFunction(i,1,{"None"});
 receivers.bindProperty(1,1,ScalarType::Bool,false);receivers.bindProperty(2,1,ScalarType::Int,int32_t(0));
 for(auto id:{2u,3u,4u,6u,10u,11u})receivers.bindPropertyReference(id,5,1);
 receivers.bindPropertyReference(5,6,2);receivers.bindPropertyReference(10,6,2);receivers.bindPropertyReference(9,-1,1);receivers.create(1,1);receivers.create(2,1);
 auto value=[&](uint32_t receiver,uint32_t function){return std::get<bool>(receivers.invoke(receiver,function));};
 check(std::get<bool>(p.invoke(1)));check(!value(1,2));check(!value(1,11));check(value(1,3));check(value(1,11));check(value(1,9));check(!value(2,2));check(!value(1,4));
 rejects([&]{receivers.invoke(1,5);},"requires bool storage");rejects([&]{receivers.invoke(1,6);},"requires bool");check(!value(1,2));
 rejects([&]{p.invoke(7);},"supported variable");rejects([&]{p.invoke(8);},"requires bool storage");rejects([&]{receivers.invoke(1,10);},"property type mismatch");
 rejects([&]{receivers.invoke(1,3,{},1);},"budget");check(!value(1,2));
 std::cout<<"PASS BoolVariable reads/assignments: locals, instances, branches, separate receivers, imported bindings, type errors, invalid wrappers and shared budgets\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
