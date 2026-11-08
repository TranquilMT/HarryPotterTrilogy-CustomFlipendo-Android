import os,pathlib,tempfile,subprocess,unittest
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
class Tracing(unittest.TestCase):
 def test_production_trace_hook_and_callback_preservation(self):
  body=definition((ROOT/'src/engine/SurrealEngine/VM/Frame.cpp').read_text(),'void Frame::TraceCall(')
  code=r'''#include "hp3/PoaTracing.h"
#include <string>
#include <vector>
#include <functional>
#include <cassert>
struct NameString{std::string Text;std::string ToString(){return Text;}bool operator==(const NameString&n)const{return Text==n.Text;}};
struct UObject{NameString Name;};struct UFunction{NameString Name;int FuncFlags=0;};struct ExpressionValue{};template<class T>using ArrayView=std::vector<T>;
struct Trace{bool Enabled=true;NameString Function;std::function<void(UFunction*,UObject*,const ArrayView<ExpressionValue>&)>Callback;};
struct Frame{static std::vector<Trace>Traces;static void TraceCall(UFunction*,UObject*,const ArrayView<ExpressionValue>&);};std::vector<Trace>Frame::Traces;
namespace FunctionFlags{constexpr int Native=1;}bool AllFlags(int a,int b){return(a&b)==b;}
struct Launch{bool poa=true;bool IsHarryPotter3(){return poa;}};struct Engine{Launch LaunchInfo;}instance;Engine*engine=&instance;
std::vector<std::string>Logs;void LogMessage(const std::string&s){Logs.push_back(s);}
'''+body+r'''
int main(){UObject actor{{"Hero"}};UFunction script{{"Update"},0},native{{"Tick"},1};int callbacks=0;Frame::Traces.push_back({true,{"Update"},[&](UFunction*,UObject*,const auto&){++callbacks;}});
 PoaTracing::Set(true,false);Frame::TraceCall(&script,&actor,{});Frame::TraceCall(&native,&actor,{});assert(Logs.size()==1&&Logs[0]=="UTrace Hero.Update"&&callbacks==1);
 PoaTracing::Set(true,true);Frame::TraceCall(&native,&actor,{});assert(Logs.size()==2&&Logs.back()=="UTrace Hero.Tick");
 PoaTracing::Set(false,false);Frame::TraceCall(&script,&actor,{});assert(Logs.size()==2&&callbacks==2);
 engine->LaunchInfo.poa=false;PoaTracing::Set(true,true);Frame::TraceCall(&native,&actor,{});assert(Logs.size()==2);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src'),str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
