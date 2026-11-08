import os
"""Production static mesh reader and loader with object service test doubles."""
import pathlib,tempfile,subprocess,unittest,os,struct,json,hashlib
from inspect_packages import inspect
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
class StaticMeshTests(unittest.TestCase):
 def runner(self,temp):
  source=(ENGINE/'Packages/Engine/Resources/Mesh/UPoaStaticMesh.cpp').read_text()
  code=r'''#include "Package/PoaStaticMeshData.h"
#include "Package/CompactIndex.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <iterator>
#include <utility>
template<class T>using Array=std::vector<T>;
struct NameString{std::string Text;bool IsNone()const{return Text=="None";}bool operator==(const char*s)const{return Text==s;}};
struct Exception{static void Throw(const char*s){throw std::runtime_error(s);}};
enum class ObjectFlags{HasStack=1};bool AllFlags(int a,ObjectFlags b){return (a&int(b))!=0;}
struct Package{};
struct ObjectStream{std::vector<uint8_t>Data;std::vector<std::string>Names;size_t Position=0,NativeStart=0;int Version=129;Package PackageValue;
 size_t Remaining(){return Data.size()-Position;}uint32_t Tell(){return Position;}void Seek(uint32_t p){if(p>Data.size())throw std::runtime_error("seek");Position=p;}void Skip(uint32_t n){if(n>Remaining())throw std::runtime_error("skip");Position+=n;}
 template<class T>T Read(){if(sizeof(T)>Remaining())throw std::runtime_error("truncated");T v;std::memcpy(&v,Data.data()+Position,sizeof(T));Position+=sizeof(T);return v;}
 uint8_t ReadUInt8(){return Read<uint8_t>();}uint16_t ReadUInt16(){return Read<uint16_t>();}uint32_t ReadUInt32(){return Read<uint32_t>();}float ReadFloat(){return Read<float>();}int ReadIndex(){return PackageIndices::Read([&](){return ReadUInt8();});}NameString ReadName(){int i=ReadIndex();if(i<0||size_t(i)>=Names.size())throw std::runtime_error("name");return {Names[i]};}int GetVersion(){return Version;}Package*GetPackage(){return &PackageValue;}};
struct UPrimitive{virtual~UPrimitive()=default;int Flags=0;void Load(ObjectStream*);};
struct UPoaStaticMesh:UPrimitive{PoaAssets::StaticMeshData Geometry;Array<int>MaterialReferences;Array<uint8_t>MaterialCollision;Package*SourcePackage=nullptr;bool UseSimpleBoxCollision=false;void Load(ObjectStream*);};
struct Tag { NameString Name; int Kind=0;uint32_t End=0;bool Bool=false; };
'''+definition(source,'Tag ReadTag(')+r'''
void UPrimitive::Load(ObjectStream*s){while(true){auto t=ReadTag(s);if(t.Name.IsNone())break;s->Seek(t.End);}s->Skip(41);s->NativeStart=s->Tell();}
'''+definition(source,'void UPoaStaticMesh::Load(')+r'''
uint32_t U32(std::istream&f){uint32_t x=0;f.read(reinterpret_cast<char*>(&x),4);if(!f)throw std::runtime_error("fixture");return x;}
int main(int argc,char**argv){
 if(argc==1){
  ObjectStream negative;negative.Data={0x81};bool bad=false;try{PoaAssets::ReadStaticMesh(negative,129);}catch(...){bad=true;}assert(bad);
  ObjectStream huge;huge.Data={0x7f,0xff,0xff,0xff,0x1f};bad=false;try{PoaAssets::ReadStaticMesh(huge,129);}catch(...){bad=true;}assert(bad);
  ObjectStream wrong;bad=false;try{PoaAssets::ReadStaticMesh(wrong,127);}catch(...){bad=true;}assert(bad);return 0;
 }
 std::ifstream f(argv[1],std::ios::binary);ObjectStream sample;sample.Version=U32(f);auto names=U32(f);for(uint32_t i=0;i<names;++i){std::string s(U32(f),'\0');f.read(s.data(),s.size());sample.Names.push_back(s);}auto count=U32(f);size_t vertices=0,triangles=0;
 for(uint32_t i=0;i<count;++i){ObjectStream stream=sample;stream.Data.resize(U32(f));f.read(reinterpret_cast<char*>(stream.Data.data()),stream.Data.size());assert(f);UPoaStaticMesh mesh;mesh.Load(&stream);assert(stream.Remaining()==0);vertices+=mesh.Geometry.Vertices.size();for(const auto&s:mesh.Geometry.Sections)triangles+=s.Triangles;
  // Retry representative truncated reads; a failed loader does not replace geometry.
  if(i==0){for(auto size:{size_t(0),size_t(1),size_t(12),stream.NativeStart+mesh.Geometry.NativeBytes-1}){ObjectStream cut=sample;cut.Data.assign(stream.Data.begin(),stream.Data.begin()+size);bool bad=false;try{mesh.Load(&cut);}catch(...){bad=true;}assert(bad);assert(!mesh.Geometry.Vertices.empty());}}
 }
 std::cout<<count<<" "<<vertices<<" "<<triangles;
}
'''
  t=pathlib.Path(temp);(t/'runner.cpp').write_text(code);exe=t/'runner'
  subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ENGINE),str(t/'runner.cpp'),'-o',str(exe)],check=True)
  return exe
 def test_malformed_count_and_version(self):
  with tempfile.TemporaryDirectory() as t:subprocess.run([str(self.runner(t))],check=True,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original EU mesh corpus')
 def test_original_eu_static_mesh_corpus(self):
  rows=[];totals=[0,0,0]
  with tempfile.TemporaryDirectory() as temp:
   exe=self.runner(temp)
   for p in sorted((pathlib.Path(os.environ['POA_EU_GAME'])/'StaticMeshes').glob('*.usx')):
    model=inspect(p);data=p.read_bytes();exports=[e for e in model['exports'] if e['class']=='StaticMesh'];parts=[struct.pack('<II',model['version'],len(model['names']))]
    for name in model['names']:
     b=name.encode('latin1');parts.extend([struct.pack('<I',len(b)),b])
    parts.append(struct.pack('<I',len(exports)))
    for e in exports:parts.extend([struct.pack('<I',e['size']),data[e['offset']:e['offset']+e['size']]])
    fixture=pathlib.Path(temp)/'case.bin';fixture.write_bytes(b''.join(parts));result=subprocess.check_output([str(exe),str(fixture)],text=True,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"));counts=list(map(int,result.split()));totals=[a+b for a,b in zip(totals,counts)];rows.append({'package':p.name,'sha256':hashlib.sha256(data).hexdigest(),'meshes':counts[0],'vertices':counts[1],'triangles':counts[2]})
  self.assertEqual(totals,[797,116099,91339]);print('PASS original EU mesh corpus:',totals)
  if os.environ.get('POA_MESH_REPORT'):pathlib.Path(os.environ['POA_MESH_REPORT']).write_text(json.dumps({'totals':dict(zip(['meshes','vertices','triangles'],totals)),'packages':rows,'scope':'Production native loader bodies with bounded stream and UPrimitive/object service doubles; not GPU or device execution'},indent=2)+'\n')
if __name__=='__main__':unittest.main()
