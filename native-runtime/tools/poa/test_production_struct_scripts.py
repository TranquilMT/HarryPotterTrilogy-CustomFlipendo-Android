"""Production function-header helpers and actual UStruct token decoder tests.

Surrounding engine identity/file interfaces are stubs. This is not a complete
UClass/UStruct object load, native binding, VM evaluation or Android scene test.
"""
import os
import pathlib
import subprocess
import tempfile
import unittest


class ProductionStructScripts(unittest.TestCase):
    def test_function_header_and_actual_token_reader(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        source = (root / 'src/engine/SurrealEngine/Packages/Core/UStruct.cpp').read_text()
        header = (root / 'src/engine/SurrealEngine/Packages/Core/UStruct.h').read_text()
        self.assertIn('StructScripts::Read(*stream, poaStruct,', source)
        self.assertIn('StructScripts::Write(*stream, poaStruct', source)
        self.assertIn('const bool poaStruct = engine->LaunchInfo.IsHarryPotter3();', source)
        tokens = source[source.index('ExprToken UStruct::ReadToken('):]
        enum = header[header.index('enum class ExprToken : uint8_t\n{'):]
        harness = r'''
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <iostream>
#include "Package/StructScriptHeader.h"
#include "Package/PoaPrimitiveCast.h"
#include "Package/CompactIndex.h"
#include "compat/ue2/BytecodeReader.h"
struct NameString {std::string ToString()const{return "SyntheticFunction";}};
struct Exception {[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct LaunchInfo {bool poa=false,hp1=false;bool IsHarryPotter3()const{return poa;}bool IsHarryPotter2()const{return false;}bool IsHarryPotter1()const{return hp1;}bool IsUnreal1_227()const{return false;}};
struct Engine {struct LaunchInfo LaunchInfo;};Engine instance;Engine* engine=&instance;
namespace HP2 {constexpr uint8_t DebugInfoToken=0xff;uint8_t ToStockToken(uint8_t t){return t;}}
struct Package {std::string GetName(int n){if(n==0)return "None";if(n==1)return "Function";throw std::runtime_error("bad name");}};
struct ObjectStream {
 std::vector<uint8_t> data;size_t pos=0;int version=129;Package package;
 uint8_t ReadUInt8(){if(pos==data.size())throw std::runtime_error("truncated");return data[pos++];}
 uint16_t ReadUInt16(){uint16_t lo=ReadUInt8();return lo|(uint16_t(ReadUInt8())<<8);}
 uint32_t ReadUInt32(){uint32_t lo=ReadUInt16();return lo|(uint32_t(ReadUInt16())<<16);}
 float ReadFloat(){uint32_t bits=ReadUInt32();float f;memcpy(&f,&bits,4);return f;}
 int32_t ReadIndex(){return PackageIndices::Read([&]{return ReadUInt8();});}
 std::string ReadAsciiZ(){std::string s;for(uint8_t c;(c=ReadUInt8())!=0;)s+=char(c);return s;}
 std::wstring ReadUnicodeZ(){std::wstring s;for(uint16_t c;(c=ReadUInt16())!=0;)s+=wchar_t(c);return s;}
 void WriteUInt8(uint8_t b){data.push_back(b);}void WriteUInt32(uint32_t n){for(int i=0;i<4;++i)data.push_back(uint8_t(n>>(8*i)));}
 int GetVersion(){return version;}Package*GetPackage(){return &package;}
};
'''
        harness += enum + r'''
struct UStruct {
 NameString Name;std::vector<uint8_t>Bytecode;
 ExprToken ReadToken(ObjectStream*,int);
 void PushBytes(const void*,size_t);void PushUInt8(uint8_t);void PushUInt16(uint16_t);void PushUInt32(uint32_t);
 void PushIndex(int32_t);void PushFloat(float);void PushAsciiZ(const std::string&);void PushUnicodeZ(const std::wstring&);
};
'''
        harness += tokens + r'''
int checks=0;
auto readHeader(ObjectStream& s,bool poa,int32_t* cpp=nullptr){
 return StructScripts::Read(s,poa,[&]{const auto ref=s.ReadIndex();if(cpp)*cpp=ref;});
}
void writeHeader(ObjectStream& s,bool poa,StructScripts::Header h){
 StructScripts::Write(s,poa,h,[&]{s.WriteUInt8(0);});
}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}assert(failed);++checks;}
void same(const std::vector<uint8_t>&raw,uint32_t memory){ObjectStream stream;stream.data=raw;UStruct function;function.ReadToken(&stream,0);assert(stream.pos==raw.size());hp3::Reader reader(raw);const std::vector<std::string> names={"None","Function"};hp3::BytecodeReader decoder(reader,names,1,1,memory);auto expected=decoder.read();assert(function.Bytecode==expected.memory);++checks;}
int main(){
 StructScripts::Header values{123,456,3};ObjectStream encoded;writeHeader(encoded,true,values);assert(encoded.data.size()==13);++checks;
 std::vector<uint8_t> original={0,0,0,0,0,1};original.insert(original.end(),encoded.data.begin(),encoded.data.end());
 hp3::Reader oracle(original);auto expected=hp3::readFunctionHeader(oracle,{"None","Function"});
 encoded.data.push_back(0xaa);auto actual=readHeader(encoded,true);assert(actual.line==expected.line&&actual.textPosition==expected.textPosition&&actual.memoryBytes==expected.bytecodeMemoryBytes&&encoded.pos+6==oracle.position()&&encoded.ReadUInt8()==0xaa);++checks;
 ObjectStream legacy;legacy.version=69;writeHeader(legacy,false,values);assert(legacy.data.size()==12);auto old=readHeader(legacy,false);assert(old.line==123&&old.textPosition==456&&old.memoryBytes==3);++checks;
 for(size_t n=0;n<13;++n){ObjectStream shortHeader;shortHeader.data.assign(encoded.data.begin(),encoded.data.begin()+n);rejects([&]{readHeader(shortHeader,true);});}
 ObjectStream unknown;unknown.version=128;rejects([&]{readHeader(unknown,true);});rejects([&]{writeHeader(unknown,true,values);});
 ObjectStream cppText;cppText.data={0x40,0x21};
 cppText.data.insert(cppText.data.end(),encoded.data.begin()+1,encoded.data.end());
 int32_t cppRef=0;auto cppHeader=readHeader(cppText,true,&cppRef);
 assert(cppRef==2112&&cppHeader.line==123&&cppText.ReadUInt8()==0xaa);++checks;
 ObjectStream malformed;malformed.data={0x40,0x80,0x80,0x80,0xff};rejects([&]{readHeader(malformed,true);});
 ObjectStream huge;huge.WriteUInt8(0);huge.WriteUInt32(0);huge.WriteUInt32(0);huge.WriteUInt32(StructScripts::MaximumBytes+1);rejects([&]{readHeader(huge,true);});
 ObjectStream output;rejects([&]{writeHeader(output,true,{0,0,StructScripts::MaximumBytes+1});});assert(output.data.empty());
 engine->LaunchInfo.poa=true;
 same({0x60,0x01,0x16},3); // extended native 1, not UE1's conversion token 0x60
 same({0x61,0x23,0x25,0x16},4);same({0x6f,0xfe,0x16},3);same({0x70,0x25,0x16},3);
 same({0x1b,1,0x16},6);same({0x1c,1,0x16},6);same({0x29,1},5);
 same({0x34,0x41,0,0xe9,0,0,0},7); // UTF-16 units, independent of 32-bit wchar_t
 same({0x34,0x3d,0xd8,0x00,0xde,0,0},7);
 for(int code=0x3a;code<=0x45;++code) same({0x39,uint8_t(code),0x25},3);
 same({0x39,0x3f,0x39,0x41,0x27},5); // nested casts retain both prefix bytes
 for(int op=0x39;op<0x60;++op){ObjectStream stream;stream.data={uint8_t(op),0x25};UStruct f;rejects([&]{f.ReadToken(&stream,0);});}
 ObjectStream truncated;truncated.data={0x60};UStruct f;rejects([&]{f.ReadToken(&truncated,0);});
 ObjectStream depth;depth.data={0x25};rejects([&]{f.ReadToken(&depth,64);});
 engine->LaunchInfo.poa=false;ObjectStream conversion;conversion.version=69;conversion.data={0x39,0x25};UStruct ue1;ue1.ReadToken(&conversion,0);assert(ue1.Bytecode==conversion.data);++checks;
 engine->LaunchInfo.hp1=true;ObjectStream hp1;hp1.version=67;hp1.data={0x37,0x25};UStruct length;length.ReadToken(&hp1,0);assert(length.Bytecode==hp1.data);++checks;
 UStruct cap;cap.Bytecode.resize(StructScripts::MaximumBytes);rejects([&]{cap.PushUInt8(0);});rejects([&]{cap.PushUInt32(0);});
 std::cout<<"PASS "<<checks<<" production function-header/token cases\n";
}
'''
        with tempfile.TemporaryDirectory(prefix='poa-struct-scripts-') as tmp:
            tmp = pathlib.Path(tmp)
            cpp, binary = tmp / 'test.cpp', tmp / 'test'
            cpp.write_text(harness)
            subprocess.run(['g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                            '-I', str(root / 'src/engine/SurrealEngine'), '-I', str(root / 'src'),
                            str(cpp), '-o', str(binary)], check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


if __name__ == '__main__':
    unittest.main()
