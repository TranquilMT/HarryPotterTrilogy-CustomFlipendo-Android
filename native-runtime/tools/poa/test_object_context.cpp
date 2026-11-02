#include "../../src/compat/ue2/ReceiverProgram.h"
#include <iostream>
using namespace hp3;
static void check(bool ok){if(!ok)throw std::runtime_error("Object context assertion failed");}
template<class F> static void rejects(F fn,const std::string& reason){try{fn();}catch(const std::runtime_error& e){check(std::string(e.what()).find(reason)!=std::string::npos);return;}throw std::runtime_error("Expected object rejection: "+reason);}
static FunctionImage image(uint32_t id,std::vector<uint8_t> raw,uint32_t size,ScalarType type=ScalarType::Int){Reader r(raw);std::vector<std::string> names={"None","Read","Identity","Who","Write"};BytecodeReader decoder(r,names,10,100,size);FunctionImage f;f.id=id;f.script=decoder.read();f.result={100,type,false,0};return f;}
int main(){try{
 TypedProgram p;DispatchResolver resolver;resolver.addClass({1,0,{{"Read",1},{"Identity",2},{"Who",3},{"Write",11}}});resolver.addClass({2,1,{}});resolver.addClass({3,0,{}});
 p.add(image(1,{4,1,5},6));auto identity=image(2,{4,0,7},6);identity.arguments.push_back({7,ScalarType::Int,false,0});p.add(identity);
 p.add(image(3,{4,0x17},2,ScalarType::Object));p.add(image(4,{4,0x2a},2,ScalarType::Object));
 // Context target comes from an Object-typed argument; field read is on it.
 auto read=image(5,{4,0x19,0,6,5,0,4,1,5},15);read.arguments.push_back({6,ScalarType::Object,false,0});p.add(read);
 auto call=image(6,{4,0x19,0,6,6,0,4,0x1b,1,0x16},16);call.arguments=read.arguments;p.add(call);
 auto who=image(7,{4,0x19,0,6,6,0,4,0x1b,3,0x16},16,ScalarType::Object);who.arguments=read.arguments;p.add(who);
 // Self inside the context expression remains the caller's frame Self.
 auto self=image(8,{4,0x19,0,6,1,0,4,0x17},11,ScalarType::Object);self.arguments=read.arguments;p.add(self);
 // Arguments are evaluated on caller Self, then the call resumes on target.
 auto arg=image(9,{4,0x19,0,6,11,0,4,0x1b,2,1,5,0x16},21);arg.arguments=read.arguments;p.add(arg);
 auto wrong=image(10,{4,0x19,0x26,5,0,4,1,5},11);p.add(wrong);
 auto set=image(11,{0x0f,1,5,0,7,4,1,5},17);set.arguments.push_back({7,ScalarType::Int,false,0});p.add(set);
 auto save=image(12,{0x0f,1,8,0x17,4,1,8},13,ScalarType::Object);p.add(save);
 auto contextFromField=image(13,{4,0x19,1,8,5,0,4,1,5},15);p.add(contextFromField);
 auto saveArg=image(14,{0x0f,1,8,0,6,4,1,8},17,ScalarType::Object);saveArg.arguments=read.arguments;p.add(saveArg);
 // Cross-class field access requires an explicit context binding.
 auto foreign=image(15,{4,0x19,0,6,5,0,4,1,9},15);foreign.arguments=read.arguments;p.add(foreign);
 auto throwing=image(16,{4,0x19,0,6,5,0,4,1,0x63,1},15);throwing.arguments=read.arguments;p.add(throwing);
 auto mutate=image(17,{4,0x19,0,6,8,0,4,0x1b,4,0x2c,7,0x16},18);mutate.arguments=read.arguments;p.add(mutate);
 auto restore=image(18,{0x19,0,6,5,0,4,1,5,4,1,5},20);restore.arguments=read.arguments;p.add(restore);
 auto twice=image(19,{4,0x19,0,6,10,0,4,0x19,0x17,5,0,4,1,5},20);twice.arguments=read.arguments;p.add(twice);
 ReceiverProgram receivers(p,resolver);for(uint32_t id=1;id<=19;++id)receivers.bindFunction(id,1,{"None","Read","Identity","Who","Write"});
 receivers.bindProperty(1,1,ScalarType::Int,int32_t(3));receivers.bindProperty(2,1,ScalarType::Object,ObjectRef{});receivers.bindProperty(3,3,ScalarType::Int,int32_t(42));receivers.bindClassDefault(2,1,int32_t(9));
 for(auto id:{1u,5u,9u,10u,11u,13u,18u,19u})receivers.bindPropertyReference(id,5,1);
 for(auto id:{12u,13u,14u})receivers.bindPropertyReference(id,8,2);
 receivers.bindContextPropertyReference(15,9,3);receivers.create(1,1);receivers.create(2,2);receivers.create(3,3);
 check(std::get<ObjectRef>(receivers.invoke(1,3)).identity==1);check(std::get<ObjectRef>(receivers.invoke(2,3)).identity==2);check(std::get<ObjectRef>(receivers.invoke(1,4)).identity==0);
 check(std::get<int32_t>(receivers.invoke(1,5,{ObjectRef{2}}))==9);check(std::get<int32_t>(receivers.invoke(1,6,{ObjectRef{2}}))==9);check(std::get<ObjectRef>(receivers.invoke(1,7,{ObjectRef{2}})).identity==2);check(std::get<ObjectRef>(receivers.invoke(1,8,{ObjectRef{2}})).identity==1);check(std::get<int32_t>(receivers.invoke(1,9,{ObjectRef{2}}))==3);
 check(std::get<int32_t>(receivers.invoke(1,15,{ObjectRef{3}}))==42);rejects([&]{receivers.invoke(1,5,{ObjectRef{3}});},"unrelated receiver");
 check(std::get<ObjectRef>(receivers.invoke(1,12)).identity==1);check(std::get<int32_t>(receivers.invoke(1,13))==3);receivers.invoke(1,14,{ObjectRef{2}});check(std::get<int32_t>(receivers.invoke(1,13))==9);
 rejects([&]{receivers.invoke(1,5,{ObjectRef{}});},"Null object context");rejects([&]{receivers.invoke(1,5,{ObjectRef{999}});},"identity unavailable");rejects([&]{receivers.invoke(1,5,{int32_t(2)});},"argument type");rejects([&]{receivers.invoke(1,10);},"not an object");rejects([&]{receivers.invoke(1,16,{ObjectRef{2}});},"Unresolved instance");check(std::get<ObjectRef>(receivers.invoke(1,3)).identity==1);
 rejects([&]{receivers.invoke(1,14,{ObjectRef{999}});},"identity unavailable");check(std::get<int32_t>(receivers.invoke(1,13))==9);rejects([&]{receivers.invoke(1,6,{ObjectRef{2}},3);},"budget");check(std::get<int32_t>(receivers.invoke(1,1))==3);
 check(std::get<int32_t>(receivers.invoke(1,18,{ObjectRef{2}}))==3);check(std::get<int32_t>(receivers.invoke(1,19,{ObjectRef{2}}))==3);check(std::get<int32_t>(receivers.invoke(1,17,{ObjectRef{2}}))==7);check(std::get<int32_t>(receivers.invoke(2,1))==7&&std::get<int32_t>(receivers.invoke(1,1))==3);
 // Direct environment exposes cleanup state after an exception within Context.
 uint32_t current=1;std::vector<uint32_t> contexts,frames;TypedProgram::CallEnvironment environment;environment.validate=[](uint32_t){};environment.resolveNamed=[](uint32_t,int32_t,bool){return uint32_t(1);};environment.pushFrame=[&]{frames.push_back(current);};environment.popFrame=[&]{frames.pop_back();};environment.self=[&]{return ObjectRef{frames.back()};};environment.enterContext=[&](ObjectRef object){contexts.push_back(current);current=object.identity;};environment.leaveContext=[&]{current=contexts.back();contexts.pop_back();};environment.readProperty=[](uint32_t,int32_t)->Scalar{throw std::runtime_error("Injected target read failure");};
 rejects([&]{p.invokeWithEnvironment(5,{ObjectRef{2}},environment);},"Injected target");check(current==1&&contexts.empty()&&frames.empty());
 rejects([&]{p.invoke(3);},"Self reader unavailable");
 std::cout<<"PASS object contexts: Self/None, typed references, live-handle checks, target field/call access, caller Self/argument scope, cross-class bindings, context unwind, null rejection and shared budget\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
