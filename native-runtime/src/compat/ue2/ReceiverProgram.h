// Isolated receiver identities and scalar calls; no live actors or state events.
// GPL-3.0.
#pragma once
#include "StructProperties.h"
namespace hp3 {
class ReceiverProgram {
public:
 ReceiverProgram(const TypedProgram& program,const DispatchResolver& resolver):program_(program),resolver_(resolver),structs_(resolver){}
 // Borrowed program/resolver must remain alive and unchanged during calls.
 void bindFunction(uint32_t id,uint32_t ownerClass,std::vector<std::string> names){
  resolver_.isSubclass(ownerClass,ownerClass);
  if(!id||!functions_.emplace(id,Binding{ownerClass,std::move(names),{}}).second)throw std::runtime_error("Duplicate or zero receiver function binding");
 }
 void bindState(uint32_t state,uint32_t ownerClass){
  resolver_.stateLineage(state);resolver_.isSubclass(ownerClass,ownerClass);
  if(!states_.emplace(state,ownerClass).second)throw std::runtime_error("Duplicate receiver state binding");
 }
 // Primitive schemas/defaults are supplied explicitly, before any receivers.
 void bindProperty(uint32_t id,uint32_t ownerClass,ScalarType type,const Scalar& initial,uint32_t objectClass=0){
  if(schemasFrozen_)throw std::runtime_error("Property schemas must precede receiver creation");
  resolver_.isSubclass(ownerClass,ownerClass);
  if(objectClass){if(type!=ScalarType::Object)throw std::runtime_error("Class constraint requires an object property");resolver_.isSubclass(objectClass,objectClass);}
  if(structs_.contains(id))throw std::runtime_error("Property identity already used by struct");
  if(auto object=std::get_if<ObjectRef>(&initial)){if(object->identity)throw std::runtime_error("Non-null object defaults require a verified loader");}
  if(type==ScalarType::Void||!matches(type,initial))throw std::runtime_error("Invalid instance property type or default");
  if(!id||!properties_.emplace(id,Property{ownerClass,type,initial,objectClass}).second)throw std::runtime_error("Duplicate or zero instance property");
 }
 void bindObjectProperty(uint32_t id,uint32_t ownerClass,uint32_t targetClass){
  if(!targetClass)throw std::runtime_error("Object property target class is required");
  bindProperty(id,ownerClass,ScalarType::Object,ObjectRef{},targetClass);
 }
 void bindPropertyReference(uint32_t function,int32_t reference,uint32_t property){
  auto binding=functions_.find(function);auto field=properties_.find(property);
  if(binding==functions_.end()||field==properties_.end())throw std::runtime_error("Property or function binding unavailable");
  if(!resolver_.isSubclass(binding->second.owner,field->second.owner))throw std::runtime_error("Property belongs to unrelated function class");
  auto structLinks=structRoots_.find(function);
  if(structLinks!=structRoots_.end()&&structLinks->second.count(reference))throw std::runtime_error("Property reference already used by struct");
  if(!reference||!binding->second.properties.emplace(reference,property).second)throw std::runtime_error("Duplicate or zero instance property reference");
 }
 // Explicit primitive class defaults only; no serialized default-object loader.
 void bindClassDefault(uint32_t receiverClass,uint32_t property,const Scalar& value){
  if(schemasFrozen_)throw std::runtime_error("Class defaults must precede receiver creation");
  auto schema=properties_.find(property);if(schema==properties_.end())throw std::runtime_error("Default property schema unavailable");
  if(!resolver_.isSubclass(receiverClass,schema->second.owner))throw std::runtime_error("Default belongs to unrelated receiver class");
  if(auto object=std::get_if<ObjectRef>(&value)){if(object->identity)throw std::runtime_error("Non-null object defaults require a verified loader");}
  if(!matches(schema->second.type,value))throw std::runtime_error("Class default type mismatch");
  auto found=defaults_.find(receiverClass);
  if(found!=defaults_.end()&&found->second.count(property))throw std::runtime_error("Duplicate class default");
  defaults_[receiverClass].emplace(property,value);
 }
 void bindStructProperty(uint32_t root,uint32_t owner,const std::vector<StructLeaf>& leaves){
  if(schemasFrozen_)throw std::runtime_error("Struct schemas must precede receiver creation");
  if(properties_.count(root))throw std::runtime_error("Struct identity already used by primitive property");
  structs_.define(root,owner,leaves);
 }
 void bindStructDefault(uint32_t receiverClass,uint32_t root,const MemberPath& path,const Scalar& value){
  if(schemasFrozen_)throw std::runtime_error("Struct defaults must precede receiver creation");
  structs_.bindDefault(receiverClass,root,path,value);
 }
 void bindStructReference(uint32_t caller,int32_t reference,uint32_t root){
  auto binding=functions_.find(caller);if(binding==functions_.end())throw std::runtime_error("Struct function binding unavailable");
  if(!resolver_.isSubclass(binding->second.owner,structs_.owner(root)))throw std::runtime_error("Struct belongs to unrelated function class");
  if(binding->second.properties.count(reference))throw std::runtime_error("Struct reference already used by primitive property");
  if(!reference||!structRoots_[caller].emplace(reference,root).second)throw std::runtime_error("Duplicate or zero struct reference");
 }
 void bindStructMemberReference(uint32_t caller,int32_t reference,uint32_t member){
  if(!functions_.count(caller)||!reference||!member)throw std::runtime_error("Invalid struct member binding");
  if(!structMembers_[caller].emplace(reference,member).second)throw std::runtime_error("Duplicate struct member binding");
 }
 void bindContextPropertyReference(uint32_t function,int32_t reference,uint32_t property){
  auto binding=functions_.find(function);if(binding==functions_.end()||!properties_.count(property))throw std::runtime_error("Context property binding unavailable");
  auto structs=structRoots_.find(function);if(structs!=structRoots_.end()&&structs->second.count(reference))throw std::runtime_error("Context property reference already used by struct");
  if(!reference||!binding->second.properties.emplace(reference,property).second)throw std::runtime_error("Duplicate or zero context property reference");
 }
 void create(uint32_t identity,uint32_t receiverClass){
  resolver_.isSubclass(receiverClass,receiverClass);
  if(retired_.count(identity))throw std::runtime_error("Retired receiver identities cannot be reused");
  std::map<uint32_t,Scalar> fields;
  for(const auto& field:properties_)if(resolver_.isSubclass(receiverClass,field.second.owner))fields.emplace(field.first,defaultValue(receiverClass,field.first));
  if(!identity||!receivers_.emplace(identity,Receiver{receiverClass,0,std::move(fields)}).second)throw std::runtime_error("Duplicate or zero receiver identity");
  structs_.create(identity,receiverClass);schemasFrozen_=true;
 }
 // Host-only registry retirement, between invocations. Not Unreal Destroy/GC.
 void retire(uint32_t identity){
  if(activeInvocations_)throw std::runtime_error("Receiver retirement during invocation is not implemented");
  if(!receivers_.count(identity))throw std::runtime_error("Receiver identity unavailable");
  retired_.insert(identity);structs_.retire(identity);receivers_.erase(identity);
 }
 // Changes only the dispatch selection between invocations. BeginState/EndState,
 // labels, latent actions, actor lifecycle and script-driven transitions pending.
 void selectState(uint32_t identity,uint32_t state){
  auto found=receivers_.find(identity);if(found==receivers_.end())throw std::runtime_error("Receiver identity unavailable");
  if(state){
   uint32_t childOwner=0;
   for(auto id:resolver_.stateLineage(state)){
    auto owner=states_.find(id);if(owner==states_.end())throw std::runtime_error("State owner binding unavailable");
    if(!resolver_.isSubclass(found->second.classId,owner->second))throw std::runtime_error("State belongs to unrelated receiver class");
    if(childOwner&&!resolver_.isSubclass(childOwner,owner->second))throw std::runtime_error("State parent belongs to unrelated owner class");
    childOwner=owner->second;
   }
  }
  found->second.stateId=state;
 }
 Scalar invoke(uint32_t identity,uint32_t function,const std::vector<Scalar>& arguments={},size_t budget=10000){
  auto found=receivers_.find(identity);if(found==receivers_.end())throw std::runtime_error("Receiver identity unavailable");
  struct InvocationGuard{size_t& active;explicit InvocationGuard(size_t& a):active(a){++active;}~InvocationGuard(){--active;}} invocationGuard{activeInvocations_};
  uint32_t current=identity;std::vector<uint32_t> contexts,frames;
  auto receiver=[&]() -> Receiver& {return receivers_.at(current);};
  auto validateObject=[&](ObjectRef object){if(object.identity&&!receivers_.count(object.identity))throw std::runtime_error("Object receiver identity unavailable");};
  TypedProgram::CallEnvironment environment;
  environment.validateObject=validateObject;
  environment.validateObjectClass=[&](ObjectRef object,uint32_t targetClass){
   resolver_.isSubclass(targetClass,targetClass);validateObject(object);
   if(object.identity&&!resolver_.isSubclass(receivers_.at(object.identity).classId,targetClass))throw std::runtime_error("Function object class constraint mismatch");
  };
  environment.pushFrame=[&](){frames.push_back(current);};environment.popFrame=[&](){frames.pop_back();};
  environment.self=[&](){return ObjectRef{frames.back()};};
  environment.enterContext=[&](ObjectRef object){validateObject(object);contexts.push_back(current);current=object.identity;};
  environment.leaveContext=[&](){current=contexts.back();contexts.pop_back();};
  environment.validate=[&](uint32_t id){
   auto binding=functions_.find(id);if(binding==functions_.end())throw std::runtime_error("Receiver function binding unavailable");
   if(!resolver_.isSubclass(receiver().classId,binding->second.owner))throw std::runtime_error("Function belongs to unrelated receiver class");
  };
  environment.resolveNamed=[&](uint32_t caller,int32_t name,bool global){
   const auto& names=functions_.at(caller).names;
   if(name<0||size_t(name)>=names.size())throw std::runtime_error("Receiver call name outside caller table");
   return resolver_.resolve(names[size_t(name)],receiver().classId,receiver().stateId,global);
  };
  environment.readProperty=[&](uint32_t caller,int32_t reference){Scalar value=*propertySlot(receiver(),caller,reference).second;if(auto object=std::get_if<ObjectRef>(&value)){validateObject(*object);}return value;};
  environment.readDefaultProperty=[&](uint32_t caller,int32_t reference){
   propertySlot(receiver(),caller,reference);
   Scalar value=defaultValue(receiver().classId,functions_.at(caller).properties.at(reference));
   if(auto object=std::get_if<ObjectRef>(&value)){validateObject(*object);}return value;
  };
  environment.writeProperty=[&](uint32_t caller,int32_t reference,const Scalar& value){
   auto slot=propertySlot(receiver(),caller,reference);
   if(auto object=std::get_if<ObjectRef>(&value)){
    validateObject(*object);
    if(object->identity&&slot.first->objectClass&&!resolver_.isSubclass(receivers_.at(object->identity).classId,slot.first->objectClass))throw std::runtime_error("Object property class constraint mismatch");
   }
   if(!matches(slot.first->type,value))throw std::runtime_error("Instance property type mismatch");
   *slot.second=value;
  };
  environment.readStruct=[&](uint32_t caller,int32_t reference,const std::vector<int32_t>& members){auto path=structPath(receiver(),caller,reference,members);return structs_.read(current,path.first,path.second);};
  environment.readDefaultStruct=[&](uint32_t caller,int32_t reference,const std::vector<int32_t>& members){auto path=structPath(receiver(),caller,reference,members);return structs_.readDefault(receiver().classId,path.first,path.second);};
  environment.writeStruct=[&](uint32_t caller,int32_t reference,const std::vector<int32_t>& members,const Scalar& value){auto path=structPath(receiver(),caller,reference,members);structs_.write(current,path.first,path.second,value);};
  return program_.invokeWithEnvironment(function,arguments,environment,budget);
 }
private:
 Scalar defaultValue(uint32_t receiverClass,uint32_t property)const{
  for(auto cls:resolver_.classLineage(receiverClass)){
   auto defaults=defaults_.find(cls);if(defaults==defaults_.end())continue;
   auto value=defaults->second.find(property);if(value!=defaults->second.end())return value->second;
  }
  return properties_.at(property).initial;
 }
 struct Binding{uint32_t owner;std::vector<std::string> names;std::map<int32_t,uint32_t> properties;};
 struct Property{uint32_t owner;ScalarType type;Scalar initial;uint32_t objectClass;};
 struct Receiver{uint32_t classId,stateId;std::map<uint32_t,Scalar> fields;};
 std::pair<const Property*,Scalar*> propertySlot(Receiver& receiver,uint32_t caller,int32_t reference){
  auto binding=functions_.find(caller);if(binding==functions_.end())throw std::runtime_error("Receiver function binding unavailable");
  auto link=binding->second.properties.find(reference);if(link==binding->second.properties.end())throw std::runtime_error("Unresolved instance property reference");
  const auto& schema=properties_.at(link->second);
  if(!resolver_.isSubclass(receiver.classId,schema.owner))throw std::runtime_error("Property belongs to unrelated receiver class");
  auto slot=receiver.fields.find(link->second);if(slot==receiver.fields.end())throw std::runtime_error("Instance property slot unavailable");
  return {&schema,&slot->second};
 }
 std::pair<uint32_t,MemberPath> structPath(const Receiver& receiver,uint32_t caller,int32_t reference,const std::vector<int32_t>& members)const{
  auto roots=structRoots_.find(caller);auto fields=structMembers_.find(caller);
  if(roots==structRoots_.end()||fields==structMembers_.end())throw std::runtime_error("Struct reference bindings unavailable");
  auto root=roots->second.find(reference);if(root==roots->second.end())throw std::runtime_error("Unresolved struct root reference");
  if(!resolver_.isSubclass(receiver.classId,structs_.owner(root->second)))throw std::runtime_error("Struct belongs to unrelated receiver class");
  MemberPath path;for(auto ref:members){auto member=fields->second.find(ref);if(member==fields->second.end())throw std::runtime_error("Unresolved struct member reference");path.push_back(member->second);}
  return {root->second,std::move(path)};
 }
 bool schemasFrozen_=false;size_t activeInvocations_=0;std::set<uint32_t> retired_;
 const TypedProgram& program_;const DispatchResolver& resolver_;
 StructProperties structs_;std::map<uint32_t,std::map<int32_t,uint32_t>> structRoots_,structMembers_;
 std::map<uint32_t,std::map<uint32_t,Scalar>> defaults_;std::map<uint32_t,Property> properties_;std::map<uint32_t,Binding> functions_;std::map<uint32_t,uint32_t> states_;std::map<uint32_t,Receiver> receivers_;
};
} // namespace hp3
