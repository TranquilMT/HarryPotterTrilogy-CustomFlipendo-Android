// Isolated scalar execution only; not the live HP3 game VM. GPL-3.0.
#pragma once
#include "BytecodeReader.h"
#include <variant>
#include <map>
#include <cstring>
#include <functional>
namespace hp3 {
struct ObjectRef{uint32_t identity=0;bool operator==(const ObjectRef& other)const{return identity==other.identity;}bool operator!=(const ObjectRef& other)const{return !(*this==other);}};
using Scalar=std::variant<std::monostate,bool,int32_t,float,ObjectRef>;
enum class ScalarType:uint32_t { Void, Bool, Int, Float, Byte, Object };
inline bool matches(ScalarType type,const Scalar& value){
 switch(type){
  case ScalarType::Void:return std::holds_alternative<std::monostate>(value);
  case ScalarType::Bool:return std::holds_alternative<bool>(value);
  case ScalarType::Int:return std::holds_alternative<int32_t>(value);
  case ScalarType::Float:return std::holds_alternative<float>(value);
  case ScalarType::Object:return std::holds_alternative<ObjectRef>(value);
  case ScalarType::Byte:{auto p=std::get_if<int32_t>(&value);return p&&*p>=0&&*p<=255;}
 }
 return false;
}
inline Scalar zero(ScalarType type){switch(type){case ScalarType::Bool:return false;case ScalarType::Int:case ScalarType::Byte:return int32_t(0);case ScalarType::Float:return 0.0f;case ScalarType::Object:return ObjectRef{};default:return std::monostate{};}}
struct ScalarFrame {
 std::map<int32_t,Scalar> locals;std::map<int32_t,ScalarType> layout;bool strictLayout=false;
 Scalar load(int32_t ref)const{auto it=locals.find(ref);if(it==locals.end())throw std::runtime_error("Uninitialized local variable");return it->second;}
 void store(int32_t ref,const Scalar& value){
  if(strictLayout){auto type=layout.find(ref);if(type==layout.end())throw std::runtime_error("Undeclared local variable");if(!matches(type->second,value))throw std::runtime_error("Local schema type mismatch");}
  else{auto old=locals.find(ref);if(old!=locals.end()&&old->second.index()!=value.index())throw std::runtime_error("Local value type changed");}
  locals[ref]=value;
 }
};
class ScalarVM {
public:
 using PropertyRead=std::function<Scalar(int32_t)>;
 using PropertyWrite=std::function<void(int32_t,const Scalar&)>;
 using StructRead=std::function<Scalar(int32_t,const std::vector<int32_t>&)>;
 using StructWrite=std::function<void(int32_t,const std::vector<int32_t>&,const Scalar&)>;
 using SelfRead=std::function<ObjectRef()>;
 using EnterContext=std::function<void(ObjectRef)>;
 using LeaveContext=std::function<void()>;
 using CallHandler=std::function<Scalar(int32_t,const std::vector<Scalar>&)>;
 explicit ScalarVM(const Script& script,size_t budget=10000,size_t* sharedBudget=nullptr,CallHandler finalCall={},CallHandler nativeCall={},CallHandler virtualCall={},CallHandler globalCall={},PropertyRead propertyRead={},PropertyWrite propertyWrite={},PropertyRead defaultRead={},StructRead structRead={},StructWrite structWrite={},StructRead defaultStructRead={},SelfRead selfRead={},EnterContext enterContext={},LeaveContext leaveContext={},LeaveContext beginArguments={},LeaveContext endArguments={})
 :script_(script),localBudget_(budget),budget_(sharedBudget?sharedBudget:&localBudget_),finalCall_(std::move(finalCall)),nativeCall_(std::move(nativeCall)),virtualCall_(std::move(virtualCall)),globalCall_(std::move(globalCall)),propertyRead_(std::move(propertyRead)),propertyWrite_(std::move(propertyWrite)),defaultRead_(std::move(defaultRead)),structRead_(std::move(structRead)),structWrite_(std::move(structWrite)),defaultStructRead_(std::move(defaultStructRead)),selfRead_(std::move(selfRead)),enterContext_(std::move(enterContext)),leaveContext_(std::move(leaveContext)),beginArguments_(std::move(beginArguments)),endArguments_(std::move(endArguments)){
  for(const auto& i:script.instructions)if(!i.depth)statements_.insert(i.offset);
 }
 ScalarVM(const ScalarVM&)=delete;ScalarVM& operator=(const ScalarVM&)=delete;
 Scalar run(ScalarFrame& frame){
  while(ip_<script_.memory.size()){
   tick();uint8_t op=byte();
   if(op==4)return expression(frame,0);
   if(op==8)return std::monostate{};
   if(op==6){jump(word());continue;}
   if(op==7){uint16_t target=word();Scalar condition=expression(frame,0);auto b=std::get_if<bool>(&condition);if(!b)throw std::runtime_error("Branch condition is not bool");if(!*b)jump(target);continue;}
   --ip_;expression(frame,0);
  }
  throw std::runtime_error("Scalar function ended without return");
 }
private:
 void tick(){if(!*budget_)throw std::runtime_error("Script execution budget exhausted");--*budget_;}
 uint8_t byte(){if(ip_>=script_.memory.size())throw std::runtime_error("Read outside expanded script");return script_.memory[ip_++];}
 uint16_t word(){uint16_t n=byte();return n|(uint16_t(byte())<<8);}
 uint32_t dword(){uint32_t n=word();return n|(uint32_t(word())<<16);}
 void jump(uint32_t target){if(!statements_.count(target))throw std::runtime_error("Invalid executable jump target");ip_=target;}
 std::vector<Scalar> arguments(ScalarFrame& frame,unsigned depth){
  if(beginArguments_)beginArguments_();
  struct Guard{const LeaveContext& leave;~Guard(){if(leave)leave();}} guard{endArguments_};
  std::vector<Scalar> values;
  while(true){
   if(ip_>=script_.memory.size())throw std::runtime_error("Unterminated call arguments");
   if(script_.memory[ip_]==0x16){++ip_;return values;}
   if(values.size()>=1024)throw std::runtime_error("Too many call arguments");
   values.push_back(expression(frame,depth+1));
  }
 }
 struct StructPath{bool defaults=false;int32_t root=0;std::vector<int32_t> members;};
 StructPath structPath(unsigned depth){
  if(depth>=32)throw std::runtime_error("Struct path depth exceeded");
  int32_t member=int32_t(dword());tick();uint8_t source=byte();StructPath path;
  if(source==0x36)path=structPath(depth+1);
  else if(source==1||source==2){path.defaults=source==2;path.root=int32_t(dword());}
  else throw std::runtime_error("Struct access requires an instance variable root");
  path.members.push_back(member);return path;
 }
 Scalar expression(ScalarFrame& frame,unsigned depth){
  if(depth>=128){throw std::runtime_error("Scalar expression depth exceeded");}
  tick();uint8_t op=byte();
  if(op>=0x60){int32_t index=op>=0x70?op:((op-0x60)<<8)|byte();if(!nativeCall_)throw std::runtime_error("Native call handler unavailable");auto values=arguments(frame,depth);return nativeCall_(index,values);}
  switch(op){
   case 0:case 0x3a:return frame.load(int32_t(dword()));
   case 1:{int32_t ref=int32_t(dword());if(!propertyRead_)throw std::runtime_error("Instance property reader unavailable");return propertyRead_(ref);}
   case 2:{int32_t ref=int32_t(dword());if(!defaultRead_)throw std::runtime_error("Default property reader unavailable");return defaultRead_(ref);}
   case 0x36:{auto path=structPath(0);const auto& read=path.defaults?defaultStructRead_:structRead_;if(!read)throw std::runtime_error("Struct reader unavailable");return read(path.root,path.members);}
   case 0x2d:{
    if(ip_>=script_.memory.size()||(script_.memory[ip_]!=0&&script_.memory[ip_]!=1&&script_.memory[ip_]!=2&&script_.memory[ip_]!=0x36))throw std::runtime_error("BoolVariable requires a supported variable");
    Scalar value=expression(frame,depth+1);if(!std::holds_alternative<bool>(value))throw std::runtime_error("BoolVariable requires bool storage");return value;
   }
   case 0x17:if(!selfRead_)throw std::runtime_error("Self reader unavailable");return selfRead_();
   case 0x2a:return ObjectRef{};
   case 0x19:{
    Scalar object=expression(frame,depth+1);word();byte();
    auto reference=std::get_if<ObjectRef>(&object);if(!reference)throw std::runtime_error("Context receiver is not an object reference");
    if(!reference->identity)throw std::runtime_error("Null object context semantics are not implemented");
    if(!enterContext_||!leaveContext_)throw std::runtime_error("Object context environment unavailable");
    enterContext_(*reference);
    struct Guard{const LeaveContext& leave;~Guard(){leave();}} guard{leaveContext_};
    return expression(frame,depth+1);
   }
   case 0x0b:return std::monostate{};
   case 0x0f:case 0x14:{
    uint8_t target=byte();bool wrapped=target==0x2d;
    if(wrapped){tick();target=byte();}
    if(target==0x36){
     auto path=structPath(0);if(path.defaults)throw std::runtime_error("Default struct writes are not implemented");if(!structWrite_)throw std::runtime_error("Struct writer unavailable");Scalar value=expression(frame,depth+1);
     if((op==0x14||wrapped)&&!std::holds_alternative<bool>(value))throw std::runtime_error("Bool assignment requires bool");
     structWrite_(path.root,path.members,value);return value;
    }
    if(target==2)throw std::runtime_error("Default property writes are not implemented");
    if(target!=0&&target!=1){throw std::runtime_error("Assignment requires a local or instance slot");}
    if(target==1&&!propertyWrite_)throw std::runtime_error("Instance property writer unavailable");
    int32_t ref=int32_t(dword());Scalar value=expression(frame,depth+1);
    if((op==0x14||wrapped)&&!std::holds_alternative<bool>(value))throw std::runtime_error("Bool assignment requires bool");
    if(target==0)frame.store(ref,value);else propertyWrite_(ref,value);return value;
   }
   case 0x1b:case 0x38:{
    int32_t name=int32_t(dword());const auto& handler=op==0x1b?virtualCall_:globalCall_;
    if(!handler)throw std::runtime_error("Named call handler unavailable");
    auto values=arguments(frame,depth);return handler(name,values);
   }
   case 0x1c:{int32_t ref=int32_t(dword());if(!finalCall_)throw std::runtime_error("Final call handler unavailable");auto values=arguments(frame,depth);return finalCall_(ref,values);}
   case 0x1d:return int32_t(dword());
   case 0x1e:{uint32_t bits=dword();float value;std::memcpy(&value,&bits,sizeof(value));return value;}
   case 0x24:case 0x2c:return int32_t(byte());
   case 0x25:return int32_t(0);
   case 0x26:return int32_t(1);
   case 0x27:return true;
   case 0x28:return false;
   default:throw std::runtime_error("Opcode requires unimplemented execution semantics");
  }
 }
 const Script& script_;size_t ip_=0,localBudget_;size_t* budget_;std::set<uint32_t> statements_;CallHandler finalCall_,nativeCall_,virtualCall_,globalCall_;PropertyRead propertyRead_;PropertyWrite propertyWrite_;PropertyRead defaultRead_;StructRead structRead_;StructWrite structWrite_;StructRead defaultStructRead_;SelfRead selfRead_;EnterContext enterContext_;LeaveContext leaveContext_,beginArguments_,endArguments_;
};
} // namespace hp3

