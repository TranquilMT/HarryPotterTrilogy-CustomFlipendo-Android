"""Port channel state round trips and production actor trailer integration."""
import pathlib,tempfile,subprocess,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class AnimationStateIO(unittest.TestCase):
 def test_cold_round_trip_validation_and_actor_trailers(self):
  source=(ROOT/'src/hp3/PoaActorAnimation.cpp').read_text();load=definition(source,'void Load(UActor* actor,ObjectStream* stream)');save=definition(source,'void Save(UActor* actor,PackageStreamWriter* stream)')
  code=r'''#include "Package/PoaAnimationStateIO.h"
#include <memory>
#include <cassert>
#include <cstring>
struct UPoaMeshAnimation{};struct Exception{[[noreturn]]static void Throw(const char*s){throw std::runtime_error(s);}};
struct UActor{std::shared_ptr<PoaChannels::State>PoaAnimation;UPoaMeshAnimation*PoaAnimationObject=nullptr;};
struct PackageStreamWriter{std::vector<uint8_t>Bytes;UPoaMeshAnimation*Resource=nullptr;template<class T>void Put(T v){auto p=reinterpret_cast<const uint8_t*>(&v);Bytes.insert(Bytes.end(),p,p+sizeof(v));}void WriteUInt32(uint32_t v){Put(v);}void WriteInt32(int32_t v){Put(v);}void WriteUInt64(uint64_t v){Put(v);}void WriteFloat(float v){Put(v);}void WriteString(const std::string&s){WriteUInt32(s.size());Bytes.insert(Bytes.end(),s.begin(),s.end());}void WriteObject(UPoaMeshAnimation*p){Resource=p;WriteInt32(p?1:0);}};
struct ObjectStream{std::vector<uint8_t>Bytes;size_t Pos=0;UPoaMeshAnimation*Resource=nullptr;size_t Remaining(){return Bytes.size()-Pos;}uint32_t Tell(){return uint32_t(Pos);}void Seek(uint32_t pos){if(pos>Bytes.size())throw std::runtime_error("seek");Pos=pos;}template<class T>T Get(){if(sizeof(T)>Remaining())throw std::runtime_error("truncated");T v;std::memcpy(&v,Bytes.data()+Pos,sizeof(v));Pos+=sizeof(v);return v;}uint32_t ReadUInt32(){return Get<uint32_t>();}int32_t ReadInt32(){return Get<int32_t>();}uint64_t ReadUInt64(){return Get<uint64_t>();}float ReadFloat(){return Get<float>();}std::string ReadString(){auto n=ReadUInt32();if(n>Remaining()||n>4096)throw std::runtime_error("string");std::string s(reinterpret_cast<const char*>(Bytes.data()+Pos),n);Pos+=n;return s;}template<class T>T*ReadObject(){auto index=ReadInt32();if(index<0||index>1)throw std::runtime_error("ref");return index?Resource:nullptr;}};
'''+load+save+r'''
int main(){using namespace PoaChannels;UActor actor;actor.PoaAnimation=std::make_shared<State>();auto&c=actor.PoaAnimation->At(3);c.Name="Walk";c.FromName="Idle";c.Bone="Spine";c.Phase=.123456789012;c.FromPhase=.75;c.Rate=1.5;c.Alpha=.4;c.TargetAlpha=.8;c.AlphaTime=.3;c.Tween=.2;c.TweenTotal=.5;c.BlendIn=2.5;c.BlendOut=3;c.FromPose={{{0,0,0,1},{1,2,3}}};c.Playing=c.Loop=c.GlobalPose=true;actor.PoaAnimation->Waiting=3;UPoaMeshAnimation resource;actor.PoaAnimationObject=&resource;PackageStreamWriter writer;Save(&actor,&writer);
 UActor restored;ObjectStream reader;reader.Bytes=writer.Bytes;reader.Resource=writer.Resource;Load(&restored,&reader);assert(restored.PoaAnimationObject==&resource&&reader.Remaining()==0);auto&out=restored.PoaAnimation->At(3);assert(out.BlendIn==c.BlendIn&&out.BlendOut==c.BlendOut&&out.FromPose.size()==1&&out.FromPose[0].Position==c.FromPose[0].Position&&out.Name==c.Name&&out.FromName==c.FromName&&out.Bone==c.Bone&&out.Phase==c.Phase&&out.FromPhase==c.FromPhase&&out.Rate==c.Rate&&out.Alpha==c.Alpha&&out.Tween==c.Tween&&out.Loop&&out.Playing&&out.GlobalPose&&restored.PoaAnimation->Waiting==3);
 // A new actor and stream perform the cold load; serialize again identically.
 PackageStreamWriter second;Save(&restored,&second);assert(second.Bytes==writer.Bytes);
 for(size_t length:{size_t(4),size_t(8),size_t(12),writer.Bytes.size()-1}){auto cut=reader;cut.Pos=0;cut.Bytes.resize(length);auto previous=restored.PoaAnimation;bool failed=false;try{Load(&restored,&cut);}catch(...){failed=true;}assert(failed&&restored.PoaAnimation==previous);}
 auto extra=reader;extra.Pos=0;extra.Bytes.push_back(0);bool failed=false;try{Load(&restored,&extra);}catch(...){failed=true;}assert(failed);
 auto unknown=reader;unknown.Pos=0;unknown.Bytes[8]=99;failed=false;try{Load(&restored,&unknown);}catch(...){failed=true;}assert(failed);
 ObjectStream original;original.Bytes={1,2,3,4,5,6,7,8};Load(&restored,&original);assert(original.Pos==0&&!restored.PoaAnimation&&!restored.PoaAnimationObject);
 PackageStreamWriter absent;Save(&restored,&absent);assert(absent.Bytes.empty());
 State bad;bad.Waiting=16;failed=false;try{WriteState(absent,bad);}catch(...){failed=true;}assert(failed&&absent.Bytes.empty());bad.Waiting=-1;bad.At(0).Rate=NAN;failed=false;try{WriteState(absent,bad);}catch(...){failed=true;}assert(failed);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
