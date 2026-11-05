"""Compile primitive-cast bridge and actual existing AST construction cases.

The harness substitutes stream/node allocation only. It does not compile the
complete VM, execute game natives or establish original PoA numeric parity.
"""
import os
import pathlib
import re
import subprocess
import tempfile
import unittest


class ProductionPrimitiveCasts(unittest.TestCase):
    def test_primitive_expression_bridge(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        cpp = (root / 'src/engine/SurrealEngine/VM/Bytecode.cpp').read_text()
        header = (root / 'src/engine/SurrealEngine/Packages/Core/UStruct.h').read_text()
        enum = header[header.index('enum class ExprToken : uint8_t\n{'):]
        bridge = re.search(r'token = static_cast<ExprToken>\(PoaPrimitiveCasts::ExpressionToken\(\*stream,\s*uint8_t\(token\), engine->LaunchInfo.IsHarryPotter3\(\)\)\);', cpp)
        self.assertIsNotNone(bridge)
        names = ['ByteToInt', 'ByteToBool', 'ByteToFloat', 'IntToByte', 'IntToBool', 'IntToFloat',
                 'BoolToByte', 'BoolToInt', 'BoolToFloat', 'FloatToByte', 'FloatToInt', 'FloatToBool',
                 'RotatorToVector', 'StringToByte', 'StringToInt', 'StringToBool', 'StringToFloat',
                 'VectorToRotator', 'ByteToString', 'IntToString', 'BoolToString', 'FloatToString',
                 'ObjectToString', 'NameToString', 'VectorToString', 'RotatorToString']
        cases = []
        for name in names + ['IntZero', 'IntOne', 'True', 'False']:
            case = re.search(r'case ExprToken::' + name + r': \{[^\n]+\}', cpp)
            self.assertIsNotNone(case, name)
            cases.append(case.group())
        declarations = '\n'.join('struct ' + name + 'Expression:Expression {' + name +
                                 'Expression(){kind=uint8_t(ExprToken::' + name + ');}};'
                                 for name in names + ['IntZero', 'IntOne', 'True', 'False'])
        harness = r'''
#include <algorithm>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include "Package/PoaPrimitiveCast.h"
'''
        harness += enum + r'''
struct LaunchInfo {bool poa=true;bool IsHarryPotter3()const{return poa;}};
struct Engine {struct LaunchInfo LaunchInfo;};Engine instance;Engine*engine=&instance;
struct Stream {std::vector<uint8_t>data;size_t pos=0;uint8_t ReadUInt8(){if(pos==data.size())throw std::runtime_error("truncated");return data[pos++];}ExprToken ReadToken(){return ExprToken(ReadUInt8());}uint16_t GetOffset(){return uint16_t(pos);}};
struct Expression {virtual ~Expression()=default;uint8_t kind=0;uint16_t offset=0;Expression*Value=nullptr;};
'''
        harness += declarations + r'''
struct AST {
 std::vector<std::unique_ptr<Expression>> storage;std::map<uint16_t,Expression*> lookup;
 template<class T>T*Create(uint16_t offset){auto node=std::make_unique<T>();auto p=node.get();p->offset=offset;lookup[offset]=p;storage.push_back(std::move(node));return p;}
 Expression*ReadToken(Stream*stream,int depth){
  if(depth>=64) { throw std::runtime_error("depth"); }
  ++depth;
  uint16_t exproffset=stream->GetOffset();ExprToken token=stream->ReadToken();
'''
        harness += bridge.group() + '\n switch(token){\n' + '\n'.join(cases) + r'''
 default:throw std::runtime_error("not a scalar expression in this harness");}
 }
};
int checks=0;
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}assert(failed);++checks;}
int main(){
 const std::vector<int> supported={0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x45,
                                   0x49,0x4a,0x4b,0x4c,0x50,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59};
 for(int code:supported){Stream stream;stream.data={0x39,uint8_t(code),0x25};AST ast;Expression*e=ast.ReadToken(&stream,0);assert(e->kind==code&&e->offset==0&&e->Value->kind==0x25&&e->Value->offset==2&&stream.pos==3);assert(ast.lookup.at(0)==e&&ast.lookup.at(2)==e->Value);++checks;}
 Stream nested;nested.data={0x39,0x3f,0x39,0x41,0x27};AST ast;auto e=ast.ReadToken(&nested,0);assert(e->kind==0x3f&&e->Value->kind==0x41&&e->Value->Value->kind==0x27&&e->Value->offset==2&&e->Value->Value->offset==4&&nested.pos==5);++checks;
 for(int code=0;code<256;++code){if(std::find(supported.begin(),supported.end(),code)!=supported.end())continue;Stream stream;stream.data={0x39,uint8_t(code),0x25};AST bad;rejects([&]{bad.ReadToken(&stream,0);});}
 for(int token=0x3a;token<0x60;++token){Stream stream;stream.data={uint8_t(token),0x25};AST bad;rejects([&]{bad.ReadToken(&stream,0);});}
 Stream sequential;sequential.data={0x39,0x3a,0x25,0x26};AST sequence;auto first=sequence.ReadToken(&sequential,0);auto second=sequence.ReadToken(&sequential,0);assert(first->offset==0&&first->Value->offset==2&&second->offset==3&&sequential.pos==4);++checks;
 Stream shortCast;shortCast.data={0x39};AST shortAst;rejects([&]{shortAst.ReadToken(&shortCast,0);});
 Stream shortValue;shortValue.data={0x39,0x3a};AST shortValueAst;rejects([&]{shortValueAst.ReadToken(&shortValue,0);});
 Stream unchanged;unchanged.data={0x12};assert(PoaPrimitiveCasts::ExpressionToken(unchanged,0x60,true)==0x60&&unchanged.pos==0);++checks;
 engine->LaunchInfo.poa=false;
 for(int code=0x3a;code<=0x45;++code){Stream legacy;legacy.data={uint8_t(code),0x25};AST old;auto n=old.ReadToken(&legacy,0);assert(n->kind==code&&n->Value->offset==1&&legacy.pos==2);++checks;}
 Stream legacy;legacy.data={0x25};assert(PoaPrimitiveCasts::ExpressionToken(legacy,0x39,false)==0x39&&legacy.pos==0);++checks;
 std::cout<<"PASS "<<checks<<" primitive-cast AST bridge cases\n";
}
'''
        with tempfile.TemporaryDirectory(prefix='poa-scalar-casts-') as tmp:
            tmp = pathlib.Path(tmp)
            source, binary = tmp / 'test.cpp', tmp / 'test'
            source.write_text(harness)
            compiled = subprocess.run(['g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                            '-I', str(root / 'src/engine/SurrealEngine'), str(source), '-o', str(binary)],
                           capture_output=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr.decode())
            subprocess.run([str(binary)], check=True, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


if __name__ == '__main__':
    unittest.main()
