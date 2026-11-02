#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Struct default assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType type=ScalarType::Int){Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,type,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,1,{}});resolver.addClass({3,2,{}});resolver.addClass({4,0,{}});
 p.add(image(1,{4,0x36,5,2,6},11));p.add(image(2,{4,0x36,5,1,6},11));auto write=image(3,{0x0f,0x36,5,1,6,0,7,4,0x36,5,1,6},27);write.arguments.push_back({7,ScalarType::Int,false,0});p.add(write);
 p.add(image(4,{4,0x2d,0x36,8,2,6},12,ScalarType::Bool));p.add(image(5,{4,0x36,9,0x36,10,2,6},16,ScalarType::Float));
 p.add(image(6,{0x0f,0x36,5,2,6,0x26,4,0x36,5,2,6},23));p.add(image(7,{0x14,0x2d,0x36,8,2,6,0x28,4,0x2d,0x36,8,2,6},25,ScalarType::Bool));
 auto nested=image(8,{4,0x1c,1,0x16},7);nested.finalTargets[1]=1;p.add(nested);p.add(image(9,{4,0x36,0x81,2,0x82},11));
 ReceiverProgram receivers(p,resolver);for(uint32_t id=1;id<=9;++id){receivers.bindFunction(id,1,{"None"});}
 receivers.bindStructProperty(20,1,{{{1},ScalarType::Int,int32_t(3)},{{2},ScalarType::Bool,false},{{3,4},ScalarType::Float,1.5f},{{5},ScalarType::Byte,int32_t(0)}});
 for(uint32_t id=1;id<=8;++id){receivers.bindStructReference(id,6,20);receivers.bindStructMemberReference(id,5,1);receivers.bindStructMemberReference(id,8,2);receivers.bindStructMemberReference(id,9,4);receivers.bindStructMemberReference(id,10,3);}
 receivers.bindStructReference(9,-2,20);receivers.bindStructMemberReference(9,-1,1);
 receivers.bindStructDefault(2,20,{1},int32_t(9));receivers.bindStructDefault(3,20,{1},int32_t(12));receivers.bindStructDefault(2,20,{2},true);receivers.bindStructDefault(2,20,{3,4},2.5f);
 rejects([&]{receivers.bindStructDefault(2,20,{1},int32_t(1));},"Duplicate");rejects([&]{receivers.bindStructDefault(4,20,{1},int32_t(1));},"unrelated");rejects([&]{receivers.bindStructDefault(1,20,{2},int32_t(1));},"type mismatch");rejects([&]{receivers.bindStructDefault(1,20,{5},int32_t(256));},"type mismatch");rejects([&]{receivers.bindStructDefault(1,20,{9},int32_t(1));},"leaf unavailable");
 receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);receivers.create(4,2);
 auto n=[&](uint32_t receiver,uint32_t fn){return std::get<int32_t>(receivers.invoke(receiver,fn));};
 check(n(1,1)==3&&n(2,1)==9&&n(3,1)==12);check(n(2,2)==9&&n(3,2)==12);receivers.invoke(2,3,{int32_t(77)});check(n(2,2)==77&&n(2,1)==9&&n(4,2)==9);check(n(2,8)==9&&n(2,9)==9);
 check(!std::get<bool>(receivers.invoke(1,4))&&std::get<bool>(receivers.invoke(3,4)));check(std::get<float>(receivers.invoke(3,5))==2.5f);
 rejects([&]{receivers.invoke(2,6);},"writes are not implemented");rejects([&]{receivers.invoke(2,7);},"writes are not implemented");check(n(2,1)==9&&n(2,2)==77);rejects([&]{receivers.bindStructDefault(2,20,{5},int32_t(1));},"precede receiver");rejects([&]{p.invoke(1);},"reader unavailable");
 std::cout<<"PASS struct defaults: nearest inherited leaf override, copied instance values, default isolation, bool/nested/imported reads, readonly writes and schema failures\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
