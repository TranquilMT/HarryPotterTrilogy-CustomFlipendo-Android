#include "../../src/compat/ue2/DispatchResolver.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Dispatch assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected failure");}
static FunctionImage call(uint32_t id,uint8_t opcode,uint8_t name,const std::vector<std::string>& names){
 std::vector<uint8_t> bytes={4,opcode,name,0x16};Reader reader(bytes);BytecodeReader decoder(reader,names,0,100,7);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Int,false,0};return f;
}
static FunctionImage constant(uint32_t id,uint8_t value){
 std::vector<uint8_t> bytes={4,0x2c,value};Reader reader(bytes);std::vector<std::string> names={"None"};BytecodeReader decoder(reader,names,0,100,3);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,ScalarType::Int,false,0};return f;
}
int main(){try{
 DispatchResolver resolver;resolver.addClass({1,0,{{"Compute",10},{"Inherited",11}}});resolver.addClass({2,1,{{"COMPUTE",12}}});resolver.addState({3,0,{{"Compute",13}}});resolver.addState({4,3,{}});
 std::vector<std::string> names={"None","cOmPuTe","Inherited","Absent"};TypedProgram p;
 for(uint32_t id=10;id<=13;++id)p.add(constant(id,uint8_t(id)));
 auto v=call(20,0x1b,1,names);resolver.link(v,names,2,4);p.add(v);check(std::get<int32_t>(p.invoke(20))==13);
 auto g=call(21,0x38,1,names);resolver.link(g,names,2,4);p.add(g);check(std::get<int32_t>(p.invoke(21))==12);
 auto inherited=call(22,0x1b,2,names);resolver.link(inherited,names,2,4);p.add(inherited);check(std::get<int32_t>(p.invoke(22))==11);
 auto classOnly=call(23,0x1b,1,names);resolver.link(classOnly,names,2);p.add(classOnly);check(std::get<int32_t>(p.invoke(23))==12);
 std::vector<std::string> otherNames={"None","Inherited","Compute"};auto other=call(24,0x1b,1,otherNames);resolver.link(other,otherNames,2);p.add(other);check(std::get<int32_t>(p.invoke(24))==11);
 auto unknown=call(25,0x1b,3,names);unknown.virtualTargets[77]=99;rejects([&]{resolver.link(unknown,names,2);},"Unresolved metadata");check(unknown.virtualTargets.at(77)==99);
 rejects([&]{resolver.link(v,names,999);},"Missing dispatch");rejects([&]{resolver.link(v,names,0);},"Missing receiver");rejects([&]{resolver.link(v,names,2,999);},"Missing dispatch");rejects([&]{resolver.link(v,{"None"},2);},"outside caller");
 DispatchResolver cycle;cycle.addClass({1,2,{{"Compute",10}}});cycle.addClass({2,1,{}});rejects([&]{cycle.link(v,names,1);},"Cyclic");
 DispatchResolver missing;missing.addClass({1,99,{{"Compute",10}}});rejects([&]{missing.link(v,names,1);},"Missing dispatch parent");
 DispatchResolver stateCycle;stateCycle.addClass({1,0,{}});stateCycle.addState({1,1,{{"Compute",10}}});rejects([&]{stateCycle.link(v,names,1,1);},"Cyclic");
 rejects([&]{resolver.addClass({2,0,{}});},"Duplicate");rejects([&]{resolver.addClass({9,0,{{"A",10},{"a",11}}});},"Duplicate");rejects([&]{resolver.addClass({9,0,{{"A",0}}});},"zero");
 auto corrupt=v;corrupt.script.memory.resize(3);rejects([&]{resolver.link(corrupt,names,2);},"Truncated");
 std::cout<<"PASS metadata dispatch: state/class inheritance, global bypass, name folding, package-local names, transactional failure, missing/cyclic scopes and malformed calls\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
