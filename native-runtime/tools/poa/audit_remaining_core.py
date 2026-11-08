"""Locate unresolved Core declarations and statically resolved call sites.
Registration-name matches are candidates, not proofs of runtime ABI/behavior.
"""
import argparse,pathlib,re,json
from inspect_packages import inspect
from object_payloads import function_header,native_stub
from bytecode import Decoder
from package_objects import PackageObjects

def audit(system,source):
 paths=sorted(p for p in system.iterdir() if p.suffix.lower()=='.u')
 registry=PackageObjects([(p.stem,inspect(p),p.read_bytes()) for p in paths])
 candidates=set()
 roots=[source/'src/engine/SurrealEngine/Native',source/'src/hp3',source/'src/hp2',source/'src/knowwonder']
 for root in roots:
  for p in root.rglob('*.cpp'):
   candidates.update(re.findall(r'RegisterVMNativeFunc_\d+\("([^"]+)",\s*"([^"]+)"',p.read_text(errors='replace')))
 core=next(p for p in paths if p.name.lower()=='core.u');m=inspect(core);data=core.read_bytes();missing={}
 for i,e in enumerate(m['exports'],1):
  if e['class']!='Function':continue
  r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);stub=native_stub(r,h)
  owner=m['exports'][e['outer']-1]['name'] if e['outer']>0 else ''
  if stub and stub['native'] and (owner,e['name']) not in candidates:
   missing[owner+'.'+e['name']]={'owner':owner,'function':e['name'],'identity':registry.path(core.stem,i),'export':i,'nativeIndex':stub['nativeIndex'],'callSites':[]}
 decoded=0
 for p in paths:
  m=inspect(p);data=p.read_bytes()
  for i,e in enumerate(m['exports'],1):
   if e['class']!='Function':continue
   r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);decoder=Decoder(r,m['names'],len(m['imports']),len(m['exports']),h['declaredBytecodeMemorySize']);decoder.decode();decoded+=1
   for token in decoder.tokens:
    matches=[]
    if token.get('opcode')==0x1c:
     target=registry.resolve(p.stem,token['reference'])
     if target and target['package']=='core':
      identity=registry.path('core',target['index']);matches=[(n,'resolved-final') for n,row in missing.items() if row['identity']==identity]
    elif token.get('functionName'):matches=[(n,'named-candidate') for n,row in missing.items() if row['function']==token['functionName']]
    elif token.get('nativeIndex'):
     matches=[(n,'native-index') for n,row in missing.items() if row['nativeIndex'] and row['nativeIndex']==token['nativeIndex']]
    for name,kind in matches:missing[name]['callSites'].append({'caller':registry.path(p.stem,i),'offset':token['offset'],'kind':kind})
 return {'scope':'Static Core registration-name candidate audit; conditional registrations and ABI behavior still require runtime checks','decodedFunctions':decoded,'coreSHA256':inspect(core)['sha256'],'missingCoreCandidates':missing,'phoneExecution':False}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('system',type=pathlib.Path);p.add_argument('source',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args();result=audit(a.system,a.source);a.output.write_text(json.dumps(result,indent=2)+'\n');print({n:len(v['callSites']) for n,v in result['missingCoreCandidates'].items()})
