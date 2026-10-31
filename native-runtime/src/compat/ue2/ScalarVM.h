// Isolated scalar execution only; not the live HP3 game VM. GPL-3.0.
#pragma once
#include "BytecodeReader.h"
#include <variant>
#include <map>
#include <cstring>
#include <functional>
namespace hp3 {
using Scalar=std::variant<std::monostate,bool,int32_t,float>;
enum class ScalarType:uint32_t { Void, Bool, Int, Float, Byte };
inline bool matches(ScalarType type,const Scalar& value){
 switch(type){
  case ScalarType::Void:return std::holds_alternative<std::monostate>(value);
  case ScalarType::Bool:return std::holds_alternative<bool>(value);
  case ScalarType::Int:return std::holds_alternative<int32_t>(value);
  case ScalarType::Float:return std::holds_alternative<float>(value);
  case ScalarType::Byte:{auto p=std::get_if<int32_t>(&value);return p&&*p>=0&&*p<=255;}
 }
 return false;
}
inline Scalar zero(ScalarType type){switch(type){case ScalarType::Bool:return false;case ScalarType::Int:case ScalarType::Byte:return int32_t(0);case ScalarType::Float:return 0.0f;default:return std::monostate{};}}
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
 using CallHandler=std::function<Scalar(int32_t,const std::vector<Scalar>&)>;
 explicit ScalarVM(const Script& script,size_t budget=10000,size_t* sharedBudget=nullptr,CallHandler finalCall={},CallHandler nativeCall={})
 :script_(script),localBudget_(budget),budget_(sharedBudget?sharedBudget:&localBudget_),finalCall_(std::move(finalCall)),nativeCall_(std::move(nativeCall)){
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
  std::vector<Scalar> values;
  while(true){
   if(ip_>=script_.memory.size())throw std::runtime_error("Unterminated call arguments");
   if(script_.memory[ip_]==0x16){++ip_;return values;}
   if(values.size()>=1024)throw std::runtime_error("Too many call arguments");
   values.push_back(expression(frame,depth+1));
  }
 }
 Scalar expression(ScalarFrame& frame,unsigned depth){
  if(depth>=128){throw std::runtime_error("Scalar expression depth exceeded");}
  tick();uint8_t op=byte();
  if(op>=0x60){int32_t index=op>=0x70?op:((op-0x60)<<8)|byte();if(!nativeCall_)throw std::runtime_error("Native call handler unavailable");auto values=arguments(frame,depth);return nativeCall_(index,values);}
  switch(op){
   case 0:case 0x3a:return frame.load(int32_t(dword()));
   case 0x0b:return std::monostate{};
   case 0x0f:case 0x14:{
    if(byte()!=0){throw std::runtime_error("Assignment requires a local slot");}
    int32_t ref=int32_t(dword());Scalar value=expression(frame,depth+1);
    if(op==0x14&&!std::holds_alternative<bool>(value))throw std::runtime_error("Bool assignment requires bool");
    frame.store(ref,value);return value;
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
 const Script& script_;size_t ip_=0,localBudget_;size_t* budget_;std::set<uint32_t> statements_;CallHandler finalCall_,nativeCall_;
};
} // namespace hp3
