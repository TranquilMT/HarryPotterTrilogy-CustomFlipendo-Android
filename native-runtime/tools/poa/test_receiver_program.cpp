#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Receiver assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected receiver rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t memory){
 Reader source(raw);std::vector<std::string> names={"None","Probe","Other"};BytecodeReader decoder(source,names,1,100,memory);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Int,false,0};return f;
}
int main(){try{
 TypedProgram p;DispatchResolver resolver;
 resolver.addClass({1,0,{{"Probe",10}}});resolver.addClass({2,1,{{"Probe",11}}});resolver.addClass({3,0,{{"Probe",12}}});
 resolver.addState({100,0,{{"Probe",13}}});resolver.addState({101,100,{}});resolver.addState({102,0,{{"Probe",12}}});
 for(uint32_t i=10;i<=13;++i)p.add(image(i,{4,0x2c,uint8_t(i)},3));
 auto virt=image(20,{4,0x1b,1,0x16},7);virt.virtualTargets[1]=12;p.add(virt);
 p.add(image(21,{4,0x38,1,0x16},7));
 auto final=image(22,{4,0x1c,1,0x16},7);final.finalTargets[1]=20;p.add(final);
 p.add(image(23,{4,0x1b,2,0x16},7));
 auto otherFinal=image(24,{4,0x1c,1,0x16},7);otherFinal.finalTargets[1]=12;p.add(otherFinal);
 auto nested=image(25,{4,0x1b,1,0x16},7);p.add(nested);
 // Nested callee has another package's name table: index 2 means Probe there.
 resolver.addClass({4,2,{{"Other",23}}});
 auto recursive=image(26,{4,0x1b,1,0x16},7);p.add(recursive);resolver.addClass({5,1,{{"Probe",26}}});
 auto argument=image(27,{4,0,5},6);argument.arguments.push_back({5,ScalarType::Int,false,0});p.add(argument);resolver.addClass({6,1,{{"Probe",27}}});
 auto viaNative=image(28,{4,0x92,0x26,0x26,0x16},5);p.add(viaNative);
 FunctionImage native;native.id=29;native.native=true;native.nativeIndex=146;native.intrinsic=Intrinsic::AddInt;native.qualifiedName="Core.Object.Add_IntInt";native.arguments={{1,ScalarType::Int,false,0},{2,ScalarType::Int,false,0}};native.result={3,ScalarType::Int,false,0};p.add(native);
 auto argumentCall=image(30,{4,0x1b,1,0x2c,7,0x16},9);p.add(argumentCall);
 ReceiverProgram receivers(p,resolver);
 for(auto id:{10u,11u,13u,20u,21u,22u,24u,25u,26u,27u,28u,29u,30u})receivers.bindFunction(id,1,{"None","Probe","Other"});
 receivers.bindFunction(12,3,{"None"});receivers.bindFunction(23,1,{"None","Other","Probe"});
 receivers.bindState(100,1);receivers.bindState(101,2);receivers.bindState(102,3);
 receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);receivers.create(4,4);receivers.create(5,5);receivers.create(6,6);
 auto expect=[&](uint32_t receiver,uint32_t function,int32_t value){check(std::get<int32_t>(receivers.invoke(receiver,function))==value);};
 expect(1,20,10);expect(2,20,11);expect(3,12,12);expect(2,22,11);
 receivers.selectState(2,101);expect(2,20,13);expect(2,21,11);expect(2,22,13);expect(1,20,10);
 receivers.selectState(2,0);expect(2,20,11);
 rejects([&]{receivers.selectState(2,102);},"unrelated receiver");expect(2,20,11);
 // Invalid function ownership is enforced on both direct and final calls.
 rejects([&]{receivers.invoke(2,12);},"unrelated receiver");rejects([&]{receivers.invoke(2,24);},"unrelated receiver");
 // Other => callee 23 => Probe uses the callee's table, retaining receiver 4.
 auto viaOther=image(31,{4,0x1b,2,0x16},7);p.add(viaOther);receivers.bindFunction(31,1,{"None","Probe","Other"});expect(4,31,11);
 rejects([&]{receivers.invoke(5,26,{},10000);},"recursion");rejects([&]{receivers.invoke(5,26,{},5);},"budget");
 expect(6,30,7);rejects([&]{receivers.invoke(6,20);},"required");expect(2,28,2);
 rejects([&]{receivers.invoke(99,20);},"identity unavailable");rejects([&]{receivers.create(2,2);},"Duplicate");rejects([&]{receivers.bindFunction(20,1,{});},"Duplicate");
 p.add(image(32,{4,0x1b,1,0x16},7));receivers.bindFunction(32,1,{"None","Missing"});rejects([&]{receivers.invoke(2,32);},"Unresolved metadata");
 ReceiverProgram missing(p,resolver);missing.create(1,1);rejects([&]{missing.invoke(1,20);},"binding unavailable");
 ReceiverProgram badNames(p,resolver);badNames.create(1,1);badNames.bindFunction(20,1,{"None"});rejects([&]{badNames.invoke(1,20);},"outside caller");
 ReceiverProgram missingOwner(p,resolver);missingOwner.create(1,2);missingOwner.bindState(101,2);rejects([&]{missingOwner.selectState(1,101);},"owner binding unavailable");
 // Unrelated parent owners are rejected even if receiver inherits both scopes.
 DispatchResolver invalid;invalid.addClass({1,0,{}});invalid.addClass({2,1,{}});invalid.addState({1,0,{}});invalid.addState({2,1,{}});
 ReceiverProgram ancestry(p,invalid);ancestry.create(1,2);ancestry.bindState(1,2);ancestry.bindState(2,1);rejects([&]{ancestry.selectState(1,2);},"parent belongs");
 TypedProgram::CallEnvironment incomplete;rejects([&]{p.invokeWithEnvironment(20,{},incomplete);},"Incomplete");
 // Legacy explicit links remain available when no receiver environment is used.
 check(std::get<int32_t>(p.invoke(20))==12);
 std::cout<<"PASS receiver calls: per-instance dispatch/state selection, global bypass, final/native context propagation, caller-local names, ownership, shared limits and explicit failures\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
