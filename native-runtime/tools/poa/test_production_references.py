"""Compile the production reference methods against a small in-memory table.

This tests actual Package.cpp bodies without requiring SDL/Android or retail assets.
It does not validate UObject construction or the full engine build.
"""
import os
import pathlib
import subprocess
import tempfile
import unittest


class ProductionReferences(unittest.TestCase):
    def test_actual_reference_methods(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        source = (root / 'src/engine/SurrealEngine/Package/Package.cpp').read_text()
        accessors = source[source.index('const NameString& Package::GetName('):source.index('void Package::ReadTables()')]
        export_name = source[source.index('std::string Package::GetExportName('):]
        harness = r'''
#include "PackageReferences.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <climits>
using NameString = struct Name { std::string text; std::string ToString() const { return text; } };
struct Entry { int ObjOuter; int ObjName; };
using ExportTableEntry = Entry;
using ImportTableEntry = Entry;
struct Exception { [[noreturn]] static void Throw(const std::string& message) { throw std::runtime_error(message); } };
struct Package {
 NameString Name{"Test"};
 struct Named { NameString Name; };
 std::vector<Named> NameTable = {{{"Root"}}, {{"Child"}}, {{"Asset"}}, {{"External"}}};
 std::vector<Entry> ExportTable = {{0,0},{1,1},{2,2}};
 std::vector<Entry> ImportTable = {{0,3},{-1,0}};
 const NameString& GetName(int) const;
 ExportTableEntry* GetExportEntry(int);
 ImportTableEntry* GetImportEntry(int);
 std::string GetExportName(int);
};
template<class F> void rejects(F f) {
 bool failed=false;
 try { f(); } catch(const std::runtime_error&) { failed=true; }
 assert(failed);
}
'''
        harness += accessors + export_name + r'''
int main() {
 Package p;
 assert(p.GetExportName(1)=="Test.Root");
 assert(p.GetExportName(3)=="Test.Root.Child.Asset");
 assert(p.GetExportName(3)=="Test.Root.Child.Asset");
 assert(p.ExportTable[2].ObjOuter==2 && p.ExportTable[2].ObjName==2);
 rejects([&] { p.GetExportName(4); });
 rejects([&] { p.GetImportEntry(INT_MIN); });
 rejects([&] { p.GetName(-1); });
 rejects([&] { p.GetName(4); });
 p.ExportTable[0].ObjOuter=3;
 rejects([&] { p.GetExportName(3); });
 p.ExportTable[0].ObjOuter=0;
 p.ExportTable[2].ObjOuter=-2;
 assert(p.GetExportName(3)=="Test.External.Root.Asset");
 assert(p.GetExportName(0)=="None");
}
'''
        with tempfile.TemporaryDirectory(prefix='poa-production-refs-') as tmp:
            cpp = pathlib.Path(tmp) / 'test.cpp'
            binary = pathlib.Path(tmp) / 'test'
            cpp.write_text(harness)
            subprocess.run(['g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                            '-I', str(root / 'src/engine/SurrealEngine/Package'),
                            str(cpp), '-o', str(binary)], check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, capture_output=True,
                           env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


if __name__ == '__main__':
    unittest.main()
