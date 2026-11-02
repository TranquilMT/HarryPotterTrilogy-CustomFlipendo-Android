// Synthetic linker tests: no retail data or live object dispatch. GPL-3.0.
#include "../../src/compat/ue2/TypedProgram.h"
#include <iostream>
using namespace hp3;
static void require(bool ok){if(!ok)throw std::runtime_error("Named-call assertion failed");}
template<class F> static void fails(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){require(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected rejection");}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size){
 Reader reader(raw);std::vector<std::string> names={"None","Compute"};
 BytecodeReader decoder(reader,names,0,100,size);FunctionImage f;f.id=id;f.script=decoder.read();require(!reader.remaining());f.result={100,ScalarType::Int,false,0};return f;
}
int main(){try{
 TypedProgram p;
 auto identity=image(1,{4,0,5},6);identity.arguments.push_back({5,ScalarType::Int,false,0});p.add(identity);
 auto constant=image(2,{4,0x2c,42},3);p.add(constant);
 auto virtualCall=image(3,{4,0x1b,1,0x2c,7,0x16},9);virtualCall.virtualTargets[1]=1;p.add(virtualCall);
 require(std::get<int32_t>(p.invoke(3))==7);
 // The same package name resolves independently for a global call.
 auto globalCall=image(4,{4,0x38,1,0x16},7);globalCall.globalTargets[1]=2;globalCall.virtualTargets[1]=1;p.add(globalCall);
 require(std::get<int32_t>(p.invoke(4))==42);
 auto nested=image(5,{4,0x1b,1,0x38,1,0x16,0x16},13);nested.virtualTargets[1]=1;nested.globalTargets[1]=2;p.add(nested);
 require(std::get<int32_t>(p.invoke(5))==42);
 auto unbound=virtualCall;unbound.id=6;unbound.virtualTargets.clear();unbound.globalTargets[1]=1;p.add(unbound);
 fails([&]{p.invoke(6);},"Unresolved named");
 auto missing=virtualCall;missing.id=7;missing.virtualTargets[1]=999;p.add(missing);fails([&]{p.invoke(7);},"implementation unavailable");
 auto wrong=image(8,{4,0x1b,1,0x27,0x16},8);wrong.virtualTargets[1]=1;p.add(wrong);fails([&]{p.invoke(8);},"type mismatch");
 auto absent=image(9,{4,0x1b,1,0x16},7);absent.virtualTargets[1]=1;p.add(absent);fails([&]{p.invoke(9);},"required");
 auto recursive=image(10,{4,0x38,1,0x16},7);recursive.globalTargets[1]=10;p.add(recursive);
 fails([&]{p.invoke(10,{},10000);},"recursion");fails([&]{p.invoke(10,{},10);},"budget");
 fails([&]{p.invoke(5,{},2);},"budget");
 // Bindings remain caller-local, even when both packages use name index 1.
 auto other=globalCall;other.id=11;other.globalTargets[1]=3;p.add(other);require(std::get<int32_t>(p.invoke(11))==7);
 auto bare=virtualCall;ScalarFrame frame;ScalarVM vm(bare.script);fails([&]{vm.run(frame);},"handler unavailable");
 std::cout<<"PASS named calls: virtual/global separation, nested arguments, caller-local links, missing bindings/targets, strict arguments, recursion and shared budget\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
