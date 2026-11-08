"""Production stack loader preserves the actual state apart from code owner."""
import pathlib,tempfile,subprocess,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class SavedStateScope(unittest.TestCase):
 def test_poa_inherited_code_scope_and_latent_restore(self):
  body=definition((ENGINE/'Packages/Core/UObject.cpp').read_text(),'void UObject::Load(')
  code=r'''#include <memory>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>
#include <type_traits>
using NameString=std::string;class UClass;class ObjectStream;class UStruct;class UState;struct Frame;
struct Properties{void Init(UClass*){}void Load(ObjectStream*){}};
enum class ObjectFlags{HasStack=1};bool AllFlags(ObjectFlags a,ObjectFlags b){return (int(a)&int(b))==int(b);}
enum class LatentRunState{Continue,FinishAnim,Stop};enum class EventName{};NameString ToNameString(EventName){return "event";}void LogMessage(const char*){}
struct UObject{virtual~UObject()=default;UClass*Class=nullptr;ObjectFlags Flags=ObjectFlags::HasStack;std::shared_ptr<Frame>StateFrame;Properties PropertyData;NameString Name;void LoadNow(){}void Load(ObjectStream*);template<class T>static T*TryCast(UObject*p){return dynamic_cast<T*>(p);}void DisableEvent(NameString){}void SetObject(const char*,UObject*){}void SetName(const char*,NameString){}void SetInt(const char*,int){}};
struct Code{int FindStatementIndex(int index){return index;}};struct UStruct:UObject{struct Code*Code=nullptr;};struct UState:UStruct{};struct UClass:UState{std::vector<int>Properties;};
struct Frame{UStruct*Function;UStruct*State;LatentRunState Latent=LatentRunState::Continue;Frame(UObject*,UStruct*function,int,bool,UStruct*state=nullptr):Function(function),State(state?state:function){}void SetLatentState(LatentRunState state){Latent=state;}};
namespace NativeFunctions{std::vector<LatentRunState>LatentActionByIndex={LatentRunState::Continue,LatentRunState::FinishAnim};}
struct Profile{bool HP2=false,HP3=true;bool IsHarryPotter2(){return HP2;}bool IsHarryPotter3(){return HP3;}};struct Engine{Profile LaunchInfo;};Engine instance;Engine*engine=&instance;
struct ObjectStream{UStruct*Function=nullptr;UState*State=nullptr;template<class T>T*ReadObject(){if constexpr(std::is_same_v<T,UState>)return State;else return Function;}int64_t ReadInt64(){return -1;}int32_t ReadInt32(){return 1;}int ReadIndex(){return 0;}};
'''+body+r'''
int main(){Code code;UStruct inheritedFunction;inheritedFunction.Code=&code;UState derivedState;ObjectStream stream{&inheritedFunction,&derivedState};UObject actor;actor.Load(&stream);assert(actor.StateFrame->Function==&inheritedFunction&&actor.StateFrame->State==&derivedState&&actor.StateFrame->Latent==LatentRunState::FinishAnim);
 instance.LaunchInfo.HP3=false;actor.Load(&stream);assert(actor.StateFrame->State==&inheritedFunction);instance.LaunchInfo.HP2=true;actor.Load(&stream);assert(actor.StateFrame->State==&derivedState);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
