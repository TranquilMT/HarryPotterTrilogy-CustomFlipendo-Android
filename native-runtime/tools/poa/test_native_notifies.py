"""Production notify dispatcher with script/audio/object adapters."""
import pathlib,tempfile,subprocess,unittest,os
ROOT=pathlib.Path(__file__).resolve().parents[2]
class NativeNotifies(unittest.TestCase):
 def test_script_sound_dispatch_parameters_and_unknown_types(self):
  body='\n'.join(line for line in (ROOT/'src/hp3/PoaNotifies.cpp').read_text().splitlines()if not line.startswith('#include'))
  code=r'''#include <string>
#include <map>
#include <vector>
#include <optional>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>
struct NameString{std::string Text;NameString(const char*s="None"):Text(s){}NameString(std::string s):Text(s){}bool IsNone()const{return Text=="None";}std::string ToString()const{return Text;}};
struct UFunction{int NumParms=0;uint32_t FuncFlags=0;};struct UObject{virtual~UObject()=default;std::string Type;std::map<std::string,UObject*>Objects;std::map<std::string,NameString>Names;std::map<std::string,float>Floats;std::map<std::string,int>Ints;std::map<std::string,UFunction*>Functions;void LoadNow(){}bool IsA(const char*t){return Type==t;}NameString GetName(const char*n){return Names[n];}UObject*GetUObject(const char*n){return Objects[n];}float GetFloat(const char*n){return Floats[n];}int GetInt(const char*n){return Ints[n];}};struct UActor:UObject{bool Deleted=false;bool bDeleteMe(){return Deleted;}};
struct ExpressionValue{UObject*Object;static ExpressionValue ObjectValue(UObject*p){return {p};}};struct Event{UObject*Context;std::string Name;std::vector<ExpressionValue>Args;};std::vector<Event>Events;
UFunction*FindEventFunction(UObject*p,const NameString&name){auto found=p->Functions.find(name.Text);return found==p->Functions.end()?nullptr:found->second;}
void CallEvent(UObject*p,const NameString&name,std::vector<ExpressionValue>args={}){Events.push_back({p,name.Text,args});}
template<class T>struct GCRoot{GCRoot(T*){}};struct Exception{[[noreturn]]static void Throw(const char*s){throw std::runtime_error(s);}};constexpr uint8_t SLOT_None=0;
struct NActor{static int Calls;static float Volume,Radius,Pitch;static uint8_t Slot;static UObject*Sound;static UObject*Owner;static void PlaySound(UObject*owner,UObject*sound,std::optional<uint8_t>slot,std::optional<float>volume,std::optional<bool>override,std::optional<float>radius,std::optional<float>pitch){++Calls;Owner=owner;Sound=sound;Slot=*slot;Volume=*volume;Radius=*radius;Pitch=*pitch;assert(override&&!*override);}};int NActor::Calls=0;float NActor::Volume=0,NActor::Radius=0,NActor::Pitch=0;uint8_t NActor::Slot=99;UObject*NActor::Sound=nullptr;UObject*NActor::Owner=nullptr;
'''+body+r'''
int main(){using namespace PoaNotifies;UActor actor;UObject scripted,soundNotify,sound,unknown;UFunction function;scripted.Functions["Notify"]=&function;assert(Dispatch(&scripted,&actor)&&Events.size()==1&&Events.back().Context==&scripted&&Events.back().Args[0].Object==&actor);
 scripted.Functions.clear();scripted.Type="AnimNotify_Script";scripted.Names["NotifyName"]="Footstep";actor.Functions["Footstep"]=&function;assert(Dispatch(&scripted,&actor)&&Events.size()==2&&Events.back().Context==&actor&&Events.back().Args.empty());
 function.NumParms=1;bool rejected=false;try{Dispatch(&scripted,&actor);}catch(...){rejected=true;}assert(rejected);function.NumParms=0;scripted.Names["NotifyName"]="None";assert(Dispatch(&scripted,&actor)&&Events.size()==2);
 soundNotify.Type="AnimNotify_Sound";soundNotify.Objects["Sound"]=&sound;soundNotify.Floats["Volume"]=.75;soundNotify.Ints["Radius"]=0;assert(Dispatch(&soundNotify,&actor)&&NActor::Calls==1&&NActor::Owner==&actor&&NActor::Sound==&sound&&NActor::Slot==0&&NActor::Volume==.75&&NActor::Radius==80&&NActor::Pitch==1);
 soundNotify.Ints["Radius"]=120;Dispatch(&soundNotify,&actor);assert(NActor::Radius==120);soundNotify.Objects["Sound"]=nullptr;Dispatch(&soundNotify,&actor);assert(NActor::Calls==2);assert(!Dispatch(&unknown,&actor));actor.Deleted=true;assert(Dispatch(&unknown,&actor));
}
'''
  with tempfile.TemporaryDirectory()as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(t/'test.cpp'),'-o',str(t/'test')],check=True);subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
