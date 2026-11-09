"""Production bone query/attachment/base bodies with actor adapters."""
import pathlib,tempfile,subprocess,unittest,os,re
from test_core_seven_runtime import definition
from inspect_packages import inspect
from object_payloads import function_header,native_stub
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class BoneAttachments(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for EU binding schemas')
 def test_six_bindings_match_eu_indices_and_arity(self):
  source=(ROOT/'src/hp3/PoaBones.cpp').read_text();bindings={name:(int(n),int(index))for n,name,index in re.findall(r'RegisterVMNativeFunc_(\d+)\("Actor","([^"]+)",&\w+,(\d+)\)',source)};self.assertEqual(len(bindings),6)
  p=pathlib.Path(os.environ['POA_EU_GAME'])/'System/Engine.u';m=inspect(p);raw=p.read_bytes();seen=set()
  for e in m['exports']:
   if e['class']=='Function'and e['name']in bindings and e['outer']>0 and m['exports'][e['outer']-1]['name']=='Actor':
    r,h=function_header(raw[e['offset']:e['offset']+e['size']],m['names']);stub=native_stub(r,h);self.assertEqual(bindings[e['name']],(len(stub['parameters'])+(e['name']!='BoneRefresh'),stub['nativeIndex']));seen.add(e['name'])
  self.assertEqual(seen,set(bindings))
 def test_bone_world_frames_reparent_detach_and_legacy_base(self):
  body='\n'.join(line for line in (ROOT/'src/hp3/PoaBones.cpp').read_text().splitlines()if not line.startswith('#include'))
  bases=(ENGINE/'Packages/Engine/Actors/UActor_Base.cpp').read_text();baseBodies='\n'.join(definition(bases,'void UActor::'+name+'(')for name in ('AddBasedActor','RemoveBasedActor','SetBase','RelinkBasedActor'))+definition(bases,'bool UActor::IsBasedOn(')
  transform=definition((ENGINE/'Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h').read_text(),'inline mat4 PoaStaticMeshToWorld(')
  code=r'''#include "Package/PoaBoneTransforms.h"
#include "Package/PoaAnimationChannels.h"
#include "hp3/PoaRotation.h"
#include <memory>
#include <cassert>
#include <map>
#include <set>
#include <functional>
#include <optional>
struct NameString{std::string Text;NameString(const char*s="None"):Text(s){}NameString(const std::string&s):Text(s){}std::string ToString()const{return Text;}bool operator!=(const NameString&b)const{return Text!=b.Text;}};
struct UObject{virtual~UObject()=default;void LoadNow(){}template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}template<class T>static T*Cast(UObject*p){return dynamic_cast<T*>(p);}};
struct UMesh:UObject{};struct UPoaSkeletalMesh:UMesh{PoaAssets::SkeletalMeshData Geometry;mat4 meshToObject=mat4::identity();};
struct FakeArray{std::vector<UObject*>*Values;size_t size(){return Values->size();}UObject*&operator[](size_t i){return (*Values)[i];}void push_back(UObject*p){Values->push_back(p);}void pop_back(){Values->pop_back();}};
struct UActor:UObject{UMesh*MeshObject=nullptr;UActor*Base=nullptr;std::vector<UActor*>BasedActors;std::vector<UObject*>Attached;NameString Bone;vec3 Position={0,0,0},Relative={0,0,0};Rotator Angle={0,0,0},RelativeAngle={0,0,0};float Scale=1;uint8_t Count=0;bool Deleted=false,HasCounter=false;std::vector<PoaAssets::BoneTransform>CustomPose;UMesh*&Mesh(){return MeshObject;}UActor*&ActorBase(){return Base;}UActor*Level(){return nullptr;}bool bDeleteMe(){return Deleted;}bool HasProperty(const char*n){return std::string(n)!="StandingCount"||HasCounter;}NameString GetName(const char*){return Bone;}void SetName(const char*,const NameString&name){Bone=name;}vec3 GetVector(const char*n){return std::string(n)=="RelativeLocation"?Relative:vec3(1);}Rotator GetRotator(const char*){return RelativeAngle;}int GetPropertyDataOffset(const char*){return 0;}template<class T>FakeArray DynamicArray(int){return {&Attached};}vec3&Location(){return Position;}vec3 PrePivot(){return {0,0,0};}Rotator Rotation(){return Angle;}float DrawScale(){return Scale;}uint8_t&StandingCount(){if(!HasCounter)throw std::runtime_error("absent counter");return Count;}bool SetLocation(const vec3&p){Position=p;return true;}bool SetRotation(const Rotator&r){Angle=r;return true;}void AddBasedActor(UActor*);void RemoveBasedActor(UActor*);void SetBase(UActor*,bool);void RelinkBasedActor();bool IsBasedOn(UActor*);};
struct BitfieldBool{bool&Value;BitfieldBool&operator=(bool b){Value=b;return*this;}};struct Exception{[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){}T*get(){return Value;}T*operator->(){return Value;}};
struct Profile{bool Poa=true;bool IsHarryPotter3(){return Poa;}};struct Engine{Profile LaunchInfo;};Engine instance;Engine*engine=&instance;
enum class EventName{Attach,Detach,BaseChange};struct ExpressionValue{UObject*Object;static ExpressionValue ObjectValue(UObject*p){return {p};}};void CallEvent(UObject*,EventName,std::vector<ExpressionValue>={}){}
namespace PoaBones{bool IsAttached(UActor*);bool UpdateAttachment(UActor*,int depth=0);void UpdateAttachments(UActor*);void SyncAttached(UActor*,UActor*,bool);}
namespace PoaActorAnimation{std::vector<PoaAssets::BoneTransform>Pose(UActor*actor,UPoaSkeletalMesh*mesh){return actor->CustomPose.empty()?PoaAssets::SkeletalPose(mesh->Geometry):actor->CustomPose;}}
template<class F>void RegisterVMNativeFunc_0(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_2(const char*,const char*,F,int){}template<class F>void RegisterVMNativeFunc_3(const char*,const char*,F,int){}
'''+transform+baseBodies+body+r'''
int main(){using namespace PoaBones;UPoaSkeletalMesh mesh;mesh.Geometry.Bones={{"root",0,{0,0,0,1},{0,0,0},0,{0,0,0},1,0},{"hand",0,{0,0,0,1},{1,0,0},0,{0,0,0},0,0}};UActor parent,child,other;parent.MeshObject=&mesh;parent.Position={10,20,30};parent.Scale=2;child.Relative={1,0,0};bool ok=false;BitfieldBool result{ok};HasBone(&parent,"hand",result);assert(ok);HasBone(&parent,"missing",result);assert(!ok);Coords coords;GetBoneCoords(&parent,"hand",coords);assert(length(coords.Origin-vec3(12,20,30))<1e-5&&std::abs(length(coords.XAxis)-2)<1e-5);
 AttachToBone(&parent,&child,"hand",result);assert(ok&&child.Base==&parent&&parent.BasedActors.size()==1&&parent.Attached.size()==1&&length(child.Position-vec3(14,20,30))<1e-5);child.RelinkBasedActor();assert(parent.BasedActors.size()==1&&parent.Attached.size()==1);
 parent.CustomPose=PoaAssets::SkeletalPose(mesh.Geometry);parent.CustomPose[1].Position[0]=2;BoneRefresh(&parent);assert(length(child.Position-vec3(16,20,30))<1e-5);
 parent.Angle={0,16384,0};Rotator rotation;GetBoneRotation(&parent,"hand",0,rotation);assert(std::abs(rotation.Yaw-16384)<8);UpdateAttachments(&parent);assert(length(child.Position-vec3(10,26,30))<1e-4);
 other.MeshObject=&mesh;AttachToBone(&other,&child,"hand",result);assert(ok&&child.Base==&other&&parent.Attached.empty()&&other.Attached.size()==1);auto position=child.Position;DetachFromBone(&other,&child,result);assert(ok&&!child.Base&&child.Bone.Text=="None"&&other.Attached.empty()&&length(child.Position-position)<1e-5);
 AttachToBone(&parent,&parent,"hand",result);assert(!ok);AttachToBone(&parent,&child,"missing",result);assert(!ok);
 instance.LaunchInfo.Poa=false;parent.HasCounter=child.HasCounter=true;child.SetBase(&parent,true);assert(parent.Count==1);child.SetBase(nullptr,true);assert(parent.Count==0);RegisterNatives();
}
'''
  code=code.replace('std::vector<ExpressionValue>={}', 'std::vector<ExpressionValue> args={}').replace('void CallEvent(UObject*,EventName,std::vector<ExpressionValue> args={}){}', 'void CallEvent(UObject*,EventName,std::vector<ExpressionValue> = {}){}')
  with tempfile.TemporaryDirectory()as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ENGINE),'-I'+str(ROOT/'src'),str(t/'test.cpp'),str(ENGINE/'Math/mat.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
