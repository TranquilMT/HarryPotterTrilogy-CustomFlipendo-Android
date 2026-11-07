"""Exercise production tagged-struct reading on EU CompanionCombatManager defaults."""
import json, os, pathlib, subprocess, tempfile, unittest

def method(text, signature):
    start=text.index(signature);brace=text.index('{',start);depth=1;end=brace+1
    while depth:
        depth += (text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]

class RangeDefaults(unittest.TestCase):
    def test_eu_companion_defaults(self):
        game=os.environ.get('HP3_EU_GAME')
        if not game:self.skipTest('HP3_EU_GAME required for original EU regression')
        from inspect_packages import inspect,Reader
        from object_payloads import tags
        path=next(p for p in pathlib.Path(game).rglob('*') if p.name.lower()=='kwgame.u')
        model=inspect(path);self.assertEqual(model['sha256'],'de29aa642969ff5586559580fe05f29df6ca7fffa215dd50156d3be7e0f9b11e')
        entry=next(e for e in model['exports'] if e['name']=='CompanionCombatManager' and e['class']=='Class')
        raw=path.read_bytes()[entry['offset']:entry['offset']+entry['size']]
        candidates=[]
        for start in range(len(raw)):
            try:
                r=Reader(raw,start);rows=tags(r,model['names'])
                if r.p==len(raw) and rows and rows[0]['name']=='EnemyLoc_DestPositionRange':candidates.append((start,rows))
            except ValueError:pass
        self.assertEqual(len(candidates),1);start,rows=candidates[0]
        self.assertEqual(rows[0]['size'],55)
        # Reproduce the old flat RangeVector reader, including the exact misleading
        # unknown-property log followed by the out-of-range name index.
        r=Reader(raw,rows[0]['offset']+24)
        self.assertEqual(model['names'][r.compact()],'SerializeFrom')
        self.assertEqual(r.read(1),b'\0');r.read(1)
        self.assertFalse(0<=r.compact()<len(model['names']))
        root=pathlib.Path(__file__).resolve().parents[2]
        obj=(root/'src/engine/SurrealEngine/Packages/Core/UObject.cpp').read_text()
        prop=(root/'src/engine/SurrealEngine/Packages/Core/Properties/UStructProperty.cpp').read_text()
        bodies='\n'.join([method(obj,'void PropertyDataBlock::LoadStruct('),method(prop,'void UStructProperty::LoadValue('),method(prop,'void UStructProperty::LoadStructMemberValue('),method(obj,'void PropertyDataBlock::SaveStruct('),method(prop,'void UStructProperty::SaveHeader('),method(prop,'void UStructProperty::SaveValue('),method(prop,'void UStructProperty::SaveStructMemberValue(')])
        harness=r'''
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "Package/CompactIndex.h"
struct NameString:std::string{using std::string::string;NameString():std::string("None"){}NameString(std::string s):std::string(std::move(s)){}std::string ToString()const{return *this;}};
struct Exception{[[noreturn]]static void Throw(std::string s){throw std::runtime_error(s);}};
void LogMessage(std::string){}
enum UnrealPropertyType{UPT_Invalid,UPT_Byte,UPT_Int,UPT_Bool,UPT_Float,UPT_Object,UPT_Name,UPT_String,UPT_Class,UPT_Array,UPT_Struct};
struct PropertyHeader{UnrealPropertyType type;NameString structName;uint32_t size=0;int arrayIndex=0;bool boolValue=false;};
namespace ObjectFlags{constexpr int TagExp=1;}
struct ObjectStream{std::vector<uint8_t>d;size_t pos=0;std::vector<NameString>names;
 uint8_t ReadUInt8(){if(pos>=d.size())throw std::runtime_error("truncated");return d[pos++];}uint8_t ReadInt8(){return ReadUInt8();}
 uint16_t ReadUInt16(){auto a=ReadUInt8();return a|(uint16_t(ReadUInt8())<<8);}uint32_t ReadUInt32(){auto a=ReadUInt16();return a|(uint32_t(ReadUInt16())<<16);}
 int ReadIndex(){return PackageIndices::Read([&]{return ReadUInt8();});}NameString ReadName(){int i=ReadIndex();if(i<0||size_t(i)>=names.size())throw std::runtime_error("bad name");return names[i];}
 void Skip(uint32_t n){if(n>d.size()-pos)throw std::runtime_error("skip");pos+=n;}uint32_t Tell(){return pos;}
 float ReadFloat(){uint32_t x=ReadUInt32();float v;std::memcpy(&v,&x,4);return v;}};
struct PackageStreamWriter{std::vector<uint8_t>d;std::vector<NameString>names;struct Frame{std::vector<uint8_t>data;NameString name;};std::vector<Frame>frames;
 void WriteUInt8(uint8_t b){d.push_back(b);}void WriteFloat(float v){uint8_t b[4];std::memcpy(b,&v,4);d.insert(d.end(),b,b+4);}
 void WriteName(const NameString& n){auto i=std::find(names.begin(),names.end(),n);if(i==names.end())throw std::runtime_error("missing output name");unsigned v=i-names.begin();uint8_t first=v&63;v>>=6;WriteUInt8(first|(v?64:0));while(v){uint8_t b=v&127;v>>=7;WriteUInt8(b|(v?128:0));}}
 void BeginProperty(const NameString&name){frames.push_back({std::move(d),name});d.clear();}
 void EndProperty(const PropertyHeader&h){auto payload=std::move(d);auto frame=std::move(frames.back());frames.pop_back();d=std::move(frame.data);WriteName(frame.name);uint8_t code=payload.size()==1?0:payload.size()==2?1:payload.size()==4?2:payload.size()==12?3:payload.size()==16?4:5;if(h.type==UPT_Bool)code=0;WriteUInt8(h.type|(code<<4)|((h.type==UPT_Bool&&h.boolValue)?128:0));if(h.type==UPT_Struct)WriteName(h.structName);if(code==5){assert(payload.size()<256);WriteUInt8(payload.size());}d.insert(d.end(),payload.begin(),payload.end());}
};
namespace PropertyFlags{constexpr int Native=1,Transient=2;}bool AnyFlags(int a,int b){return (a&b)!=0;}
struct UStruct;struct UProperty{virtual~UProperty()=default;NameString Name;struct{size_t DataOffset=0;}DataOffset;int ArrayDimension=1,Flags=0,PropFlags=0;virtual size_t ElementPitch(){return 4;}virtual void SaveHeader(void*,PropertyHeader&)=0;virtual void SaveValue(void*,PackageStreamWriter*)=0;virtual void SaveStructMemberValue(void*d,PackageStreamWriter*s){SaveValue(d,s);}virtual void LoadValue(void*,ObjectStream*,const PropertyHeader&)=0;virtual void LoadStructMemberValue(void*,ObjectStream*)=0;};
struct UStruct{NameString Name;size_t StructSize=0;std::vector<UProperty*>Properties;};
struct UObject{template<class T>static T*TryCast(UProperty*p){return dynamic_cast<T*>(p);}};
struct PropertyDataBlock{static void LoadStruct(void*,UStruct*,ObjectStream*);static void SaveStruct(void*,UStruct*,PackageStreamWriter*);};
struct LaunchInfo{bool poa=true;bool IsHarryPotter3(){return poa;}};struct Engine{struct LaunchInfo LaunchInfo;}instance;Engine*engine=&instance;
struct UStructProperty:UProperty{void SaveHeader(void*,PropertyHeader&)override;void SaveValue(void*,PackageStreamWriter*)override;void SaveStructMemberValue(void*,PackageStreamWriter*)override;UStruct*Struct=nullptr;void ThrowIfTypeMismatch(const PropertyHeader&h,UnrealPropertyType t){if(h.type!=t)throw std::runtime_error("type");}void LoadValue(void*,ObjectStream*,const PropertyHeader&)override;void LoadStructMemberValue(void*,ObjectStream*)override;size_t ElementPitch()override{return 24;}};
struct FloatProperty:UProperty{void SaveHeader(void*,PropertyHeader&h)override{h.type=UPT_Float;}void SaveValue(void*d,PackageStreamWriter*s)override{s->WriteFloat(*static_cast<float*>(d));}void LoadValue(void*d,ObjectStream*s,const PropertyHeader&h)override{if(h.type!=UPT_Float)throw std::runtime_error("float type");LoadStructMemberValue(d,s);}void LoadStructMemberValue(void*d,ObjectStream*s)override{*static_cast<float*>(d)=s->ReadFloat();}};
struct BoolProperty:UProperty{void SaveHeader(void*d,PropertyHeader&h)override{h.type=UPT_Bool;h.boolValue=*static_cast<bool*>(d);}void SaveValue(void*,PackageStreamWriter*)override{}void LoadValue(void*d,ObjectStream*,const PropertyHeader&h)override{if(h.type!=UPT_Bool)throw std::runtime_error("bool type");*static_cast<bool*>(d)=h.boolValue;}void LoadStructMemberValue(void*,ObjectStream*)override{throw std::runtime_error("unexpected binary bool");}};
''' + bodies + r'''
int main(int argc,char**argv){
 std::ifstream f(argv[1],std::ios::binary);ObjectStream stream;stream.d={std::istreambuf_iterator<char>(f),{}};
 stream.names={
''' + ','.join(json.dumps(n) for n in model['names']) + r'''};
 FloatProperty min,max;min.Name="Min";max.Name="Max";max.DataOffset.DataOffset=4;UStruct range;range.Name="Range";range.StructSize=8;range.Properties={&min,&max};
 UStructProperty x,y,z;x.Name="X";y.Name="Y";z.Name="Z";x.Struct=y.Struct=z.Struct=&range;y.DataOffset.DataOffset=8;z.DataOffset.DataOffset=16;
 UStruct rangeVector;rangeVector.Name="RangeVector";rangeVector.StructSize=24;rangeVector.Properties={&x,&y,&z};UStructProperty ranges;ranges.Name="EnemyLoc_DestPositionRange";ranges.Struct=&rangeVector;
 FloatProperty leave,distance;leave.Name="LeadCharLeaveFightRadius";leave.DataOffset.DataOffset=24;distance.Name="AutoStopFightStanceAnimsDistance";distance.DataOffset.DataOffset=32;
 BoolProperty stance;stance.Name="bUseFightStanceAnimsForCombat";stance.DataOffset.DataOffset=28;
 UStruct companion;companion.Name="CompanionCombatManager";companion.StructSize=36;companion.Properties={&ranges,&leave,&stance,&distance};
 alignas(float) uint8_t data[36]={};PropertyDataBlock::LoadStruct(data,&companion,&stream);assert(stream.pos==stream.d.size());assert(*reinterpret_cast<bool*>(data+28));
 assert(*reinterpret_cast<float*>(data+24)>0);assert(*reinterpret_cast<float*>(data+32)>0);
 PackageStreamWriter writer;writer.names=stream.names;PropertyDataBlock::SaveStruct(data,&companion,&writer);ObjectStream roundtrip;roundtrip.d=writer.d;roundtrip.names=stream.names;alignas(float) uint8_t restored[36]={};PropertyDataBlock::LoadStruct(restored,&companion,&roundtrip);assert(roundtrip.pos==roundtrip.d.size());assert(std::memcmp(data,restored,36)==0);
 ObjectStream invalid=stream;invalid.pos=0;ranges.DataOffset.DataOffset=36;bool rejected=false;try{PropertyDataBlock::LoadStruct(data,&companion,&invalid);}catch(const std::exception&){rejected=true;}assert(rejected);ranges.DataOffset.DataOffset=0;
 // UE1 Range remains a flat two-float struct; native Vector is also flat in PoA.
 engine->LaunchInfo.poa=false;ObjectStream legacy;float values[2]={2.5f,7.0f};legacy.d.resize(8);std::memcpy(legacy.d.data(),values,8);float result[2]={};x.LoadStructMemberValue(result,&legacy);assert(result[0]==2.5f&&result[1]==7&&legacy.pos==8);
 engine->LaunchInfo.poa=true;FloatProperty vx,vy,vz;vx.Name="X";vy.Name="Y";vz.Name="Z";vy.DataOffset.DataOffset=4;vz.DataOffset.DataOffset=8;UStruct vector;vector.Name="Vector";vector.StructSize=12;vector.Properties={&vx,&vy,&vz};UStructProperty vectorProp;vectorProp.Struct=&vector;float xyz[3]={1,2,3};ObjectStream native;native.d.resize(12);std::memcpy(native.d.data(),xyz,12);float nativeResult[3]={};vectorProp.LoadStructMemberValue(nativeResult,&native);assert(native.pos==12&&nativeResult[2]==3);
 std::cout<<"PASS production EU CompanionCombatManager nested RangeVector defaults and legacy flat Range\n";
}
'''
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td);(p/'test.cpp').write_text(harness);(p/'defaults.bin').write_bytes(raw[start:])
            subprocess.run(['g++','-std=c++20','-I'+str(root/'src/engine/SurrealEngine'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test'),str(p/'defaults.bin')],check=True)

if __name__=='__main__':unittest.main()
