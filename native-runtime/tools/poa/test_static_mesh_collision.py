import os
import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class StaticCollision(unittest.TestCase):
 def test_swept_boxes_rays_and_degenerate_triangles(self):
  code=r'''#include "Collision/BottomLevel/PoaSweepTriangle.h"
#include <cassert>
int main(){dvec3 tri[3]={{-10,-10,0},{10,-10,0},{0,10,0}},normal;
 auto ray=PoaSweepTriangle({0,0,5},{0,0,-1},0,10,{0,0,0},tri,normal);assert(std::abs(ray-5)<1e-9&&normal.z>0.999);
 auto box=PoaSweepTriangle({0,0,5},{0,0,-1},0,10,{1,1,2},tri,normal);assert(std::abs(box-3)<1e-9&&normal.z>0.999);
 assert(PoaSweepTriangle({30,30,5},{0,0,-1},0,10,{1,1,2},tri,normal)==10);
 assert(PoaSweepTriangle({0,0,0},{0,0,1},0,10,{0,0,0},tri,normal)==10);
 assert(PoaSweepTriangle({0,0,1},{0,0,0},0,1,{1,1,2},tri,normal)==0);
 assert(PoaSweepTriangle({0,0,4},{0,0,0},0,1,{1,1,2},tri,normal)==1);
 dvec3 bad[3]={{0,0,0},{0,0,0},{0,0,0}};assert(PoaSweepTriangle({0,0,5},{0,0,-1},0,10,{1,1,2},bad,normal)==10);
 dvec3 wall[3]={{0,-10,-10},{0,10,-10},{0,0,10}};
 assert(std::abs(PoaSweepTriangle({-5,0,0},{1,0,0},0,10,{2,1,1},wall,normal)-3)<1e-9&&normal.x<-.999);
 // Reversed winding and world vertices with negative/nonuniform scale.
 std::swap(tri[0],tri[1]);for(auto&p:tri){p.x*=-2;p.y*=3;p.z+=2;}
 assert(std::abs(PoaSweepTriangle({0,0,10},{0,0,-1},0,20,{1,1,2},tri,normal)-6)<1e-9&&normal.z>.999);
 assert(PoaSweepTriangle({0,0,10},{0,0,-1},0,5,{1,1,2},tri,normal)==5);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src/engine/SurrealEngine'),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
if __name__=='__main__':unittest.main()
