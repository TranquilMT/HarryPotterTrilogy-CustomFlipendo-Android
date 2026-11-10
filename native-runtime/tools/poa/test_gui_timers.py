"""Execute GUI timer production bodies with reentrant script callback adapters."""
import os,pathlib,subprocess,tempfile,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
class GuiTimers(unittest.TestCase):
 def test_timer_boundary_repeat_rearm_and_owner_lifetime(self):
  source=(ROOT/'src/hp3/PoaGuiTimers.cpp').read_text();source='\n'.join(l for l in source.splitlines()if not l.startswith('#include'));source=source.replace(definition(source,'void RegisterNatives()'),'')
  code=r'''#include <string>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include <limits>
struct UObject;enum class ExpressionValueType{ValueObject,Other};struct Inner{ExpressionValueType ValueType=ExpressionValueType::ValueObject;size_t ElementSize(){return sizeof(UObject*);}};
struct UArrayProperty{struct Inner*Inner;};struct ScriptArray{std::vector<UObject*>Items;size_t GetSize(){return Items.size();}void*GetItem(size_t n){return &Items.at(n);}void Resize(size_t n){Items.resize(n);}void Remove(size_t n,size_t count){Items.erase(Items.begin()+n,Items.begin()+n+count);}};
struct UObject{bool IsPage=false,Repeat=false;UObject*Owner=nullptr;float Countdown=0,Interval=0;int Index=-1,Calls=0;struct Inner Type;UArrayProperty Property{&Type};ScriptArray TimerArray;std::function<void()>Hook;
bool IsA(const char*){return IsPage;}bool HasProperty(const char*){return true;}UObject*GetUObject(const char*){return Owner;}UArrayProperty*GetMemberProperty(const char*){return &Property;}void*GetProperty(UArrayProperty*){return &TimerArray;}
float GetFloat(const char*n){return std::string(n)=="TimerCountdown"?Countdown:Interval;}void SetFloat(const char*n,float f){(std::string(n)=="TimerCountdown"?Countdown:Interval)=f;}bool GetBool(const char*){return Repeat;}void SetBool(const char*,bool b){Repeat=b;}void SetInt(const char*,uint32_t i){Index=int32_t(i);}template<class T>static T*TryCast(UArrayProperty*p){return static_cast<T*>(p);}};
int Roots=0;template<class T>struct GCRoot{T*Value;GCRoot(T*p):Value(p){++Roots;}~GCRoot(){--Roots;}};
namespace Exception{void Throw(const char*s){throw std::runtime_error(s);}}
void CallEvent(UObject*o,const char*n){assert(std::string(n)=="Timer"&&Roots==2);++o->Calls;if(o->Hook)o->Hook();}
'''+source+r'''
int main(){using namespace PoaGuiTimers;UObject page,a,b,c;page.IsPage=true;a.Owner=&page;b.Owner=&page;c.Owner=&a;
SetTimer(&a,1,false);assert(page.TimerArray.GetSize()==1&&a.Index==0&&!a.Repeat);Update(&page,1);assert(a.Calls==0&&a.Countdown==0);Update(&page,.1f);assert(a.Calls==1&&a.Interval==0&&a.Index==-1&&page.TimerArray.GetSize()==0&&Roots==0);
SetTimer(&a,1,true);Update(&page,1.25f);assert(a.Calls==2&&a.Countdown==.75f&&a.Repeat);Update(&page,9);assert(a.Calls==3&&a.Countdown==1&&page.TimerArray.GetSize()==1);
SetTimer(&a,2,true);assert(page.TimerArray.GetSize()==1);KillTimer(&a);assert(a.Index==0&&a.Interval==0&&!a.Repeat);Update(&page,0);assert(a.Index==-1&&page.TimerArray.GetSize()==0);
SetTimer(&a,.5f,true);a.Hook=[&](){KillTimer(&a);SetTimer(&b,.1f,false);};Update(&page,1);assert(a.Calls==4&&b.Calls==0&&page.TimerArray.GetSize()==1&&page.TimerArray.Items[0]==&b);Update(&page,.2f);assert(b.Calls==1&&page.TimerArray.GetSize()==0);a.Hook={};
SetTimer(&c,.5f,true);assert(page.TimerArray.Items[0]==&c);c.Hook=[&](){SetTimer(&c,2,true);};Update(&page,1);assert(c.Calls==1&&c.Countdown==4&&c.Interval==2);KillTimer(&c);Update(&page,0);
SetTimer(&a,.2f,true);SetTimer(&b,.2f,true);a.Hook=[&](){KillTimer(&b);};Update(&page,.3f);assert(b.Calls==1&&b.Index==-1&&page.TimerArray.GetSize()==1);a.Hook={};KillTimer(&a);Update(&page,0);
SetTimer(&a,.2f,true);a.Hook=[](){throw std::runtime_error("script");};bool threw=false;try{Update(&page,.3f);}catch(...){threw=true;}assert(threw&&Roots==0);a.Hook={};KillTimer(&a);Update(&page,0);
SetTimer(&a,-1,true);assert(a.Interval==-1&&!a.Repeat&&page.TimerArray.GetSize()==0);threw=false;try{SetTimer(&a,std::numeric_limits<float>::infinity(),true);}catch(...){threw=true;}assert(threw&&a.Interval==-1);
a.Owner=&c;threw=false;try{SetTimer(&a,1,true);}catch(...){threw=true;}assert(threw);a.Owner=nullptr;threw=false;try{SetTimer(&a,1,true);}catch(...){threw=true;}assert(threw);assert(Roots==0);
}
'''
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
