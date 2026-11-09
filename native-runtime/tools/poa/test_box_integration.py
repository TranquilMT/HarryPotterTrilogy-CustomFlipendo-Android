"""Box geometry oracle and production width-query/placement routing."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class BoxIntegration(unittest.TestCase):
 def run_cpp(self,code):
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-unused-variable','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 def test_primitive_sweeps_against_distance_oracle(self):
  self.run_cpp(r'''#include "Collision/BottomLevel/PoaBoxCollision.h"
#include <cassert>
#include <random>
int main(){dvec3 n;assert(PoaCollision::SweepBox({0,-10,0},{0,1,0},0,20,{1,4,1},n)==6&&n.y==-1);
 assert(PoaCollision::SweepBoxCylinder({0,-10,0},{0,1,0},0,20,{1,4,1},2,1,n)==4);
 assert(PoaCollision::BoxCylinderOverlap({0,0,0},{1,4,1},0,1));assert(!PoaCollision::BoxCylinderOverlap({2,5,0},{1,4,1},1,1));
 std::mt19937 rng(29);std::uniform_real_distribution<double> rnd(-1,1);
 for(int k=0;k<4000;++k){dvec3 box(1+std::abs(rnd(rng))*3,1+std::abs(rnd(rng))*6,1+std::abs(rnd(rng))*2);double radius=.1+std::abs(rnd(rng))*2,height=.1+std::abs(rnd(rng))*2;dvec3 p(rnd(rng)*20,rnd(rng)*20,rnd(rng)*20);double len=length(p);if(len<.001)continue;dvec3 d=-p/len;
 auto inside=[&](double t){auto q=p+d*t;double x=std::max(std::abs(q.x)-box.x,0.0),y=std::max(std::abs(q.y)-box.y,0.0);return x*x+y*y<=radius*radius && std::abs(q.z)<=box.z+height;};
 double expected=0;if(!inside(0)){double lo=0,hi=len;for(int j=0;j<70;++j){double mid=(lo+hi)/2;if(inside(mid))hi=mid;else lo=mid;}expected=hi;}
 double actual=PoaCollision::SweepBoxCylinder(p,d,0,len+1,box,radius,height,n);assert(std::abs(actual-expected)<1e-8);if(actual>1e-8)assert(std::abs(length(n)-1)<1e-8);
 // A path offset beyond both expanded XY axes must miss.
 assert(PoaCollision::SweepBoxCylinder({box.x+radius+1,box.y+radius+1,20},{0,0,-1},0,40,box,radius,height,n)==40);
 }
}''')
 def test_production_trace_query_preserves_width(self):
  body=definition((ENGINE/'Collision/TopLevel/TraceTest.cpp').read_text(),'CollisionHitList TraceTester::TraceExtent(')
  self.run_cpp(r'''#include "Math/vec.h"
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
struct UActor{struct{int CheckCounter=0;}Collision;};struct Hit{float Fraction;vec3 Normal;UActor*Actor;};struct CollisionHitList:std::vector<Hit>{using std::vector<Hit>::push_back;void push_back(const CollisionHitList&h){insert(end(),h.begin(),h.end());}};
struct Model{std::vector<int>Nodes{1};};struct Level{struct Model*Model;};
struct TraceRayModel{CollisionHitList Trace(UActor*,Model*,dvec3,double,dvec3,double,bool){return {};}};
struct TraceAABBModel{static inline dvec3 Extent{0.0};CollisionHitList Trace(UActor*,Model*,dvec3,double,dvec3,double,dvec3 e,bool){Extent=e;return {};}};
struct TraceTester{UActor Actor;Model M;Level L{&M};bool World=true;vec3 Seen{0.0f};
 int NextCheckCounter(){return 1;}ivec3 GetSweepStartExtents(vec3,vec3,vec3 e){Seen=e;return {0,0,0};}ivec3 GetSweepEndExtents(vec3,vec3,vec3){return {1,1,1};}std::vector<UActor*>GetActors(int,int,int){return {&Actor};}UActor*GetLevelInfo(){return &Actor;}Level*GetLevel(){return World?&L:nullptr;}
 void TraceActor(UActor*a,dvec3,double,dvec3,double,double,double,bool,bool,bool,CollisionHitList&hits,const dvec3*box){assert(box&&box->x==2&&box->y==7&&box->z==5);hits.push_back({4,vec3(0,0,1),a});}
 CollisionHitList TraceExtent(const vec3&,const vec3&,const vec3&,bool,bool,bool);
};
'''+body+r'''
int main(){TraceTester t;auto hits=t.TraceExtent({0,0,0},{0,0,-8},{2,7,5},true,true,false);assert(hits.size()==1&&hits[0].Fraction==.375f);assert(t.Seen.y==7&&TraceAABBModel::Extent.y==7);t.Actor.Collision.CheckCounter=0;t.World=false;assert(t.TraceExtent({0,0,0},{0,0,-8},{2,7,5},true,true,false).size()==1);assert(t.TraceExtent({0,0,0},{0,0,-8},{2,NAN,5},true,true,false).empty());}
''')
 def test_production_box_placement_uses_width(self):
  body=definition((ENGINE/'Packages/Engine/Actors/UActor_Phys.cpp').read_text(),'std::pair<bool, vec3> UActor::CheckLocationBox(')
  self.run_cpp(r'''#include "Math/vec.h"
#include <utility>
#include <vector>
#include <algorithm>
#include <cassert>
using CollisionHitList=std::vector<int>;
struct Scene{struct{int Calls=0;CollisionHitList OverlapExtent(vec3 p,vec3 e,bool actors,bool world,bool vis){assert(e.x==2&&e.y==7&&e.z==5&&!actors&&world&&!vis);++Calls;return p.x==7?CollisionHitList{}:CollisionHitList{1};}}Collision;};
struct UActor{Scene S;Scene*XLevel(){return &S;}std::pair<bool,vec3>CheckLocationBox(vec3,const vec3&,bool);};
'''+body+r'''
int main(){UActor a;auto p=a.CheckLocationBox(vec3(0.0f),{2,7,5},true);assert(p.first&&p.second.x==7&&a.S.Collision.Calls==2);int count=a.S.Collision.Calls;assert(a.CheckLocationBox(vec3(0.0f),{2,7,5},false).first&&a.S.Collision.Calls==count);}
''')
 def test_production_overlap_query_classification(self):
  body=definition((ENGINE/'Collision/TopLevel/OverlapTest.cpp').read_text(),'CollisionHitList OverlapTester::TestOverlapExtent(')
  self.run_cpp(r'''#include "Math/vec.h"
#include "Package/PoaWorldCollision.h"
#include <vector>
#include <algorithm>
#include <cassert>
struct UActor{virtual~UActor()=default;bool World=false,Collide=true;struct{int CheckCounter=0;}Collision;bool HasProperty(const char*){return true;}bool GetBool(const char*){return World;}bool bCollideActors(){return Collide;}};struct UMover:UActor{};struct UObject{template<class T>static T*TryCast(UActor*a){return dynamic_cast<T*>(a);}};
struct Mesh{};UActor*MeshActor=nullptr;Mesh mesh;Mesh*GetPoaStaticMesh(UActor*a){return a==MeshActor?&mesh:nullptr;}
struct Hit{float Fraction;vec3 Normal;UActor*Actor;void*A;void*B;};struct CollisionHitList:std::vector<Hit>{using std::vector<Hit>::push_back;void push_back(const CollisionHitList&h){insert(end(),h.begin(),h.end());}};
void TracePoaStaticMeshExtent(UActor*a,Mesh*,dvec3,double,dvec3,double,dvec3 e,CollisionHitList&h){assert(e.y==7);h.push_back({0,vec3(0.0f),a,nullptr,nullptr});}
struct Model{std::vector<int>Nodes{1};};struct Level{struct Model*Model;};struct OverlapAABBModel{CollisionHitList TestOverlap(UActor*,Model*,vec3,vec3 e,bool){assert(e.y==7);return {};}};
struct Engine{struct{bool IsHarryPotter3(){return true;}}LaunchInfo;}eng;Engine*engine=&eng;
struct OverlapTester{std::vector<UActor*>Actors;vec3 Seen{0.0f};int Counter=0;Model M;Level L{&M};int NextCheckCounter(){return ++Counter;}ivec3 GetStartExtents(vec3,vec3 e){Seen=e;return {0,0,0};}ivec3 GetEndExtents(vec3,vec3){return {1,1,1};}auto GetActors(int,int,int){return Actors;}Level*GetLevel(){return &L;}UActor*GetLevelInfo(){return nullptr;}static bool BoxActorOverlap(dvec3,dvec3 e,UActor*){assert(e.y==7);return true;}CollisionHitList TestOverlapExtent(const vec3&,const vec3&,bool,bool,bool);};
'''+body+r'''
int main(){OverlapTester t;UActor a;MeshActor=&a;a.World=true;a.Collide=false;t.Actors={&a};assert(t.TestOverlapExtent(vec3(0.0f),{2,7,5},false,true,false).size()==1&&t.Seen.y==7);assert(t.TestOverlapExtent(vec3(0.0f),{2,7,5},true,false,false).empty());a.World=false;a.Collide=true;assert(t.TestOverlapExtent(vec3(0.0f),{2,7,5},true,false,false).size()==1);assert(t.TestOverlapExtent(vec3(0.0f),{2,7,5},false,true,false).empty());MeshActor=nullptr;assert(t.TestOverlapExtent(vec3(0.0f),{2,7,5},true,false,false).size()==1);assert(t.TestOverlapExtent(vec3(0.0f),{2,NAN,5},true,false,false).empty());}
''')
if __name__=='__main__':unittest.main()
