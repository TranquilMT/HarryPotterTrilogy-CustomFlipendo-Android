"""Compile actual production table/stream methods against synthetic package files.

No SDL, game data, UObject deserialization or Android playback is implied.
"""
import os
import pathlib
import subprocess
import tempfile
import unittest


class ProductionPackageTables(unittest.TestCase):
    def test_actual_table_and_stream_loading(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        package = (root / 'src/engine/SurrealEngine/Package/Package.cpp').read_text()
        methods = package[package.index('void Package::ReadTables()'):package.index('std::string Package::GetExportName(')]
        stream = (root / 'src/engine/SurrealEngine/Package/PackageStream.cpp').read_text()
        stream = stream[stream.index('PackageStream::PackageStream('):stream.index('/////////////////////////////////////////////////////////////////////////////')]
        harness = r'''
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <climits>
#include <iostream>
#include "CompactIndex.h"
#include "SerializedString.h"
#include "PackageValidation.h"
template<class T> using Array=std::vector<T>;
struct NameString {std::string text;NameString(){}NameString(std::string s):text(s){}NameString(const char*s):text(s){}
 NameString& operator=(const std::string&s){text=s;return *this;}std::string ToString()const{return text;}
 bool operator<(const NameString&o)const{return text<o.text;}};
enum class PackageFlags:uint32_t{};enum class ObjectFlags:uint32_t{};
#include "PackageStream.h"
struct NameTableEntry {NameString Name;uint32_t Flags;};
struct ExportTableEntry {int32_t ObjClass,ObjBase,ObjOuter,ObjName;ObjectFlags ObjFlags;int32_t ObjSize,ObjOffset;};
struct ImportTableEntry {int32_t ClassPackage,ClassName,ObjOuter,ObjName;};
struct UClass{};
struct Exception {[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct File {
 std::vector<uint8_t> bytes;uint64_t pos=0;unsigned sizeCalls=0,reads=0;
 int64_t size(){++sizeCalls;return int64_t(bytes.size());}uint64_t tell(){return pos;}
 void seek(uint64_t p){pos=p;}
 void read(void*out,size_t n){++reads;if(pos>bytes.size()||n>bytes.size()-pos)throw std::runtime_error("Truncated read");memcpy(out,bytes.data()+pos,n);pos+=n;}
};
struct ObjectStream {std::unique_ptr<uint64_t[]> data;size_t bytes;
 ObjectStream(Package*,std::unique_ptr<uint64_t[]> d,size_t,size_t n,const NameString&,UClass*):data(std::move(d)),bytes(n){}};
struct PackageManager {std::shared_ptr<File> file=std::make_shared<File>();std::shared_ptr<PackageStream> GetStream(Package*p){return std::make_shared<PackageStream>(p,file);}};
struct Package {
 PackageManager* Packages;NameString Name{"Synthetic"};int Version=0,LicenseeMode=0;PackageFlags Flags{};uint8_t Guid[16]{};
 std::vector<NameTableEntry>NameTable;std::map<NameString,int>NameHash;std::vector<ExportTableEntry>ExportTable;std::vector<ImportTableEntry>ImportTable;
 Package(PackageManager&p):Packages(&p){}int GetVersion()const{return Version;}NameString GetPackageName(){return Name;}
 void ReadTables();std::unique_ptr<ObjectStream>OpenObjectStream(int,const NameString&,UClass*);
};
void LogMessage(const std::string&){}
'''
        harness += stream + methods + r'''
using Bytes=std::vector<uint8_t>;
void dword(Bytes& b,uint32_t n){for(int i=0;i<4;++i)b.push_back(uint8_t(n>>(8*i)));}
void patch(Bytes&b,size_t offset,uint32_t n){for(int i=0;i<4;++i)b.at(offset+i)=uint8_t(n>>(8*i));}
void compact(Bytes&b,int32_t n){uint32_t value=n<0?uint32_t(-int64_t(n)):uint32_t(n);uint8_t first=(n<0?128:0)|(value&63);value>>=6;if(value)first|=64;b.push_back(first);while(value){uint8_t next=value&127;value>>=7;if(value)next|=128;b.push_back(next);}}
struct Fixture {Bytes data;uint32_t names,exports,imports,payload;};
Fixture fixture(int version=129,bool wide=false,bool empty=false){
 Bytes names;for(const std::string& n:{std::string("Object"),std::string("Core"),std::string("Class")}){
  compact(names,wide?-int32_t(n.size()+1):int32_t(n.size()+1));for(char ch:n){names.push_back(ch);if(wide)names.push_back(0);}names.push_back(0);if(wide)names.push_back(0);dword(names,0);
 }
 Bytes imports;compact(imports,1);compact(imports,2);dword(imports,0);compact(imports,1);
 uint32_t header=version<68?44:64;Bytes exports;uint32_t payload=0;
 for(int pass=0;pass<3;++pass){exports.clear();compact(exports,0);compact(exports,0);dword(exports,0);compact(exports,0);dword(exports,0);compact(exports,empty?0:3);if(!empty)compact(exports,payload);payload=header+names.size()+exports.size()+imports.size();}
 Fixture f;f.names=header;f.exports=header+names.size();f.imports=f.exports+exports.size();f.payload=payload;
 dword(f.data,0x9e2a83c1);dword(f.data,version);dword(f.data,0);dword(f.data,3);dword(f.data,f.names);dword(f.data,1);dword(f.data,f.exports);dword(f.data,1);dword(f.data,f.imports);
 if(version<68){dword(f.data,0);dword(f.data,0);}else{f.data.resize(52,0);dword(f.data,1);dword(f.data,1);dword(f.data,3);}
 f.data.insert(f.data.end(),names.begin(),names.end());f.data.insert(f.data.end(),exports.begin(),exports.end());f.data.insert(f.data.end(),imports.begin(),imports.end());if(!empty){f.data.push_back(11);f.data.push_back(22);f.data.push_back(33);}return f;
}
int checks=0;
void valid(const Fixture&f,int version){PackageManager m;m.file->bytes=f.data;Package p(m);p.ReadTables();assert(p.Version==version&&p.NameTable.size()==3&&p.NameTable[0].Name.text=="Object"&&p.ImportTable.size()==1&&p.ExportTable.size()==1);auto stream=p.OpenObjectStream(0,"Object",nullptr);assert(stream->bytes==3&&reinterpret_cast<uint8_t*>(stream->data.get())[2]==33);++checks;}
template<class F>void rejects(F action){bool failed=false;try{action();}catch(const std::runtime_error&){failed=true;}assert(failed);++checks;}
void invalid(const Bytes&data){rejects([&]{PackageManager m;m.file->bytes=data;Package p(m);p.ReadTables();});}
int main(){
 valid(fixture(),129);valid(fixture(129,true),129);valid(fixture(67),67);valid(fixture(69),69);
 auto f=fixture();Bytes b;
 b=f.data;patch(b,12,0xffffffff);invalid(b);
 b=f.data;patch(b,12,1000001);invalid(b);
 b=f.data;patch(b,16,8);invalid(b);
 b=f.data;patch(b,24,b.size()+1);invalid(b);
 b=f.data;patch(b,32,0xffffffff);invalid(b);
 b=f.data;patch(b,32,f.exports);invalid(b); // individually decodable tables must not overlap
 b=f.data;patch(b,52,0xffffffff);invalid(b);
 b=f.data;patch(b,52,1000000);invalid(b);
 b=f.data;patch(b,56,0xffffffff);invalid(b);
 b=f.data;b[f.exports]=2;invalid(b); // local class reference outside exports
 b=f.data;b[f.exports+6]=3;invalid(b); // name outside table
 b=f.data;b[f.imports]=3;invalid(b); // import package name outside table
 b=f.data;patch(b,f.exports+2,1);invalid(b); // export self outer
 b=f.data;patch(b,f.imports+2,uint32_t(-1));invalid(b); // import self outer
 b=f.data;patch(b,f.exports+2,uint32_t(-1));patch(b,f.imports+2,1);invalid(b); // mixed cycle
 b=f.data;b[f.exports+11]=0x81;invalid(b); // negative serialized size
 b=f.data;b.pop_back();invalid(b); // out-of-file payload
 b=f.data;b[f.exports+12]=64; b[f.exports+13]=1;invalid(b); // payload overlaps name table
 for(size_t n=0;n<64;++n){b=f.data;b.resize(n);invalid(b);}
 // Empty tables may have zero offsets; zero-size exports do not allocate payloads.
 PackageManager m;auto e=fixture(129,false,true);m.file->bytes=e.data;Package p(m);p.ReadTables();auto retained=m.file;m.file.reset();assert(p.OpenObjectStream(0,"Object",nullptr)->bytes==0);m.file=retained;++checks;
 p.ExportTable[0].ObjSize=INT_MAX;p.ExportTable[0].ObjOffset=64;rejects([&]{p.OpenObjectStream(0,"Object",nullptr);});
 p.ExportTable[0].ObjSize=-1;m.file.reset();rejects([&]{p.OpenObjectStream(0,"Object",nullptr);});m.file=retained;
 rejects([&]{p.OpenObjectStream(-1,"Object",nullptr);});rejects([&]{p.OpenObjectStream(1,"Object",nullptr);});
 m.file->bytes.resize(56);std::fill(m.file->bytes.begin(),m.file->bytes.end(),0);patch(m.file->bytes,0,0x9e2a83c1);patch(m.file->bytes,4,129);Package empty(m);empty.ReadTables();assert(empty.NameTable.empty());++checks;
 auto raw=std::make_shared<File>();raw->bytes={1,2,3};PackageStream s(&empty,raw);assert(s.ReadUInt8()==1);s.Seek(3);s.Skip(0);rejects([&]{s.ReadUInt8();});rejects([&]{s.Seek(4);});rejects([&]{s.Skip(UINT32_MAX);});assert(raw->sizeCalls==1);++checks;
 rejects([]{PackageValidation::Range(UINT64_MAX,2,100,"overflow");});rejects([]{PackageValidation::Count(0,0,0,"zero width");});
 rejects([]{PackageValidation::Outers(UINT32_MAX,1,[](int){return 0;},"overflow");});
 std::vector<int> chain(1025);for(int i=0;i<1025;++i)chain[i]=i;
 rejects([&]{PackageValidation::Outers(1025,0,[&](int ref){return chain[ref-1];},"deep");});
 std::cout<<"PASS "<<checks<<" production package table/stream cases\n";
}
'''
        with tempfile.TemporaryDirectory(prefix='poa-tables-') as directory:
            directory = pathlib.Path(directory)
            cpp, binary = directory / 'test.cpp', directory / 'test'
            cpp.write_text(harness)
            subprocess.run(['g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                            '-I', str(root / 'src/engine/SurrealEngine/Package'), str(cpp), '-o', str(binary)],
                           check=True, capture_output=True)
            subprocess.run([str(binary)], check=True,
                           env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


if __name__ == '__main__':
    unittest.main()
