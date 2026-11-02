// Explicit scalar leaves of instance structs; no native memory layout. GPL-3.0.
#pragma once
#include "DispatchResolver.h"
namespace hp3 {
using MemberPath=std::vector<uint32_t>;
struct StructLeaf {MemberPath path;ScalarType type=ScalarType::Void;Scalar initial;};
class StructProperties {
public:
 explicit StructProperties(const DispatchResolver& resolver):resolver_(resolver){}
 void define(uint32_t root,uint32_t owner,const std::vector<StructLeaf>& leaves){
  if(schemasFrozen_)throw std::runtime_error("Struct schemas must precede receiver creation");
  resolver_.isSubclass(owner,owner);
  if(!root||schemas_.count(root)||leaves.empty())throw std::runtime_error("Invalid or duplicate struct schema");
  Schema schema;schema.owner=owner;
  for(const auto& leaf:leaves){
   if(leaf.path.empty()||leaf.path.size()>32||(leaf.type==ScalarType::Void||leaf.type>ScalarType::Byte)||!matches(leaf.type,leaf.initial))throw std::runtime_error("Invalid struct leaf schema");
   for(auto field:leaf.path)if(!field)throw std::runtime_error("Zero struct member identity");
   for(const auto& previous:schema.leaves){size_t n=std::min(previous.first.size(),leaf.path.size());if(std::equal(leaf.path.begin(),leaf.path.begin()+n,previous.first.begin()))throw std::runtime_error("Overlapping struct leaf paths");}
   schema.leaves.emplace(leaf.path,leaf);
  }
  schemas_.emplace(root,std::move(schema));
 }
 bool contains(uint32_t root)const{return schemas_.count(root)!=0;}
 uint32_t owner(uint32_t root)const{auto found=schemas_.find(root);if(found==schemas_.end())throw std::runtime_error("Struct schema unavailable");return found->second.owner;}
 void bindDefault(uint32_t receiverClass,uint32_t root,const MemberPath& path,const Scalar& value){
  if(schemasFrozen_)throw std::runtime_error("Struct defaults must precede receiver creation");
  if(!resolver_.isSubclass(receiverClass,owner(root)))throw std::runtime_error("Struct default belongs to unrelated class");
  const auto& leaves=schemas_.at(root).leaves;auto leaf=leaves.find(path);if(leaf==leaves.end())throw std::runtime_error("Struct leaf unavailable");
  if(!matches(leaf->second.type,value))throw std::runtime_error("Struct default type mismatch");
  auto& fields=defaults_[receiverClass][root];if(!fields.emplace(path,value).second)throw std::runtime_error("Duplicate struct default");
 }
 Scalar readDefault(uint32_t receiverClass,uint32_t root,const MemberPath& path)const{
  if(!resolver_.isSubclass(receiverClass,owner(root)))throw std::runtime_error("Struct default belongs to unrelated class");
  const auto& leaves=schemas_.at(root).leaves;auto leaf=leaves.find(path);if(leaf==leaves.end())throw std::runtime_error("Struct leaf unavailable");
  for(auto cls:resolver_.classLineage(receiverClass)){auto defaults=defaults_.find(cls);if(defaults==defaults_.end())continue;auto fields=defaults->second.find(root);if(fields==defaults->second.end())continue;auto value=fields->second.find(path);if(value!=fields->second.end())return value->second;}
  return leaf->second.initial;
 }
 void create(uint32_t identity,uint32_t receiverClass){
  std::map<uint32_t,std::map<MemberPath,Scalar>> fields;
  for(const auto& schema:schemas_)if(resolver_.isSubclass(receiverClass,schema.second.owner))for(const auto& leaf:schema.second.leaves)fields[schema.first].emplace(leaf.first,readDefault(receiverClass,schema.first,leaf.first));
  if(!instances_.emplace(identity,std::move(fields)).second)throw std::runtime_error("Duplicate struct receiver");
  schemasFrozen_=true;
 }
 void retire(uint32_t identity){if(!instances_.erase(identity))throw std::runtime_error("Struct receiver unavailable");}
 Scalar read(uint32_t identity,uint32_t root,const MemberPath& path)const{
  const auto& fields=instance(identity,root);auto value=fields.find(path);if(value==fields.end())throw std::runtime_error("Struct leaf unavailable");return value->second;
 }
 void write(uint32_t identity,uint32_t root,const MemberPath& path,const Scalar& value){
  auto definition=schemas_.find(root);if(definition==schemas_.end())throw std::runtime_error("Struct schema unavailable");
  const auto& schema=definition->second;auto leaf=schema.leaves.find(path);if(leaf==schema.leaves.end())throw std::runtime_error("Struct leaf unavailable");
  if(!matches(leaf->second.type,value))throw std::runtime_error("Struct leaf type mismatch");
  instance(identity,root).at(path)=value;
 }
private:
 const std::map<MemberPath,Scalar>& instance(uint32_t identity,uint32_t root)const{
  auto receiver=instances_.find(identity);if(receiver==instances_.end())throw std::runtime_error("Struct receiver unavailable");auto fields=receiver->second.find(root);if(fields==receiver->second.end())throw std::runtime_error("Struct root unavailable on receiver");return fields->second;
 }
 std::map<MemberPath,Scalar>& instance(uint32_t identity,uint32_t root){return const_cast<std::map<MemberPath,Scalar>&>(static_cast<const StructProperties*>(this)->instance(identity,root));}
 struct Schema{uint32_t owner=0;std::map<MemberPath,StructLeaf> leaves;};
 bool schemasFrozen_=false;
 std::map<uint32_t,std::map<uint32_t,std::map<MemberPath,Scalar>>> defaults_;
 const DispatchResolver& resolver_;std::map<uint32_t,Schema> schemas_;std::map<uint32_t,std::map<uint32_t,std::map<MemberPath,Scalar>>> instances_;
};
} // namespace hp3
