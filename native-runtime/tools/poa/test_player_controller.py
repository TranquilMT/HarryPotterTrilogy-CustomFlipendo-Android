"""Execute production controller tick with script and actor adapters."""
import os,pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class PlayerController(unittest.TestCase):
 def test_actor_inheritance_and_single_script_tick(self):
  source='\n'.join(l for l in (ROOT/'src/hp3/PoaPlayerController.h').read_text().splitlines()if not l.startswith('#'))
  code=r'''#include <cassert>
#include <string>
#include <initializer_list>
#include <type_traits>
struct UActor{virtual~UActor()=default;bool Deleted=false,Local=true;int Ticks=0;virtual void Tick(float){++Ticks;}bool bDeleteMe(){return Deleted;}bool HasProperty(const char*){return true;}UActor*GetUObject(const char*){return Local?this:nullptr;}};
struct ExpressionValue{float Float=0;static ExpressionValue FloatValue(float f){return{f};}};
int Calls=0;void CallEvent(UActor*,const char*n,std::initializer_list<ExpressionValue>args){assert(std::string(n)=="PlayerTick"&&args.size()==1&&args.begin()->Float==.25f);++Calls;}
'''+source+r'''
int main(){static_assert(std::is_base_of_v<UActor,UPoaController>);static_assert(std::is_base_of_v<UPoaController,UPoaPlayerController>);UPoaPlayerController c;c.Tick(.25f);assert(c.Ticks==1&&Calls==1);c.Local=false;c.Tick(.25f);assert(c.Ticks==2&&Calls==1);c.Local=true;c.Deleted=true;c.Tick(.25f);assert(c.Ticks==3&&Calls==1);}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
