"""Production UE2 animation decoder/sampler against original EU resources."""
import pathlib,tempfile,subprocess,unittest,os,struct,json,hashlib
from inspect_packages import inspect,Reader
from object_payloads import tags
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class AnimationDataTests(unittest.TestCase):
 def build(self,temp):
  loader=definition((ENGINE/'Packages/Engine/Resources/Mesh/UPoaMeshAnimation.cpp').read_text(),'void UPoaMeshAnimation::Load(')
  code=r'''#include "Package/PoaAnimationData.h"
#include "Package/CompactIndex.h"
#include <fstream>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
struct NameString{std::string Text;std::string ToString(){return Text;}};
struct ObjectStream{std::vector<uint8_t>Data;std::vector<std::string>Names;size_t Pos=0;int GetVersion(){return 129;}size_t Remaining(){return Data.size()-Pos;}template<class T>T Read(){if(sizeof(T)>Remaining())throw std::runtime_error("truncated");T v;std::memcpy(&v,Data.data()+Pos,sizeof(T));Pos+=sizeof(T);return v;}uint8_t ReadUInt8(){return Read<uint8_t>();}uint16_t ReadUInt16(){return Read<uint16_t>();}uint32_t ReadUInt32(){return Read<uint32_t>();}int32_t ReadInt32(){return Read<int32_t>();}float ReadFloat(){return Read<float>();}int ReadIndex(){return PackageIndices::Read([&](){return ReadUInt8();});}NameString ReadName(){int i=ReadIndex();if(i<0||size_t(i)>=Names.size())throw std::runtime_error("name");return {Names[i]};}};
struct UObject{void Load(ObjectStream*){}};struct UPoaMeshAnimation:UObject{PoaAssets::AnimationData Animation;void Load(ObjectStream*);};
'''+loader+r'''
uint32_t U32(std::istream&f){uint32_t n=0;f.read(reinterpret_cast<char*>(&n),4);if(!f)throw std::runtime_error("fixture");return n;}
int main(int argc,char**argv){using namespace PoaAssets;
 if(argc==1){
  AnimationTrack track;track.Time={0,1};track.Position={Vector{0,0,0},Vector{10,0,0}};track.Rotation={Quat{0,0,0,1},Quat{0,0,1,0}};
  auto halfway=SampleAnimationTrack(track,.5,2,false);assert(halfway.HasPosition&&halfway.HasRotation&&std::abs(halfway.Position[0]-5)<1e-5&&std::abs(halfway.Rotation[2]-.70710678)<1e-5);
  auto loop=SampleAnimationTrack(track,1.5,2,true);assert(std::abs(loop.Position[0]-5)<1e-5);auto clamp=SampleAnimationTrack(track,10,2,false);assert(clamp.Position[0]==10);auto negative=SampleAnimationTrack(track,-.5,2,true);assert(std::abs(negative.Position[0]-5)<1e-5);
  track.Rotation={Quat{0,0,0,1},Quat{0,0,0,-1}};auto aligned=SampleAnimationTrack(track,.5,2,false);assert(std::abs(aligned.Rotation[3]-1)<1e-5);
  auto identity=UnitAnimationQuat({0,0,0,0});assert(identity==Quat({0,0,0,1}));
  AnimationTrack absent;absent.Position={Vector{NAN,NAN,NAN}};assert(!SampleAnimationTrack(absent,0,1,false).HasPosition);
  AnimationTrack empty;assert(!SampleAnimationTrack(empty,0,0,false).HasRotation);bool rejected=false;try{SampleAnimationTrack(track,NAN,2,false);}catch(...){rejected=true;}assert(rejected);
  auto positionStream=[](Vector position){ObjectStream r;auto put=[&](auto v){auto p=reinterpret_cast<const uint8_t*>(&v);r.Data.insert(r.Data.end(),p,p+sizeof(v));};put(uint32_t(0));r.Data.push_back(0);r.Data.push_back(1);for(auto v:position)put(v);r.Data.push_back(0);return r;};
  auto unavailable=positionStream({NAN,NAN,NAN});auto preserved=ReadAnimationTrack(unavailable);assert(!SampleAnimationTrack(preserved,0,1,false).HasPosition);
  for(auto position:{Vector{NAN,0,0},Vector{INFINITY,INFINITY,INFINITY}}){auto invalid=positionStream(position);rejected=false;try{ReadAnimationTrack(invalid);}catch(...){rejected=true;}assert(rejected);}
  ObjectStream stream;stream.Data={4,0,0,0,0x81};rejected=false;try{ReadAnimation(stream,129);}catch(...){rejected=true;}assert(rejected);
  return 0;
 }
 std::ifstream f(argv[1],std::ios::binary);ObjectStream original;auto names=U32(f);for(uint32_t i=0;i<names;++i){std::string s(U32(f),'\0');f.read(s.data(),s.size());original.Names.push_back(s);}auto count=U32(f);size_t bones=0,moves=0,tracks=0,keys=0,samples=0,zeroKeys=0,absentPositions=0;
 for(uint32_t i=0;i<count;++i){ObjectStream stream=original;stream.Data.resize(U32(f));f.read(reinterpret_cast<char*>(stream.Data.data()),stream.Data.size());assert(f);UPoaMeshAnimation resource;try{resource.Load(&stream);}catch(const std::exception&e){throw std::runtime_error("export "+std::to_string(i)+": "+e.what());}auto&data=resource.Animation;bones+=data.Bones.size();moves+=data.Moves.size();
  for(size_t sequence=0;sequence<data.Moves.size();++sequence){const auto&move=data.Moves[sequence];tracks+=move.Tracks.size();for(const auto&track:move.Tracks){for(const auto&p:track.Position)if(std::isnan(p[0]))++absentPositions;keys+=track.Rotation.size()+track.Position.size()+track.Time.size();for(const auto&q:track.Rotation){double norm=0;for(auto v:q)norm+=v*v;if(norm==0)++zeroKeys;}}for(size_t bone=0;bone<data.Bones.size();++bone){auto sample=SampleAnimationBone(data,sequence,bone,move.Duration*.5f,true);if(sample.HasRotation){double norm=0;for(auto value:sample.Rotation)norm+=value*value;assert(std::abs(norm-1)<1e-4);}++samples;}}
  if(i==0){auto previous=data.Moves.size();for(size_t n:{size_t(0),size_t(4),stream.Data.size()-1}){auto cut=stream;cut.Pos=0;cut.Data.resize(n);bool rejected=false;try{resource.Load(&cut);}catch(...){rejected=true;}assert(rejected&&resource.Animation.Moves.size()==previous);}}
 }
 std::cout<<count<<" "<<bones<<" "<<moves<<" "<<tracks<<" "<<keys<<" "<<samples<<" "<<zeroKeys<<" "<<absentPositions;
}
'''
  t=pathlib.Path(temp);(t/'test.cpp').write_text(code);exe=t/'test';subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(exe)],check=True);return exe
 def test_interpolation_loop_clamp_and_rejections(self):
  with tempfile.TemporaryDirectory() as temp:subprocess.run([str(self.build(temp))],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original EU animation corpus')
 def test_original_eu_animation_corpus(self):
  rows=[];totals=[0]*8
  with tempfile.TemporaryDirectory() as temp:
   exe=self.build(temp)
   for p in sorted((pathlib.Path(os.environ['POA_EU_GAME'])/'Animations').glob('*.ukx')):
    model=inspect(p);data=p.read_bytes();parts=[struct.pack('<I',len(model['names']))]
    for name in model['names']:
     value=name.encode('latin1');parts.extend([struct.pack('<I',len(value)),value])
    exports=[e for e in model['exports'] if e['class']=='MeshAnimation'];parts.append(struct.pack('<I',len(exports)))
    for e in exports:
     raw=data[e['offset']:e['offset']+e['size']];r=Reader(raw);tags(r,model['names']);raw=raw[r.p:];parts.extend([struct.pack('<I',len(raw)),raw])
    path=pathlib.Path(temp)/'case.bin';path.write_bytes(b''.join(parts));counts=list(map(int,subprocess.check_output([str(exe),str(path)],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')).split()));totals=[a+b for a,b in zip(totals,counts)];rows.append({'package':p.name,'sha256':hashlib.sha256(data).hexdigest(),'totals':counts})
  self.assertEqual(totals[0],140);self.assertEqual(totals[2],3464);self.assertEqual(totals[6],298);self.assertEqual(totals[7],2);print('PASS original EU animation corpus',totals)
  if os.environ.get('POA_ANIMATION_REPORT'):pathlib.Path(os.environ['POA_ANIMATION_REPORT']).write_text(json.dumps({'columns':['resources','referenceBones','moves','tracks','keys','boneSamples','zeroQuaternionKeys','unavailablePositionKeys'],'totals':totals,'packages':rows,'scope':'Production native animation reader/sampler with stream/object adapters; not skeletal GPU integration or device gameplay'},indent=2)+'\n')
if __name__=='__main__':unittest.main()
