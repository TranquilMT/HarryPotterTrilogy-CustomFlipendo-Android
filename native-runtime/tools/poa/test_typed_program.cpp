#include "../../src/compat/ue2/TypedProgram.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <functional>
using namespace hp3;
static void require(bool x){if(!x)throw std::runtime_error("Typed-program test mismatch");}
static std::vector<uint8_t> blob(Reader& r){uint32_t n=r.dword();require(n<64*1024*1024);std::vector<uint8_t> out;for(uint32_t i=0;i<n;++i)out.push_back(r.byte());return out;}
static Slot slot(Reader& r){Slot s;s.reference=int32_t(r.dword());s.type=ScalarType(r.dword());s.optional=r.dword()!=0;s.flags=r.dword();return s;}
static void fails(std::function<void()> f,const std::string& reason){bool rejected=false;try{f();}catch(const std::runtime_error& e){rejected=std::string(e.what()).find(reason)!=std::string::npos;}require(rejected);}
static Scalar example(ScalarType type){switch(type){case ScalarType::Bool:return true;case ScalarType::Int:return int32_t(7);case ScalarType::Byte:return int32_t(3);case ScalarType::Float:return 2.5f;default:return std::monostate{};}}
static bool same(const Scalar& a,const Scalar& b){if(a.index()!=b.index())return false;if(auto x=std::get_if<float>(&a)){float y=std::get<float>(b);uint32_t p,q;std::memcpy(&p,x,4);std::memcpy(&q,&y,4);return p==q;}return a==b;}
static FunctionImage synthetic(uint32_t id,std::vector<uint8_t> bytes,uint32_t memory,ScalarType result){
 Reader r(bytes);std::vector<std::string> names={"None"};BytecodeReader decoder(r,names,1,100,memory);FunctionImage f;f.id=id;f.qualifiedName="Test.Function";f.script=decoder.read();if(result!=ScalarType::Void)f.result={100, result,false,0x580};return f;
}
int main(int argc,char** argv){try{
 require(argc==2||argc==3);std::ofstream report;if(argc==3){report.open(argv[2]);require(bool(report));}std::ifstream file(argv[1],std::ios::binary);require(bool(file));std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),{});Reader input(data);TypedProgram program;std::vector<FunctionImage> images;std::map<std::string,uint32_t> byName;
 uint32_t groups=input.dword();require(groups<10000);
 for(uint32_t g=0;g<groups;++g){
  uint32_t n=input.dword();require(n<1000000);std::vector<std::string> names;
  for(uint32_t i=0;i<n;++i){auto b=blob(input);names.emplace_back(b.begin(),b.end());}
  uint32_t imports=input.dword(),exports=input.dword(),count=input.dword();require(count<1000000);
  for(uint32_t i=0;i<count;++i){
   FunctionImage f;f.id=input.dword();auto name=blob(input);f.qualifiedName.assign(name.begin(),name.end());auto raw=blob(input);f.intrinsic=Intrinsic(input.dword());Reader source(raw);auto h=readFunctionHeader(source,names);BytecodeReader decoder(source,names,imports,exports,h.bytecodeMemoryBytes);f.script=decoder.read();f.nativeIndex=source.word();source.byte();uint32_t flags=source.dword();if(flags&0x40)source.word();require(!source.remaining());f.native=(flags&0x400)!=0;
   uint32_t args=input.dword();require(args<10000);for(uint32_t j=0;j<args;++j)f.arguments.push_back(slot(input));uint32_t locals=input.dword();require(locals<10000);for(uint32_t j=0;j<locals;++j)f.locals.push_back(slot(input));f.result=slot(input);
   uint32_t links=input.dword();require(links<100000);for(uint32_t j=0;j<links;++j){int32_t ref=int32_t(input.dword());f.finalTargets[ref]=input.dword();}
   byName[f.qualifiedName]=f.id;program.add(f);images.push_back(std::move(f));
  }
 }
 require(!input.remaining());size_t checkedConstants=0,checkedIdentity=0,checkedVoid=0,checkedDefaults=0,smokeFinished=0,smokeRejected=0;std::map<std::string,size_t> rejections;
 for(const auto& f:images){
  if(f.native)continue;
  std::vector<Scalar> args;for(const auto& s:f.arguments)args.push_back(example(s.type));
  const auto& b=f.script.memory;bool oracle=false;Scalar expected;
  if(b.size()>1&&b[0]==4){
   uint8_t op=b[1];Reader r(b);r.byte();r.byte();
   if(op==0x0b){oracle=true;expected=zero(f.result.type);if(f.result.type==ScalarType::Void)++checkedVoid;else ++checkedDefaults;}
   else if(op==0x27||op==0x28){expected=op==0x27;oracle=matches(f.result.type,expected);if(oracle)++checkedConstants;}
   else if(op==0x25||op==0x26||op==0x24||op==0x2c||op==0x1d||op==0x1e){
    if(op==0x1e){uint32_t bits=r.dword();float v;std::memcpy(&v,&bits,4);expected=v;}
    else expected=op==0x25?0:(op==0x26?1:(op==0x1d?int32_t(r.dword()):int32_t(r.byte())));
    oracle=matches(f.result.type,expected);if(oracle)++checkedConstants;
   }else if(op==0){
    int32_t ref=int32_t(r.dword());for(size_t i=0;i<f.arguments.size();++i)if(f.arguments[i].reference==ref){expected=args[i];oracle=matches(f.result.type,expected);if(oracle)++checkedIdentity;}
   }
  }
  if(oracle)require(same(program.invoke(f.id,args,2000),expected));
  try{program.invoke(f.id,args,2000);++smokeFinished;if(report)report<<f.qualifiedName<<"\tfinished\t"<<(oracle?"oracle-checked":"smoke-only")<<"\n";}catch(const std::runtime_error& e){++smokeRejected;++rejections[e.what()];if(report)report<<f.qualifiedName<<"\trejected\t"<<e.what()<<"\n";}
 }
 auto native=[&](const char* name,std::vector<Scalar> args,Scalar expected){require(same(program.invoke(byName.at(name),args),expected));};
 native("Core.Object.Add_IntInt",{int32_t(7),int32_t(5)},int32_t(12));native("Core.Object.Add_IntInt",{int32_t(2147483647),int32_t(1)},int32_t(-2147483647-1));native("Core.Object.Subtract_IntInt",{int32_t(5),int32_t(7)},int32_t(-2));native("Core.Object.Multiply_IntInt",{int32_t(7),int32_t(5)},int32_t(35));native("Core.Object.Less_IntInt",{int32_t(5),int32_t(7)},true);native("Core.Object.Greater_IntInt",{int32_t(5),int32_t(7)},false);native("Core.Object.EqualEqual_IntInt",{int32_t(7),int32_t(7)},true);native("Core.Object.NotEqual_IntInt",{int32_t(5),int32_t(7)},true);native("Core.Object.Not_PreBool",{true},false);native("Core.Object.Add_FloatFloat",{2.5f,1.5f},4.0f);native("Core.Object.Subtract_FloatFloat",{2.5f,1.5f},1.0f);native("Core.Object.Multiply_FloatFloat",{2.5f,1.5f},3.75f);
 auto identity=synthetic(900001,{4,0,5},6,ScalarType::Int);identity.arguments.push_back({5,ScalarType::Int,false,0x80});program.add(identity);
 auto caller=synthetic(900002,{4,0x1c,1,0x2c,7,0x16},9,ScalarType::Int);caller.finalTargets[1]=identity.id;program.add(caller);require(std::get<int32_t>(program.invoke(caller.id))==7);
 auto imported=synthetic(900003,{4,0x1c,0x81,0x2c,9,0x16},9,ScalarType::Int);imported.finalTargets[-1]=identity.id;program.add(imported);require(std::get<int32_t>(program.invoke(imported.id))==9);
 auto math=synthetic(900004,{4,0x92,0x2c,7,0x2c,5,0x16},7,ScalarType::Int);program.add(math);require(std::get<int32_t>(program.invoke(math.id))==12);
 auto recursive=synthetic(900005,{4,0x1c,1,0x16},7,ScalarType::Void);recursive.finalTargets[1]=recursive.id;program.add(recursive);fails([&]{program.invoke(recursive.id,{},10000);},"recursion");fails([&]{program.invoke(recursive.id,{},10);},"budget");
 auto optional=identity;optional.id=900006;optional.arguments[0].optional=true;optional.arguments[0].flags=0x90;program.add(optional);require(std::get<int32_t>(program.invoke(optional.id))==0);require(std::get<int32_t>(program.invoke(optional.id,{std::monostate{}}))==0);
 auto local=synthetic(900007,{4,0,5},6,ScalarType::Int);local.locals.push_back({5,ScalarType::Int,false,0});program.add(local);require(std::get<int32_t>(program.invoke(local.id))==0);
 auto byte=identity;byte.id=900008;byte.arguments[0].type=ScalarType::Byte;byte.result.type=ScalarType::Byte;program.add(byte);fails([&]{program.invoke(byte.id,{int32_t(256)});},"type mismatch");
 auto wrongReturn=synthetic(900009,{4,0x26},2,ScalarType::Bool);program.add(wrongReturn);fails([&]{program.invoke(wrongReturn.id);},"return type");
 auto missing=synthetic(900010,{4,0x1c,1,0x16},7,ScalarType::Int);missing.finalTargets[1]=999999;program.add(missing);fails([&]{program.invoke(missing.id);},"implementation unavailable");
 auto unknownNative=synthetic(900011,{4,0xc3,0x16},3,ScalarType::Float);program.add(unknownNative);fails([&]{program.invoke(unknownNative.id);},"not implemented");
 fails([&]{program.invoke(identity.id);},"required");fails([&]{program.invoke(identity.id,{true});},"type mismatch");fails([&]{program.invoke(identity.id,{int32_t(1),int32_t(2)});},"Too many");
 auto defaultBool=synthetic(900013,{4,0x0b},2,ScalarType::Bool);program.add(defaultBool);require(std::get<bool>(program.invoke(defaultBool.id))==false);
 auto out=identity;out.id=900012;out.arguments[0].flags|=0x100;fails([&]{program.add(out);},"Out or lazy");auto duplicate=identity;fails([&]{program.add(duplicate);},"Duplicate");
 std::cout<<"Registered "<<images.size()<<" typed images; checked "<<checkedConstants<<" constant results, "<<checkedIdentity<<" argument identities, "<<checkedVoid<<" original void returns, "<<checkedDefaults<<" declared default returns\n";
 std::cout<<"Bounded smoke probe: "<<smokeFinished<<" finished, "<<smokeRejected<<" rejected (not correctness or gameplay coverage)\n";
 for(const auto& r:rejections)std::cout<<r.first<<": "<<r.second<<"\n";
 std::cout<<"PASS typed frames, required/optional arguments, final/import calls, 11 Core intrinsics, recursion/shared budgets and failures\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}
