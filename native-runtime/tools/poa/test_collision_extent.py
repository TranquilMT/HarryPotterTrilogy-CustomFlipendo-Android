"""EU shape dimensions and production static-mesh box-width propagation."""
import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
ENGINE=ROOT/'src/engine/SurrealEngine'
class CollisionExtent(unittest.TestCase):
 def test_dimensions_and_production_triangle_sweep(self):
  body='\n'.join(l for l in (ENGINE/'Collision/TopLevel/PoaStaticMeshTrace.h').read_text().splitlines() if not l.startswith('#'))
  code=r'''#include "Collision/BottomLevel/PoaCollisionExtent.h"
#include "Collision/BottomLevel/PoaSweepTriangle.h"
#include <vector>
#include <cassert>
#include <string>
struct UActor{bool Zero=true,Nonzero=true;bool HasProperty(const char*){return true;}bool GetBool(const char*s){return std::string(s)=="bBlockZeroExtentTraces"?Zero:Nonzero;}};
struct Vertex{float Position[3];};struct Section{size_t FirstIndex,Triangles;};
struct UPoaStaticMesh{struct{std::vector<Vertex>Vertices;std::vector<unsigned>Indices;std::vector<Section>Sections;}Geometry;std::vector<bool>MaterialCollision;};
struct Identity{vec4 operator*(vec4 v){return v;}};Identity PoaStaticMeshToWorld(UActor*){return {};}
struct Hit{float Fraction;vec3 Normal;UActor*Actor;void*A;void*B;};using CollisionHitList=std::vector<Hit>;
'''+body+r'''
int main(){
 auto c=PoaCollision::Extent(0,2,NAN,5);assert(c&&c->x==2&&c->y==2&&c->z==5);
 auto b=PoaCollision::Extent(1,2,3,5);assert(b&&b->y==3);
 assert(PoaCollision::Extent(1,2,0,5)->y==2);
 assert(!PoaCollision::Extent(2,2,3,5));assert(!PoaCollision::Extent(1,2,NAN,5));
 assert(!PoaCollision::Extent(1,2,-1,5));assert(!PoaCollision::Extent(0,-1,0,5));assert(!PoaCollision::Extent(0,2,0,INFINITY));
 UActor actor;UPoaStaticMesh mesh;mesh.Geometry.Vertices={{{-10,0,-10}},{{10,0,-10}},{{0,0,10}}};mesh.Geometry.Indices={0,1,2};mesh.Geometry.Sections={{0,1}};mesh.MaterialCollision={true};CollisionHitList hits;
 TracePoaStaticMeshExtent(&actor,&mesh,{0,-10,0},0,{0,1,0},20,{1,4,1},hits);assert(hits.size()==1&&std::abs(hits[0].Fraction-6)<1e-6&&hits[0].Normal.y<-.99);
 hits.clear();TracePoaStaticMesh(&actor,&mesh,{0,-10,0},0,{0,1,0},20,1,1,hits);assert(hits.size()==1&&std::abs(hits[0].Fraction-9)<1e-6);
 hits.clear();actor.Nonzero=false;TracePoaStaticMeshExtent(&actor,&mesh,{0,-10,0},0,{0,1,0},20,{0,4,0},hits);assert(hits.empty());
 actor.Nonzero=true;mesh.MaterialCollision[0]=false;TracePoaStaticMeshExtent(&actor,&mesh,{0,-10,0},0,{0,1,0},20,{1,4,1},hits);assert(hits.empty());
 mesh.MaterialCollision[0]=true;TracePoaStaticMeshExtent(&actor,&mesh,{0,-10,0},0,{0,1,0},20,{1,NAN,1},hits);assert(hits.empty());
}
'''
  with tempfile.TemporaryDirectory() as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ENGINE),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
