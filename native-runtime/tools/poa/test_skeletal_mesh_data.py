"""Bounded production skeletal reader and CPU skinning; original EU corpus."""
import pathlib,tempfile,subprocess,unittest,os,struct,json,hashlib
from inspect_packages import inspect,Reader
from object_payloads import tags
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class SkeletalMeshTests(unittest.TestCase):
 def build(self,temp):
  code=r'''#include "Package/PoaAnimationChannels.h"
#include <map>
#include "Package/CompactIndex.h"
#include <fstream>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
struct NameString{std::string Text;std::string ToString(){return Text;}};
struct ObjectStream{std::vector<uint8_t>Data;std::vector<std::string>Names;size_t Pos=0,Offset=0;size_t Tell(){return Offset+Pos;}size_t Remaining(){return Data.size()-Pos;}template<class T>T Read(){if(sizeof(T)>Remaining())throw std::runtime_error("truncated");T v;std::memcpy(&v,Data.data()+Pos,sizeof(T));Pos+=sizeof(T);return v;}uint8_t ReadUInt8(){return Read<uint8_t>();}uint16_t ReadUInt16(){return Read<uint16_t>();}uint32_t ReadUInt32(){return Read<uint32_t>();}int32_t ReadInt32(){return Read<int32_t>();}float ReadFloat(){return Read<float>();}int ReadIndex(){return PackageIndices::Read([&](){return ReadUInt8();});}NameString ReadName(){int i=ReadIndex();if(i<0||size_t(i)>=Names.size())throw std::runtime_error("name");return {Names[i]};}};
uint32_t U32(std::istream&f){uint32_t n=0;f.read(reinterpret_cast<char*>(&n),4);if(!f)throw std::runtime_error("fixture");return n;}
int main(int argc,char**argv){using namespace PoaAssets;
 if(argc==1){
  SkeletalMeshData mesh;mesh.Bones={{"root",0,{0,0,0,1},{0,0,0},0,{0,0,0},1,0},{"child",0,{0,0,0,1},{1,0,0},0,{0,0,0},0,0}};mesh.Points={{2,0,0},{5,0,0},{0,1,0}};mesh.Influences={{.25f,0,0},{.75f,0,1},{1,2,1}};
  auto bind=SkeletalPose(mesh);auto unchanged=SkinSkeletalPoints(mesh,bind);assert(unchanged==mesh.Points);
  auto pose=bind;pose[1].Position[0]+=4;auto moved=SkinSkeletalPoints(mesh,pose);assert(std::abs(moved[0][0]-5)<1e-6&&moved[1]==mesh.Points[1]&&std::abs(moved[2][0]-4)<1e-6);
  pose=bind;pose[1].Rotation={0,0,float(std::sqrt(.5)),float(std::sqrt(.5))};auto rotated=SkinSkeletalPoints(mesh,pose);assert(std::abs(rotated[2][0])<1e-5&&std::abs(rotated[2][1]+1)<1e-5);
  AnimationData animation;animation.Bones={{"child",0,0}};animation.Moves.resize(1);animation.Moves[0].Duration=2;AnimationTrack track;track.Position={{1,0,0},{3,0,0}};track.Time={0,1};animation.Moves[0].Tracks={track};animation.Sequences.resize(1);
  auto animated=SkeletalPose(mesh,&animation,0,.5,false);assert(std::abs(animated[1].Position[0]-2)<1e-5);
  auto hierarchy=mesh;hierarchy.Bones[0].Orientation={0,0,float(std::sqrt(.5)),float(std::sqrt(.5))};hierarchy.Bones[1].Orientation=hierarchy.Bones[0].Orientation;auto composed=SkeletalPose(hierarchy);assert(std::abs(composed[1].Position[0])<1e-5&&std::abs(composed[1].Position[1]-1)<1e-5);auto basis=RotateBoneVector(composed[1].Rotation,{1,0,0});assert(std::abs(basis[0]-1)<1e-5&&std::abs(basis[1])<1e-5);
  bool bad=false;mesh.Bones[0].Parent=1;try{SkeletalPose(mesh);}catch(...){bad=true;}assert(bad);mesh.Bones[0].Parent=0;
  mesh.Influences[0].Bone=50;bad=false;try{SkinSkeletalPoints(mesh,bind);}catch(...){bad=true;}assert(bad);
  ObjectStream stream;stream.Data={4,0,0,0};bad=false;try{ReadSkeletalMesh(stream,129);}catch(...){bad=true;}assert(bad);
  return 0;
 }
 std::ifstream f(argv[1],std::ios::binary);ObjectStream original;auto names=U32(f);for(uint32_t i=0;i<names;++i){std::string s(U32(f),'\0');f.read(s.data(),s.size());original.Names.push_back(s);}auto count=U32(f);size_t bones=0,points=0,triangles=0,weights=0,lods=0,sockets=0;std::vector<SkeletalMeshData> meshes;
 for(uint32_t i=0;i<count;++i){ObjectStream stream=original;stream.Offset=U32(f);stream.Data.resize(U32(f));f.read(reinterpret_cast<char*>(stream.Data.data()),stream.Data.size());assert(f);auto mesh=ReadSkeletalMesh(stream,129);bones+=mesh.Bones.size();points+=mesh.Points.size();triangles+=mesh.Triangles.size();weights+=mesh.Influences.size();lods+=mesh.LODCount;sockets+=mesh.Sockets.size();auto bind=SkeletalPose(mesh);auto skinned=SkinSkeletalPoints(mesh,bind);for(size_t p=0;p<skinned.size();++p)for(size_t c=0;c<3;++c)assert(std::abs(skinned[p][c]-mesh.Points[p][c])<.002f);
  meshes.push_back(mesh);
  if(i==0){for(size_t n:{size_t(0),size_t(4),stream.Data.size()-1}){auto cut=stream;cut.Pos=0;cut.Data.resize(n);bool rejected=false;try{ReadSkeletalMesh(cut,129);}catch(...){rejected=true;}assert(rejected);}auto wrongOffset=stream;wrongOffset.Pos=0;wrongOffset.Offset++;bool rejected=false;try{ReadSkeletalMesh(wrongOffset,129);}catch(...){rejected=true;}assert(rejected);}
 }
 std::map<int,AnimationData> animations;auto animationCount=U32(f);
 for(uint32_t i=0;i<animationCount;++i){int reference=int(U32(f));ObjectStream stream=original;stream.Data.resize(U32(f));f.read(reinterpret_cast<char*>(stream.Data.data()),stream.Data.size());assert(f);animations.emplace(reference,ReadAnimation(stream,129));}
 size_t animatedMeshes=0,animatedPoints=0;
 for(const auto& mesh:meshes){auto found=animations.find(mesh.AnimationReference);if(found==animations.end())continue;const auto& animation=found->second;if(animation.Sequences.empty())continue;PoaChannels::State state;PoaChannels::Play(state,animation,animation.Sequences[0].Name,1,0,0,true);PoaChannels::Tick(state,animation,.1f);auto pose=PoaChannels::Pose(mesh,animation,state);auto animated=SkinSkeletalPoints(mesh,pose);for(const auto& point:animated)for(auto v:point)assert(std::isfinite(v));++animatedMeshes;animatedPoints+=animated.size();}
 assert(meshes.empty()||animatedMeshes>0||animationCount==0);
 std::cerr<<" animated meshes "<<animatedMeshes<<" points "<<animatedPoints<<"\n";
 std::cout<<count<<" "<<bones<<" "<<points<<" "<<triangles<<" "<<weights<<" "<<lods<<" "<<sockets<<" "<<animatedMeshes<<" "<<animatedPoints;
}
'''
  t=pathlib.Path(temp);(t/'test.cpp').write_text(code);exe=t/'test';subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(exe)],check=True);return exe
 def test_skinning_hierarchy_animation_and_rejections(self):
  with tempfile.TemporaryDirectory() as temp:subprocess.run([str(self.build(temp))],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original EU meshes')
 def test_original_eu_skeletal_corpus(self):
  rows=[];totals=[0]*9
  with tempfile.TemporaryDirectory() as temp:
   exe=self.build(temp)
   for p in sorted((pathlib.Path(os.environ['POA_EU_GAME'])/'Animations').glob('*.ukx')):
    model=inspect(p);data=p.read_bytes();parts=[struct.pack('<I',len(model['names']))]
    for name in model['names']:
     value=name.encode('latin1');parts.extend([struct.pack('<I',len(value)),value])
    exports=[e for e in model['exports'] if e['class']=='SkeletalMesh'];parts.append(struct.pack('<I',len(exports)))
    for e in exports:
     raw=data[e['offset']:e['offset']+e['size']];r=Reader(raw);tags(r,model['names']);r.read(41);raw=raw[r.p:];parts.extend([struct.pack('<II',e['offset']+r.p,len(raw)),raw])
    animations=[(i,e)for i,e in enumerate(model['exports'],1)if e['class']=='MeshAnimation'];parts.append(struct.pack('<I',len(animations)))
    for i,e in animations:
     raw=data[e['offset']:e['offset']+e['size']];r=Reader(raw);tags(r,model['names']);raw=raw[r.p:];parts.extend([struct.pack('<II',i,len(raw)),raw])
    path=pathlib.Path(temp)/'case.bin';path.write_bytes(b''.join(parts));counts=list(map(int,subprocess.check_output([str(exe),str(path)],text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')).split()));totals=[a+b for a,b in zip(totals,counts)];rows.append({'package':p.name,'sha256':hashlib.sha256(data).hexdigest(),'totals':counts})
  self.assertEqual(totals[:6],[167,8790,105997,163721,173285,668]);self.assertEqual(totals[7:],[146,104781]);print('PASS original EU skeletal corpus',totals)
  if os.environ.get('POA_SKELETAL_REPORT'):pathlib.Path(os.environ['POA_SKELETAL_REPORT']).write_text(json.dumps({'columns':['meshes','bones','points','triangles','influences','LODs','sockets','animatedMeshes','animatedPoints'],'totals':totals,'packages':rows,'scope':'Production decoder and CPU skinning; bounded stream adapters. Bind-pose identity across original corpus; channel-driven first-sequence samples for all 146 meshes with a local default animation reference (104781 points), checking finite output. This is not an independent expected animated pose. Not original playback parity or Android GPU acceptance.'},indent=2)+'\n')
if __name__=='__main__':unittest.main()
