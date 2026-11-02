#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Instance-property assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected property rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType result=ScalarType::Int){
 Reader r(raw);std::vector<std::string> names={"None","Read"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,result,false,0};return f;
}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{{"Read",1}}});resolver.addClass({2,1,{}});resolver.addClass({3,0,{}});
 p.add(image(1,{4,1,5},6));
 auto write=image(2,{0x0f,1,5,0,6,4,1,5},17);write.arguments.push_back({6,ScalarType::Int,false,0});p.add(write);
 auto shadow=image(3,{0x0f,0,5,0x2c,99,4,1,5},14);shadow.locals.push_back({5,ScalarType::Int,false,0});p.add(shadow);
 p.add(image(4,{0x0f,1,5,0x92,1,5,0x26,0x16,4,1,5},20));
 auto nested=image(5,{4,0x1c,1,0x2c,7,0x16},9);nested.finalTargets[1]=2;p.add(nested);
 auto wrong=image(6,{0x0f,1,5,0x27,4,1,5},13);p.add(wrong);
 auto boolWrite=image(7,{0x14,1,5,0x27,4,1,5},13,ScalarType::Bool);p.add(boolWrite);
 p.add(image(8,{4,1,0x81},6));
 auto byteWrite=write;byteWrite.id=9;byteWrite.result.type=ScalarType::Byte;p.add(byteWrite);
 auto floatWrite=write;floatWrite.id=10;floatWrite.arguments[0].type=ScalarType::Float;floatWrite.result.type=ScalarType::Float;p.add(floatWrite);
 p.add(image(11,{4,0x1b,1,0x16},7));
 p.add(image(12,{4,1,5},6));
 FunctionImage native;native.id=13;native.native=true;native.nativeIndex=146;native.intrinsic=Intrinsic::AddInt;native.qualifiedName="Core.Object.Add_IntInt";native.arguments={{1,ScalarType::Int,false,0},{2,ScalarType::Int,false,0}};native.result={3,ScalarType::Int,false,0};p.add(native);
 p.add(image(15,{4,1,5},6,ScalarType::Bool));p.add(image(16,{4,1,5},6,ScalarType::Float));p.add(image(17,{4,1,5},6,ScalarType::Byte));p.add(image(18,{0x14,1,5,0x26,4,1,5},13,ScalarType::Bool));
 ReceiverProgram receivers(p,resolver);
 for(uint32_t id=1;id<=18;++id)if(id!=14)receivers.bindFunction(id,1,{"None","Read"});
 receivers.bindProperty(1,1,ScalarType::Int,int32_t(3));receivers.bindProperty(2,1,ScalarType::Bool,false);receivers.bindProperty(3,2,ScalarType::Int,int32_t(50));receivers.bindProperty(4,1,ScalarType::Byte,int32_t(0));receivers.bindProperty(5,1,ScalarType::Float,1.5f);
 for(auto id:{1u,2u,3u,4u,5u,6u})receivers.bindPropertyReference(id,5,1);
 receivers.bindPropertyReference(7,5,2);receivers.bindPropertyReference(8,-1,1);receivers.bindPropertyReference(9,5,4);receivers.bindPropertyReference(10,5,5);
 // Reference 5 denotes a distinct property in another function/package.
 receivers.bindFunction(14,2,{"None"});p.add(image(14,{4,1,5},6));receivers.bindPropertyReference(14,5,3);
 receivers.bindPropertyReference(15,5,2);receivers.bindPropertyReference(16,5,5);receivers.bindPropertyReference(17,5,4);receivers.bindPropertyReference(18,5,2);
 receivers.create(1,1);receivers.create(2,2);
 auto number=[&](uint32_t receiver,uint32_t fn){return std::get<int32_t>(receivers.invoke(receiver,fn));};
 check(number(1,1)==3&&number(2,1)==3);check(std::get<int32_t>(receivers.invoke(1,2,{int32_t(9)}))==9);check(number(1,1)==9&&number(2,1)==3);
 check(number(1,3)==9);check(number(1,4)==10);check(number(1,8)==10);check(number(1,5)==7);check(number(1,11)==7);check(number(2,14)==50);check(number(2,1)==3);
 rejects([&]{receivers.invoke(1,6);},"property type mismatch");check(number(1,1)==7);
 check(!std::get<bool>(receivers.invoke(1,15))&&!std::get<bool>(receivers.invoke(2,15)));check(std::get<bool>(receivers.invoke(1,7)));check(!std::get<bool>(receivers.invoke(2,15)));
 rejects([&]{receivers.invoke(2,18);},"Bool assignment requires bool");check(!std::get<bool>(receivers.invoke(2,15)));
 check(std::get<int32_t>(receivers.invoke(1,9,{int32_t(255)}))==255);rejects([&]{receivers.invoke(1,9,{int32_t(256)});},"property type mismatch");
 check(std::get<int32_t>(receivers.invoke(1,17))==255&&std::get<int32_t>(receivers.invoke(2,17))==0);
 check(std::get<float>(receivers.invoke(1,10,{2.25f}))==2.25f);check(std::get<float>(receivers.invoke(2,16))==1.5f);
 rejects([&]{receivers.invoke(1,12);},"Unresolved instance property");rejects([&]{receivers.invoke(1,14);},"unrelated receiver");
 rejects([&]{receivers.bindPropertyReference(1,6,3);},"unrelated function");rejects([&]{receivers.bindPropertyReference(1,5,1);},"Duplicate");rejects([&]{receivers.bindPropertyReference(1,0,1);},"zero");rejects([&]{receivers.bindPropertyReference(99,5,1);},"binding unavailable");
 rejects([&]{receivers.bindProperty(6,1,ScalarType::Int,int32_t(0));},"precede receiver");
 ReceiverProgram defaults(p,resolver);rejects([&]{defaults.bindProperty(1,1,ScalarType::Byte,int32_t(-1));},"type or default");rejects([&]{defaults.bindProperty(1,1,ScalarType::Void,std::monostate{});},"type or default");defaults.bindProperty(1,1,ScalarType::Int,int32_t(0));rejects([&]{defaults.bindProperty(1,1,ScalarType::Int,int32_t(0));},"Duplicate");
 rejects([&]{p.invoke(1);},"reader unavailable");rejects([&]{p.invoke(2,{int32_t(1)});},"writer unavailable");
 rejects([&]{receivers.invoke(1,4,{},1);},"budget");check(number(1,1)==7);
 std::cout<<"PASS instance properties: independent receivers, inherited schemas, typed defaults, reads/writes, local separation, nested/named/native calls, package-local refs, bool/float/byte and explicit failures\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
