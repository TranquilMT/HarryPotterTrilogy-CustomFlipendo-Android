// Isolated metadata linker; no retail class deserialization or live objects. GPL-3.0.
#pragma once
#include "TypedProgram.h"
namespace hp3 {
struct DispatchScope {
 uint32_t id=0,parent=0;
 std::map<std::string,uint32_t> functions;
};
class DispatchResolver {
public:
 void addClass(DispatchScope scope){add(classes_,std::move(scope));}
 void addState(DispatchScope scope){add(states_,std::move(scope));}
 uint32_t resolve(const std::string& name,uint32_t receiverClass,uint32_t state=0,bool global=false)const{
  validate(classes_,receiverClass);if(state)validate(states_,state);
  return lookup(canonical(name),receiverClass,state,global);
 }
 bool isSubclass(uint32_t receiverClass,uint32_t baseClass)const{
  validate(classes_,receiverClass);validate(classes_,baseClass);
  while(receiverClass){if(receiverClass==baseClass)return true;receiverClass=classes_.at(receiverClass).parent;}
  return false;
 }
 std::vector<uint32_t> classLineage(uint32_t receiverClass)const{
  validate(classes_,receiverClass);std::vector<uint32_t> result;
  while(receiverClass){result.push_back(receiverClass);receiverClass=classes_.at(receiverClass).parent;}return result;
 }
 std::vector<uint32_t> stateLineage(uint32_t state)const{
  validate(states_,state);std::vector<uint32_t> result;
  while(state){result.push_back(state);state=states_.at(state).parent;}return result;
 }
 // The caller's package names are converted to canonical names before lookup.
 // A zero state means class dispatch only; global calls always bypass states.
 void link(FunctionImage& image,const std::vector<std::string>& names,uint32_t receiverClass,uint32_t state=0)const{
  validate(classes_,receiverClass);
  if(state)validate(states_,state);
  std::map<int32_t,uint32_t> virtualLinks,globalLinks;
  for(const auto& instruction:image.script.instructions){
   if(instruction.opcode!=0x1b&&instruction.opcode!=0x38)continue;
   size_t offset=size_t(instruction.offset)+1;
   if(offset>image.script.memory.size()||image.script.memory.size()-offset<4)throw std::runtime_error("Truncated named-call operand");
   uint32_t index=0;for(unsigned i=0;i<4;++i)index|=uint32_t(image.script.memory[offset+i])<<(8*i);
   if(index>=names.size())throw std::runtime_error("Named-call name outside caller table");
   const auto name=canonical(names[index]);uint32_t target=lookup(name,receiverClass,state,instruction.opcode==0x38);
   (instruction.opcode==0x1b?virtualLinks:globalLinks).emplace(int32_t(index),target);
  }
  // Commit only a complete resolution, preserving existing links on failure.
  image.virtualTargets=std::move(virtualLinks);image.globalTargets=std::move(globalLinks);
 }
private:
 using Scopes=std::map<uint32_t,DispatchScope>;
 uint32_t lookup(const std::string& name,uint32_t receiverClass,uint32_t state,bool global)const{
  uint32_t target=(!global&&state)?find(states_,state,name):0;
  if(!target)target=find(classes_,receiverClass,name);
  if(!target)throw std::runtime_error("Unresolved metadata function: "+name);
  return target;
 }
 static std::string canonical(std::string name){
  if(name.empty())throw std::runtime_error("Empty dispatch name");
  for(auto& c:name){unsigned char b=static_cast<unsigned char>(c);if(b>=128)throw std::runtime_error("Non-ASCII dispatch name requires verified folding");if(c>='A'&&c<='Z')c=char(c-'A'+'a');}
  return name;
 }
 static void add(Scopes& scopes,DispatchScope scope){
  if(!scope.id||scopes.count(scope.id))throw std::runtime_error("Duplicate or zero dispatch scope");
  std::map<std::string,uint32_t> normalized;
  for(const auto& fn:scope.functions){if(!fn.second||!normalized.emplace(canonical(fn.first),fn.second).second)throw std::runtime_error("Duplicate or zero dispatch function");}
  scope.functions=std::move(normalized);scopes.emplace(scope.id,std::move(scope));
 }
 static void validate(const Scopes& scopes,uint32_t id){
  if(!id)throw std::runtime_error("Missing receiver class");
  std::set<uint32_t> seen;
  while(id){if(!seen.insert(id).second)throw std::runtime_error("Cyclic dispatch inheritance");auto it=scopes.find(id);if(it==scopes.end())throw std::runtime_error("Missing dispatch parent");id=it->second.parent;}
 }
 static uint32_t find(const Scopes& scopes,uint32_t id,const std::string& name){
  while(id){const auto& scope=scopes.at(id);auto it=scope.functions.find(name);if(it!=scope.functions.end())return it->second;id=scope.parent;}
  return 0;
 }
 Scopes classes_,states_;
};
} // namespace hp3
