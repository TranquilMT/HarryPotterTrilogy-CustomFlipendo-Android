"""Compile production font load/save/glyph methods; optional EU corpus roundtrip."""
import os,pathlib,subprocess,tempfile,unittest
class ProductionFonts(unittest.TestCase):
 def test_production_font_reader(self):
  root=pathlib.Path(__file__).resolve().parents[2]
  source=(root/'src/engine/SurrealEngine/Packages/Engine/Resources/UFont.cpp').read_text()
  methods=source[source.index('void UFont::Load('):source.index('FontGlyph (*UFont::ExtraGlyph)')]
  methods+=source[source.index('FontGlyph UFont::FindGlyph('):source.index('std::u32string UFont::Codepoints(')]
  harness=r'''
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <cassert>
#include <iostream>
using std::size_t;
struct Exception{[[noreturn]]static void Throw(const std::string&s){throw std::runtime_error(s);}};
struct Info{bool poa=false;bool IsHarryPotter3(){return poa;}};
struct Engine{Info LaunchInfo;} instance;Engine*engine=&instance;
struct UTexture;
struct ObjectStream{
 std::vector<uint8_t>d;size_t p=0;int version=129;std::unordered_map<int,UTexture*>textures;
 uint8_t ReadUInt8(){if(p>=d.size())throw std::runtime_error("truncated");return d[p++];}
 uint16_t ReadUInt16(){uint16_t a=ReadUInt8();return a|(uint16_t(ReadUInt8())<<8);}
 uint32_t ReadUInt32(){uint32_t a=ReadUInt16();return a|(uint32_t(ReadUInt16())<<16);}
 int32_t ReadInt32(){return int32_t(ReadUInt32());}
 int ReadIndex(){uint8_t c=ReadUInt8();bool neg=c&128;int n=c&63,shift=6;for(int k=0;c&64 && k==0;){c=ReadUInt8();n|=(c&127)<<shift;shift+=7;while(c&128){c=ReadUInt8();n|=(c&127)<<shift;shift+=7;}break;}return neg?-n:n;}
 int GetVersion(){return version;}
 template<class T>T*ReadObject(){int ref=ReadIndex();if(!ref)return nullptr;auto it=textures.find(ref);if(it==textures.end())throw std::runtime_error("not a texture reference");return static_cast<T*>(it->second);}
 void WriteUInt8(uint8_t x){d.push_back(x);}void WriteUInt16(uint16_t x){WriteUInt8(x);WriteUInt8(x>>8);}void WriteUInt32(uint32_t x){WriteUInt16(x);WriteUInt16(x>>16);}void WriteInt32(int x){WriteUInt32(x);}
 void WriteIndex(int v){bool neg=v<0;unsigned n=neg?-v:v;uint8_t c=n&63;n>>=6;WriteUInt8(c|(neg?128:0)|(n?64:0));while(n){c=n&127;n>>=7;WriteUInt8(c|(n?128:0));}}
 void WriteObject(UTexture*t){if(!t){WriteIndex(0);return;}for(auto [ref,p]:textures)if(p==t){WriteIndex(ref);return;}throw std::runtime_error("unknown texture");}
};
using PackageStreamWriter=ObjectStream;
struct UObject{void Load(ObjectStream*s){assert(s->ReadIndex()==0);}void Save(ObjectStream*s){s->WriteIndex(0);}};
struct UTexture:UObject{void Load(ObjectStream*s){UObject::Load(s);}void Save(ObjectStream*s){UObject::Save(s);}};
struct FontCharacter{int StartU=0,StartV=0,USize=0,VSize=0;};
struct FontPage{UTexture*Texture=nullptr;std::vector<FontCharacter>Characters;};
struct FontGlyph{UTexture*Texture=nullptr;int StartU=0,StartV=0,USize=0,VSize=0;FontGlyph()=default;FontGlyph(UTexture*t,const FontCharacter&c):Texture(t),StartU(c.StartU),StartV(c.StartV),USize(c.USize),VSize(c.VSize){}};
struct UFont:UTexture{std::vector<FontPage>pages;int charactersPerPage=0,ue2Kerning=0;std::vector<FontCharacter>poaCharacters;std::vector<uint8_t>poaTextureIndices;bool poaLayout=false,isRemapped=false;std::unordered_map<uint16_t,uint16_t>charRemap;void Load(ObjectStream*);void Save(PackageStreamWriter*);FontGlyph FindGlyph(uint32_t)const;uint32_t RemapChar(uint32_t)const;};
'''+methods+r'''
int main(int argc,char**argv){
 UTexture a,b;ObjectStream s;s.textures={{1,&a},{2,&b}};s.WriteIndex(0);s.WriteIndex(2);
 for(int i=0;i<2;i++){s.WriteInt32(i*10);s.WriteInt32(1);s.WriteInt32(8);s.WriteInt32(12);s.WriteUInt8(i);}
 s.WriteIndex(2);s.WriteIndex(1);s.WriteIndex(2);s.WriteUInt32(3);s.WriteIndex(1);s.WriteUInt16(65);s.WriteUInt16(1);s.WriteUInt32(1);
 engine->LaunchInfo.poa=true;UFont f;f.Load(&s);assert(s.p==s.d.size());assert(f.FindGlyph(65).Texture==&b);assert(f.FindGlyph(65).StartU==10);
 ObjectStream out;out.textures=s.textures;f.Save(&out);assert(out.d==s.d);
 auto bad=s;bad.p=0;bad.d[18]=2;bool rejected=false;try{UFont g;g.Load(&bad);}catch(...){rejected=true;}assert(rejected);
 auto truncated=s;truncated.p=0;truncated.d.pop_back();rejected=false;try{UFont g;g.Load(&truncated);}catch(...){rejected=true;}assert(rejected);
 // Legacy page-first layout still selects the correct atlas.
 engine->LaunchInfo.poa=false;ObjectStream old;old.version=69;old.textures={{1,&a}};old.WriteIndex(0);old.WriteIndex(1);old.WriteIndex(1);old.WriteIndex(1);for(int v:{4,5,6,7})old.WriteInt32(v);old.WriteUInt32(1);old.WriteIndex(0);old.WriteUInt32(0);UFont legacy;legacy.Load(&old);assert(legacy.FindGlyph(0).Texture==&a);ObjectStream saved;saved.version=69;saved.textures=old.textures;legacy.Save(&saved);assert(saved.d==old.d);
 for(int arg=1;arg<argc;arg++){
  ObjectStream corpus;std::ifstream version(std::string(argv[arg])+".version");version>>corpus.version;std::ifstream in(argv[arg],std::ios::binary);corpus.d.assign(std::istreambuf_iterator<char>(in),{});
  // The fixture supplies only references proven to be Texture exports by the EU tables.
  std::ifstream refs(std::string(argv[arg])+".refs");int ref;while(refs>>ref)corpus.textures[ref]=&a;
  engine->LaunchInfo.poa=true;UFont actual;actual.Load(&corpus);if(corpus.p!=corpus.d.size()){std::cerr<<argv[arg]<<" consumed "<<corpus.p<<" of "<<corpus.d.size()<<"\n";return 2;}
  // Distinct atlas pointers preserve reference identities during roundtrip.
  std::vector<UTexture> atlases(corpus.textures.size());size_t n=0;for(auto& item:corpus.textures)item.second=&atlases[n++];
  corpus.p=0;UFont reload;reload.Load(&corpus);ObjectStream saved;saved.version=corpus.version;saved.textures=corpus.textures;reload.Save(&saved);if(saved.d!=corpus.d){std::cerr<<argv[arg]<<" roundtrip size "<<saved.d.size()<<" expected "<<corpus.d.size()<<"\n";return 3;}
  for(unsigned c=0;c<256;c++){auto glyph=reload.FindGlyph(c);if(glyph.USize)assert(glyph.Texture);}
 }
 std::cout<<"Synthetic PoA/legacy font load, save, atlas, remap and rejection checks passed\n";
}
'''
  with tempfile.TemporaryDirectory() as td:
   p=pathlib.Path(td);(p/'font.cpp').write_text(harness)
   subprocess.run(['g++','-std=c++17','-O0',str(p/'font.cpp'),'-o',str(p/'font')],check=True)
   args=[]
   game=os.environ.get('HP3_EU_GAME')
   if game:
    from tools.poa.inspect_packages import inspect
    from tools.poa.object_payloads import Reader,tags
    for path in pathlib.Path(game).rglob('*'):
     if path.suffix.lower() not in ('.u','.utx'):continue
     model=inspect(path);data=path.read_bytes()
     for e in model['exports']:
      if e['class']!='Font':continue
      raw=data[e['offset']:e['offset']+e['size']];r=Reader(raw);tags(r,model['names'])
      # Test Object stream expects a None tag at name index zero.
      raw=bytes([0])+raw[r.p:]
      fixture=p/(path.name+'-'+e['name']+'.bin');fixture.write_bytes(raw)
      (p/(fixture.name+'.version')).write_text(str(model['version']))
      refs=[str(i) for i,x in enumerate(model['exports'],1) if x['class']=='Texture']
      (p/(fixture.name+'.refs')).write_text(' '.join(refs));args.append(str(fixture))
   subprocess.run([str(p/'font'),*args],check=True)
   if game:self.assertGreaterEqual(len(args),15)
if __name__=='__main__':unittest.main()
