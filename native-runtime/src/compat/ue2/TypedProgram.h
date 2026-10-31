// Isolated typed calls and explicit scalar Core intrinsics. GPL-3.0.
// No UObject allocation, receiver, virtual dispatch or world integration here.
#pragma once
#include "ScalarVM.h"
#include <array>
namespace hp3 {
struct Slot { int32_t reference=0;ScalarType type=ScalarType::Void;bool optional=false;uint32_t flags=0; };
enum class Intrinsic:uint32_t { None,AddInt,SubtractInt,MultiplyInt,LessInt,GreaterInt,EqualInt,NotEqualInt,NotBool,AddFloat,SubtractFloat,MultiplyFloat };
struct FunctionImage {
 uint32_t id=0,nativeIndex=0;std::string qualifiedName;Script script;
 std::vector<Slot> arguments,locals;Slot result;Intrinsic intrinsic=Intrinsic::None;
 bool native=false;std::map<int32_t,uint32_t> finalTargets;
};
class TypedProgram {
public:
 void add(FunctionImage image){
  if(!image.id||functions_.count(image.id))throw std::runtime_error("Duplicate or zero function identity");
  std::set<int32_t> seen;
  auto slot=[&](const Slot& s){if(s.reference<=0||(s.type<ScalarType::Bool||s.type>ScalarType::Byte)||!seen.insert(s.reference).second)throw std::runtime_error("Invalid or duplicate frame slot");};
  for(const auto& s:image.arguments){if(s.flags&(0x100|0x200))throw std::runtime_error("Out or lazy arguments are not implemented");slot(s);}
  for(const auto& s:image.locals)slot(s);
  if(image.result.type!=ScalarType::Void)slot(image.result);
  else if(image.result.reference)throw std::runtime_error("Void result has a property reference");
  if(image.native){
   verifyIntrinsic(image);
   if(nativeIndices_.count(image.nativeIndex))throw std::runtime_error("Duplicate native index in profile");
   nativeIndices_[image.nativeIndex]=image.id;
  }else if(image.intrinsic!=Intrinsic::None)throw std::runtime_error("Script function marked intrinsic");
  functions_.emplace(image.id,std::move(image));
 }
 Scalar invoke(uint32_t id,const std::vector<Scalar>& arguments={},size_t budget=10000)const{
  Context context{budget,0};return call(id,arguments,context);
 }
 size_t functionCount()const{return functions_.size();}
private:
 struct Context{size_t remaining,depth;};
 struct DepthGuard{size_t& depth;explicit DepthGuard(size_t& d):depth(d){if(depth>=64)throw std::runtime_error("Function recursion limit exceeded");++depth;}~DepthGuard(){--depth;}};
 static void verifyIntrinsic(const FunctionImage& f){
  struct Spec{const char* name;uint32_t index;ScalarType input,output;size_t count;};
  static const std::array<Spec,11> specs={{{"Core.Object.Add_IntInt",146,ScalarType::Int,ScalarType::Int,2},{"Core.Object.Subtract_IntInt",147,ScalarType::Int,ScalarType::Int,2},{"Core.Object.Multiply_IntInt",144,ScalarType::Int,ScalarType::Int,2},{"Core.Object.Less_IntInt",150,ScalarType::Int,ScalarType::Bool,2},{"Core.Object.Greater_IntInt",151,ScalarType::Int,ScalarType::Bool,2},{"Core.Object.EqualEqual_IntInt",154,ScalarType::Int,ScalarType::Bool,2},{"Core.Object.NotEqual_IntInt",155,ScalarType::Int,ScalarType::Bool,2},{"Core.Object.Not_PreBool",129,ScalarType::Bool,ScalarType::Bool,1},{"Core.Object.Add_FloatFloat",174,ScalarType::Float,ScalarType::Float,2},{"Core.Object.Subtract_FloatFloat",175,ScalarType::Float,ScalarType::Float,2},{"Core.Object.Multiply_FloatFloat",171,ScalarType::Float,ScalarType::Float,2}}};
  uint32_t i=uint32_t(f.intrinsic);if(!i||i>specs.size())throw std::runtime_error("Native intrinsic is not implemented");
  const auto& s=specs[i-1];
  if(f.qualifiedName!=s.name||f.nativeIndex!=s.index||f.result.type!=s.output||f.arguments.size()!=s.count||!f.locals.empty())throw std::runtime_error("Native profile signature mismatch");
  for(const auto& a:f.arguments)if(a.type!=s.input||a.optional)throw std::runtime_error("Native argument signature mismatch");
 }
 static int32_t wrapped(uint32_t bits){int32_t value;std::memcpy(&value,&bits,4);return value;}
 static Scalar intrinsic(const FunctionImage& f,const std::vector<Scalar>& a){
  if(f.intrinsic==Intrinsic::NotBool)return !std::get<bool>(a[0]);
  if(f.intrinsic>=Intrinsic::AddFloat){float x=std::get<float>(a[0]),y=std::get<float>(a[1]);switch(f.intrinsic){case Intrinsic::AddFloat:return x+y;case Intrinsic::SubtractFloat:return x-y;case Intrinsic::MultiplyFloat:return x*y;default:throw std::runtime_error("Invalid float intrinsic");}}
  int32_t x=std::get<int32_t>(a[0]),y=std::get<int32_t>(a[1]);
  switch(f.intrinsic){case Intrinsic::AddInt:return wrapped(uint32_t(x)+uint32_t(y));case Intrinsic::SubtractInt:return wrapped(uint32_t(x)-uint32_t(y));case Intrinsic::MultiplyInt:return wrapped(uint32_t(x)*uint32_t(y));case Intrinsic::LessInt:return x<y;case Intrinsic::GreaterInt:return x>y;case Intrinsic::EqualInt:return x==y;case Intrinsic::NotEqualInt:return x!=y;default:throw std::runtime_error("Unsupported intrinsic");}
 }
 Scalar call(uint32_t id,const std::vector<Scalar>& values,Context& context)const{
  DepthGuard depth(context.depth);if(!context.remaining)throw std::runtime_error("Shared call budget exhausted");--context.remaining;
  auto found=functions_.find(id);if(found==functions_.end())throw std::runtime_error("Function implementation unavailable");
  const auto& f=found->second;if(values.size()>f.arguments.size())throw std::runtime_error("Too many function arguments");
  ScalarFrame frame;frame.strictLayout=true;
  auto declare=[&](const Slot& s){frame.layout.emplace(s.reference,s.type);frame.locals.emplace(s.reference,zero(s.type));};
  for(const auto& s:f.arguments)declare(s);
  for(const auto& s:f.locals)declare(s);
  if(f.result.type!=ScalarType::Void)declare(f.result);
  std::vector<Scalar> bound;
  for(size_t i=0;i<f.arguments.size();++i){
   const auto& s=f.arguments[i];bool omitted=i>=values.size()||std::holds_alternative<std::monostate>(values[i]);
   if(omitted&&!s.optional)throw std::runtime_error("Missing required function argument");
   Scalar value=omitted?zero(s.type):values[i];if(!matches(s.type,value))throw std::runtime_error("Function argument type mismatch");frame.store(s.reference,value);bound.push_back(value);
  }
  Scalar result;
  if(f.native)result=intrinsic(f,bound);
  else{
   auto finalCall=[&](int32_t reference,const std::vector<Scalar>& args){auto target=f.finalTargets.find(reference);if(target==f.finalTargets.end())throw std::runtime_error("Unresolved final function reference");return call(target->second,args,context);};
   auto nativeCall=[&](int32_t index,const std::vector<Scalar>& args){auto target=nativeIndices_.find(uint32_t(index));if(target==nativeIndices_.end())throw std::runtime_error("Native index is not implemented");return call(target->second,args,context);};
   ScalarVM vm(f.script,context.remaining,&context.remaining,finalCall,nativeCall);result=vm.run(frame);
   // Match the existing VM: a successful Nothing return has the declared default value.
   // Exceptions for unavailable behavior never reach this conversion.
   if(std::holds_alternative<std::monostate>(result)&&f.result.type!=ScalarType::Void)result=zero(f.result.type);
  }
  if(!matches(f.result.type,result)){throw std::runtime_error("Function return type mismatch");}
  return result;
 }
 std::map<uint32_t,FunctionImage> functions_;std::map<uint32_t,uint32_t> nativeIndices_;
};
} // namespace hp3
