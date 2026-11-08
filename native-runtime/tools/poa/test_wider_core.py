"""Execute wider PoA native bodies with reflection/INI service test doubles."""
import pathlib,subprocess,tempfile,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2]
class WiderCore(unittest.TestCase):
 def test_quaternions_curves_and_ini(self):
  body=(ROOT/'src/hp3/PoaCompat.cpp').read_text();body=body[body.index('namespace PoaCompat'):];reg=definition(body,'void RegisterNatives()');body=body.replace(reg,'')
  code=r'''#include "hp3/PoaMath.h"
#include "hp3/PoaRotation.h"
#include "hp3/PoaTracing.h"
#include <optional>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <cstdint>
struct Exception{[[noreturn]]static void Throw(const char*s){throw std::runtime_error(s);}};
struct UProperty{virtual ~UProperty()=default;std::string Name;int ArrayDimension=1;struct{size_t DataOffset=0;}DataOffset;virtual size_t ElementPitch(){return 16;}};
struct UFloatProperty:UProperty{};
struct UStruct{std::vector<UProperty*>Properties;size_t StructSize=16;};
struct UStructProperty:UProperty{UStruct*Struct=nullptr;};
struct UObject{template<class T>static T*TryCast(UProperty*p){return dynamic_cast<T*>(p);}template<class T>static bool IsType(UProperty*p){return dynamic_cast<T*>(p)!=nullptr;}};
struct ScriptArray{UProperty*type=nullptr;std::vector<uint8_t>bytes;size_t count=0;size_t GetSize()const{return count;}UProperty*GetType()const{return type;}const void*GetData()const{return bytes.empty()?nullptr:bytes.data();}};
struct Packages{std::string ini,section,key,language="int";const std::string&GetLanguage(){return language;}std::string GetIniValue(const std::string&i,const std::string&s,const std::string&k,const std::string&){ini=i;section=s;key=k;return "value";}};
struct Engine{Packages*packages;};Packages packages;Engine eng{&packages};Engine*engine=&eng;
'''+body+r'''
bool near(float a,float b){return std::abs(a-b)<.0001f;}
int main(){
 quaternion converted;PoaCompat::QuatFromRotator({0,0,0},converted);Rotator convertedBack;PoaCompat::QuatToRotator(converted,convertedBack);assert(convertedBack.Pitch==0&&convertedBack.Yaw==0&&convertedBack.Roll==0);

 bool clockwise=false;PoaCompat::ClockwiseFrom_IntInt(0,0,clockwise);assert(!clockwise);PoaCompat::ClockwiseFrom_IntInt(32768,0,clockwise);assert(clockwise);PoaCompat::ClockwiseFrom_IntInt(0,32768,clockwise);assert(!clockwise);PoaCompat::ClockwiseFrom_IntInt(-1,0,clockwise);assert(!clockwise);PoaCompat::ClockwiseFrom_IntInt(0,-1,clockwise);assert(clockwise);PoaCompat::ClockwiseFrom_IntInt(INT32_MIN,INT32_MAX,clockwise);assert(clockwise);
 bool sap=false;packages.language="INT";PoaCompat::NeedSAPFont(sap);assert(!sap);packages.language="rus";PoaCompat::NeedSAPFont(sap);assert(sap);packages.language="int";
 bool tracing=false;PoaCompat::SetUTracing(true,std::nullopt);PoaCompat::IsUTracing(tracing);assert(tracing&&PoaTracing::ShouldTrace(false)&&!PoaTracing::ShouldTrace(true));PoaCompat::SetUTracing(true,true);assert(PoaTracing::ShouldTrace(true));PoaCompat::SetUTracing(false,true);PoaCompat::IsUTracing(tracing);assert(!tracing&&!PoaTracing::ShouldTrace(false));
using namespace PoaCompat;
 quaternion q{0,0,std::sqrt(.5f),std::sqrt(.5f)},r;QuatInvert(q,r);auto identity=PoaCore::Product(q,r);assert(near(identity.w,1)&&near(identity.z,0));
 for(auto target:{vec3{0,1,0},vec3{-1,0,0},vec3{1,0,0}}){QuatFindBetween({2,0,0},target,r);auto v=PoaCore::Rotate(r,{1,0,0});assert(near(v.x,target.x)&&near(v.y,target.y)&&near(v.z,target.z));}
 QuatFindBetween({0,0,0},{1,0,0},r);assert(r.w==1&&r.x==0&&r.y==0&&r.z==0);
 InterpCurve curve;float minimum=99,maximum=99;InterpCurveGetOutputRange(curve,minimum,maximum);assert(minimum==0&&maximum==0);
 UFloatProperty in,out;in.Name="InVal";out.Name="OutVal";in.DataOffset.DataOffset=8;out.DataOffset.DataOffset=0;UStruct point;point.Properties={&out,&in};UStructProperty property;property.Struct=&point;curve.Points.type=&property;curve.Points.count=3;curve.Points.bytes.resize(48);
 float inputs[]={4,-2,10},outputs[]={-5,20,7};for(int i=0;i<3;++i){std::memcpy(curve.Points.bytes.data()+16*i+8,&inputs[i],4);std::memcpy(curve.Points.bytes.data()+16*i,&outputs[i],4);}
 InterpCurveGetInputDomain(curve,minimum,maximum);assert(minimum==-2&&maximum==10);InterpCurveGetOutputRange(curve,minimum,maximum);assert(minimum==-5&&maximum==20);
 out.DataOffset.DataOffset=16;bool rejected=false;try{InterpCurveGetOutputRange(curve,minimum,maximum);}catch(const std::exception&){rejected=true;}assert(rejected);out.DataOffset.DataOffset=0;
 point.Properties={&in};rejected=false;try{InterpCurveGetOutputRange(curve,minimum,maximum);}catch(const std::exception&){rejected=true;}assert(rejected);
 std::string value;GetIniEntry("Section","Key","User.INI",value);assert(value=="value"&&packages.ini=="User"&&packages.section=="Section"&&packages.key=="Key");GetIniEntry("S","K","",value);assert(packages.ini=="system");GetIniEntry("S","K","My.Options",value);assert(packages.ini=="My.Options");
}
'''
  with tempfile.TemporaryDirectory() as tmp:
   p=pathlib.Path(tmp);(p/'test.cpp').write_text(code);subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(ROOT/'src'),'-I'+str(ROOT/'src/engine/SurrealEngine'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
