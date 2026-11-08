"""Production transform/material/draw bodies; capture-device and object doubles."""
import pathlib,subprocess,tempfile,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class StaticRender(unittest.TestCase):
 def test_transforms_materials_and_render_passes(self):
  draw=(ENGINE/'Render/VisiblePoaStaticMesh.h').read_text();actor=(ENGINE/'Packages/Engine/Resources/Mesh/PoaStaticMeshActor.h').read_text()
  code=r'''#include "Package/PoaStaticMeshData.h"
#include "Math/coords.h"
#include <cassert>
#include <string>
#include <map>
#include <set>
#include <vector>
struct UObject{virtual~UObject()=default;std::map<std::string,UObject*>Objects;std::map<std::string,int>Values;void LoadNow(){}bool HasProperty(const std::string&n){return Objects.count(n)||Values.count(n);}bool GetBool(const std::string&n){return Values[n]!=0;}int GetByte(const std::string&n){return Values[n];}UObject*GetUObject(const std::string&n){return Objects[n];}template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}};
struct UTexture:UObject{};struct Package{UObject*Material=nullptr;UObject*GetUObject(int){return Material;}};
struct UPoaStaticMesh:UObject{PoaAssets::StaticMeshData Geometry;std::vector<int>MaterialReferences;Package*SourcePackage=nullptr;};
constexpr int STY_Normal=1,STY_Masked=2,STY_Translucent=3,STY_Modulated=4;
constexpr uint32_t PF_TwoSided=1,PF_Masked=2,PF_Translucent=4,PF_Modulated=8,PF_Highlighted=16;
struct UActor:UObject{UPoaStaticMesh*Mesh=nullptr;vec3 Location(){return {10,20,30};}vec3 PrePivot(){return {1,2,0};}Rotator Rotation(){return {0,0,0};}float DrawScale(){return 2;}vec3 GetVector(const std::string&){return {-2,3,1};}int Style(){return STY_Normal;}};
UPoaStaticMesh*GetPoaStaticMesh(UActor*a){return a->Mesh;}
struct TextureInfo{int USize=64,VSize=128;};
struct GouraudVertex{vec3 Point,Light;vec2 UV;vec4 Fog;float Alpha=1;};
struct SceneNode{};struct Device{int Calls=0;uint32_t Flags=0;GouraudVertex Captured[3];void DrawGouraudPolygon(SceneNode*,TextureInfo&,const GouraudVertex*p,int count,uint32_t flags){assert(count==3);++Calls;Flags=flags;for(int i=0;i<3;++i)Captured[i]=p[i];}};
struct VisibleFrame{struct Device*Device;SceneNode Frame;};
struct Render{void UpdateTexture(UTexture*){}void UpdateTextureInfo(TextureInfo&,UTexture*){}};struct Engine{Render*render;};Render render;Engine instance{&render};Engine*engine=&instance;
'''+definition(actor,'inline mat4 PoaStaticMeshToWorld(')+definition(draw,'inline UTexture* PoaDiffuseTexture(')+definition(draw,'inline bool DrawPoaStaticMesh(')+r'''
int main(){UPoaStaticMesh mesh;mesh.Geometry.Sections.push_back({0,0,0,2,0,1});mesh.Geometry.Vertices={{{0,0,0},{0,0,1}},{{1,0,0},{0,0,1}},{{0,1,0},{0,0,1}}};mesh.Geometry.Indices={0,1,2};mesh.Geometry.UV={{{0,0},{1,.5},{0,1}}};mesh.MaterialReferences={1};Package package;mesh.SourcePackage=&package;UTexture texture;UObject shader;shader.Objects["Diffuse"]=&texture;shader.Values["TwoSided"]=1;package.Material=&shader;UActor actor;actor.Mesh=&mesh;actor.Values["DrawScale3D"]=1;Device device;VisibleFrame frame{&device,{}};
 assert(!DrawPoaStaticMesh(&frame,&actor,false)&&device.Calls==1);assert(device.Flags&PF_TwoSided);assert(length(device.Captured[0].Point-vec3(14,8,30))<.001);assert(length(device.Captured[2].Point-vec3(10,8,30))<.001);assert(length(device.Captured[1].Point-vec3(14,14,30))<.001);assert(device.Captured[2].UV.x==64&&device.Captured[2].UV.y==64);
 shader.Values["OutputBlending"]=3;device.Calls=0;assert(DrawPoaStaticMesh(&frame,&actor,false)&&device.Calls==0);assert(!DrawPoaStaticMesh(&frame,&actor,true)&&device.Calls==1&&(device.Flags&PF_Translucent));
 shader.Values["OutputBlending"]=1;device.Calls=0;assert(!DrawPoaStaticMesh(&frame,&actor,true)&&device.Calls==0);assert(!DrawPoaStaticMesh(&frame,&actor,false)&&device.Calls==1&&(device.Flags&PF_Masked));
 shader.Values["OutputBlending"]=0;shader.Objects["Diffuse"]=&shader;device.Calls=0;assert(!DrawPoaStaticMesh(&frame,&actor,false)&&device.Calls==0);
 shader.Objects["Diffuse"]=&texture;shader.Values["OutputBlending"]=4;assert(!DrawPoaStaticMesh(&frame,&actor,false)&&device.Calls==0);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(t/'test.cpp'),str(ENGINE/'Math/mat.cpp'),'-o',str(t/'test')],check=True)
   subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
