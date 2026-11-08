import os
"""Execute production INI rewrite for repeated-key growth/shrink and comments."""
import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class IniText(unittest.TestCase):
 def test_repeated_keys_and_sections(self):
  code=r'''#include "Package/IniTextUpdate.h"
#include <cassert>
#include <sstream>
std::vector<std::string> Lines(const std::string&s){std::istringstream in(s);std::vector<std::string>r;std::string line;while(std::getline(in,line))r.push_back(line);return r;}
int main(){using namespace IniTextUpdate;
 std::vector<Section> model={{"Game",{{"Items",{"a","b","c"}},{"Slots[1]",{"one"}}}},{"Added",{{"New",{"ok"}}}}};
 auto input=Lines("; heading\n[GAME]\nItems=old\n; preserve\nUnknown=value\nSlots[1]=old\n[Other]\nX=1\n");
 auto result=Update(input,model);assert(result.find("Items=a\nItems=b\nItems=c\n")!=std::string::npos);assert(result.find("; preserve\nUnknown=value")!=std::string::npos);assert(result.find("Slots[1]=one")!=std::string::npos);assert(result.find("[Other]\nX=1")!=std::string::npos);assert(result.find("[Added]\nNew=ok")!=std::string::npos);assert(Update(Lines(result),model)==result);
 model[0].Keys[0].Values={"only"};auto shorter=Update(Lines(result),model);assert(shorter.find("Items=only")!=std::string::npos&&shorter.find("Items=b")==std::string::npos);assert(Update(Lines(shorter),model)==shorter);
 model[0].Keys[0].Values={};auto empty=Update(Lines(shorter),model);assert(empty.find("Items=")==std::string::npos);assert(empty.find("; preserve")!=std::string::npos);
 auto duplicate=Update(Lines("[Game]\nItems=1\n[game]\nItems=2\n// Items=comment\n"),{{"Game",{{"Items",{"new"}}}}});assert(duplicate.find("Items=2")==std::string::npos);assert(duplicate.find("// Items=comment")!=std::string::npos);assert(duplicate.find("Items=new")==duplicate.rfind("Items=new"));
 auto end=Update(Lines("[Game]\nX=1"),{{"Game",{{"Y",{"2","3"}}}}});assert(end.find("Y=2\nY=3\n")!=std::string::npos);
}
'''
  with tempfile.TemporaryDirectory() as temp:
   t=pathlib.Path(temp);(t/'test.cpp').write_text(code)
   subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'src/engine/SurrealEngine'),str(t/'test.cpp'),'-o',str(t/'test')],check=True)
   subprocess.run([str(t/'test')],check=True,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
if __name__=='__main__':unittest.main()
