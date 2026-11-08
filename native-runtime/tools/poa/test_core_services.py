"""Production PoA file-service bodies with filesystem/reflection adapters."""
import pathlib,tempfile,subprocess,unittest,os
from test_core_seven_runtime import definition
ROOT=pathlib.Path(__file__).resolve().parents[2];ENGINE=ROOT/'src/engine/SurrealEngine'
class CoreServices(unittest.TestCase):
 def test_files_encoding_array_alias_and_slot_deletion(self):
  source=(ROOT/'src/hp3/PoaServices.cpp').read_text();bodies='\n'.join(definition(source,s)for s in ['static fs::path TextPath(','static UProperty* StringElement(','static void LoadStringArray(','static void SaveStringArray(','static void KWDeleteSaveGame('])
  code=r'''#include "hp3/PoaTextArray.h"
#include "Utils/Exception.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <cassert>
namespace fs=std::filesystem;
void Exception::Throw(const std::string&s){throw std::runtime_error(s);}
enum class ExpressionValueType{ValueString,ValueInt};
struct UProperty{ExpressionValueType ValueType=ExpressionValueType::ValueString;int ArrayDimension=1;size_t ElementSize(){return sizeof(std::string);}};
struct ScriptArray{UProperty*Type;std::vector<std::string>Values;explicit ScriptArray(UProperty*t):Type(t){}UProperty*GetType(){return Type;}size_t GetSize(){return Values.size();}void*GetItem(size_t i){return &Values.at(i);}void Resize(size_t n){Values.resize(n);}void Swap(ScriptArray&v){assert(Type==v.Type);Values.swap(v.Values);}};
struct File{static fs::path find_ignoring_case(const fs::path&p){if(fs::exists(p))return p;auto parent=p.parent_path();if(fs::exists(parent)){std::string name=p.filename();for(auto&c:name)c=char(std::tolower(static_cast<unsigned char>(c)));for(const auto&e:fs::directory_iterator(parent)){std::string candidate=e.path().filename();for(auto&c:candidate)c=char(std::tolower(static_cast<unsigned char>(c)));if(candidate==name)return e.path();}}return p;}static std::shared_ptr<int>try_open_existing(const std::string&p){return fs::exists(p)?std::make_shared<int>(1):nullptr;}static std::string read_all_text(const std::string&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("read");return {std::istreambuf_iterator<char>(f),{}};}static void write_all_text(const std::string&p,const std::string&s){std::ofstream f(p,std::ios::binary);if(!f)throw std::runtime_error("write");f.write(s.data(),s.size());if(!f)throw std::runtime_error("write");}static bool try_delete(const std::string&p){return fs::remove(p);}};
struct Packages{fs::path Root;fs::path GetRootFolderPath(){return Root;}fs::path GetSystemFolderPath(){return Root/"System";}fs::path GetSaveFolderPath(){return Root/"Saves";}}packages;
struct Engine{Packages*packages;}instance{&packages};Engine*engine=&instance;
'''+bodies+r'''
int main(int argc,char**argv){assert(argc==2);packages.Root=argv[1];fs::create_directories(packages.Root/"System");fs::create_directories(packages.Root/"Saves");
 UProperty strings;ScriptArray array(&strings);array.Values={"Harry","",u8"caf\u00e9"};std::string result="not empty";
 SaveStringArray(array,"..\\System\\credits.txt",result);assert(result.empty());assert(File::read_all_text((packages.Root/"System/credits.txt").string())==std::string("Harry\r\n\r\ncaf\xe9\r\n"));array.Values={"old"};LoadStringArray(array,"CREDITS.TXT",result);assert((array.Values==std::vector<std::string>{"Harry","",u8"caf\u00e9"}));
 array.Values={u8"\u0413\u0435\u0440\u043c\u0438\u043e\u043d\u0430",u8"\U0001f9d9"};SaveStringArray(array,"unicode.txt",result);auto bytes=File::read_all_text((packages.Root/"System/unicode.txt").string());assert(bytes.substr(0,2)=="\xff\xfe");auto expected=array.Values;array.Values={"old"};LoadStringArray(array,"unicode.txt",result);assert(array.Values==expected);
 assert((PoaTextArray::Lines("a\r\n\r\nb\r\ntail")==std::vector<std::string>{"a","","b"}));assert(PoaTextArray::Lines("unterminated").empty());
 array.Values={"retain"};File::write_all_text((packages.Root/"System/bad.txt").string(),"\xff\xfe\x01");bool rejected=false;try{LoadStringArray(array,"bad.txt",result);}catch(...){rejected=true;}assert(rejected&&array.Values==std::vector<std::string>{"retain"});
 LoadStringArray(array,"absent.txt",result);assert(array.Values.empty());
 rejected=false;try{SaveStringArray(array,"../../escape.txt",result);}catch(...){rejected=true;}assert(rejected);
 strings.ValueType=ExpressionValueType::ValueInt;rejected=false;try{LoadStringArray(array,"credits.txt",result);}catch(...){rejected=true;}assert(rejected);
 auto a=(packages.Root/"Saves/Save3.usa").string(),b=(packages.Root/"Saves/Save30.usa").string();File::write_all_text(a,"slot3");File::write_all_text(b,"slot30");KWDeleteSaveGame(3);assert(!fs::exists(a)&&fs::exists(b));KWDeleteSaveGame(3);
}
'''
  # C++20 char8_t is deliberately disabled to match the engine's std::string UTF-8 API.
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-fno-char8_t','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src'),'-I'+str(ENGINE),str(t/'test.cpp'),str(ENGINE/'Utils/UTF16.cpp'),'-o',str(t/'test')],check=True)
   subprocess.run([str(t/'test'),str(t/'game')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if __name__=='__main__':unittest.main()
