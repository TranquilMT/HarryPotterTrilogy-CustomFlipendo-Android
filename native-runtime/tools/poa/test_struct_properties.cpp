#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Struct assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected struct rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType type=ScalarType::Int){Reader r(raw);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,type,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{}});resolver.addClass({2,1,{}});resolver.addClass({3,0,{}});
 p.add(image(1,{4,0x36,5,1,6},11));auto write=image(2,{0x0f,0x36,5,1,6,0,7,4,0x36,5,1,6},27);write.arguments.push_back({7,ScalarType::Int,false,0});p.add(write);
 p.add(image(3,{4,0x2d,0x36,8,1,6},12,ScalarType::Bool));p.add(image(4,{0x14,0x2d,0x36,8,1,6,0x27,4,0x2d,0x36,8,1,6},25,ScalarType::Bool));
 p.add(image(5,{4,0x36,9,0x36,10,1,6},16,ScalarType::Float));
 p.add(image(6,{4,0x36,0x81,1,0x82},11));p.add(image(7,{4,0x36,5,0,6},11));p.add(image(8,{4,0x36,5,2,6},11));
 auto byteWrite=write;byteWrite.id=9;byteWrite.result.type=ScalarType::Byte;p.add(byteWrite);
 auto bad=image(10,{0x0f,0x36,5,1,6,0x27,4,0x36,5,1,6},23);p.add(bad);
 auto nested=image(11,{4,0x1c,1,0x2c,7,0x16},9);nested.finalTargets[1]=2;p.add(nested);
 p.add(image(12,{4,0x36,11,1,6},11));p.add(image(13,{4,0x2d,0x36,5,1,6},12,ScalarType::Bool));
 ReceiverProgram receivers(p,resolver);for(uint32_t i=1;i<=13;++i)receivers.bindFunction(i,1,{"None"});
 std::vector<StructLeaf> leaves={{{1},ScalarType::Int,int32_t(3)},{{2},ScalarType::Bool,false},{{3,4},ScalarType::Float,1.5f},{{5},ScalarType::Byte,int32_t(0)}};
 receivers.bindStructProperty(20,1,leaves);
 for(auto fn:{1u,2u,3u,4u,5u,8u,9u,10u,12u,13u}){receivers.bindStructReference(fn,6,20);receivers.bindStructMemberReference(fn,5,fn==9?5:1);receivers.bindStructMemberReference(fn,8,2);receivers.bindStructMemberReference(fn,9,4);receivers.bindStructMemberReference(fn,10,3);}
 receivers.bindStructReference(6,-2,20);receivers.bindStructMemberReference(6,-1,1);receivers.create(1,1);receivers.create(2,2);
 auto n=[&](uint32_t receiver,uint32_t fn){return std::get<int32_t>(receivers.invoke(receiver,fn));};
 check(n(1,1)==3&&n(2,1)==3);check(std::get<int32_t>(receivers.invoke(1,2,{int32_t(9)}))==9);check(n(1,1)==9&&n(2,1)==3);check(n(1,6)==9);check(n(1,11)==7);check(n(1,1)==7);
 check(!std::get<bool>(receivers.invoke(1,3)));check(std::get<bool>(receivers.invoke(1,4)));check(!std::get<bool>(receivers.invoke(2,3)));check(std::get<float>(receivers.invoke(1,5))==1.5f);
 check(std::get<int32_t>(receivers.invoke(1,9,{int32_t(255)}))==255);rejects([&]{receivers.invoke(1,9,{int32_t(256)});},"type mismatch");rejects([&]{receivers.invoke(1,10);},"type mismatch");check(n(1,1)==7);
 rejects([&]{receivers.invoke(1,7);},"instance variable root");check(n(1,8)==3);rejects([&]{receivers.invoke(1,12);},"member reference");rejects([&]{receivers.invoke(1,13);},"requires bool storage");rejects([&]{p.invoke(1);},"reader unavailable");
 rejects([&]{receivers.invoke(1,2,{int32_t(8)},1);},"budget");check(n(1,1)==7);rejects([&]{receivers.bindStructProperty(21,1,leaves);},"precede receiver");
 ReceiverProgram invalid(p,resolver);rejects([&]{invalid.bindStructProperty(1,1,{{{1},ScalarType::Int,int32_t(0)},{{1,2},ScalarType::Int,int32_t(0)}});},"Overlapping");
 invalid.bindStructProperty(20,1,leaves);invalid.bindFunction(1,3,{});rejects([&]{invalid.bindStructReference(1,6,20);},"unrelated function");rejects([&]{invalid.bindProperty(20,1,ScalarType::Int,int32_t(0));},"already used");
 ReceiverProgram conflicts(p,resolver);conflicts.bindFunction(1,1,{});conflicts.bindProperty(1,1,ScalarType::Int,int32_t(0));conflicts.bindStructProperty(20,1,leaves);conflicts.bindPropertyReference(1,6,1);rejects([&]{conflicts.bindStructReference(1,6,20);},"already used");conflicts.bindStructReference(1,7,20);rejects([&]{conflicts.bindPropertyReference(1,7,1);},"already used");
 std::cout<<"PASS primitive instance structs: nested paths, typed leaves, separate receivers, inherited schemas, bool wrappers, imported refs, nested calls, unsupported roots and failures\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
