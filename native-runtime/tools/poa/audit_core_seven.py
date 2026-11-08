"""Trace the seven EU Core declarations, static call sites and class config names.
This is static corpus evidence, not phone or full gameplay execution.
"""
import argparse,collections,json,pathlib
from inspect_packages import inspect,Reader
from object_payloads import function_header,native_stub,name
from bytecode import Decoder
from package_objects import PackageObjects
NAMES=('Acos','Asin','Greater_StrStr','GetLastStateName','QuatProduct','QuatRotateVector','QuatFromAxisAndAngle')
def audit(system):
 paths=sorted(p for p in system.iterdir() if p.suffix.lower()=='.u');registry=PackageObjects([(p.stem,inspect(p),p.read_bytes()) for p in paths]);rows={n:{'declaration':None,'callSites':[]} for n in NAMES};models={};functions=0;configs=collections.defaultdict(list)
 for p in paths:
  m=inspect(p);data=p.read_bytes();models[p.name]=m['sha256']
  for i,e in enumerate(m['exports'],1):
   if not e['size']:continue
   raw=data[e['offset']:e['offset']+e['size']]
   if e['class']=='Function':
    r,h=function_header(raw,m['names']);d=Decoder(r,m['names'],len(m['imports']),len(m['exports']),h['declaredBytecodeMemorySize']);d.decode();functions+=1
    if p.name.lower()=='core.u' and e['name'] in rows:
     r,h=function_header(raw,m['names']);stub=native_stub(r,h);assert stub and stub['native']
     fields=[];ref=h['children'];seen=set()
     while ref:
      assert ref>0 and ref not in seen;seen.add(ref);field=m['exports'][ref-1];assert field['outer']==i
      pr=Reader(data[field['offset']:field['offset']+field['size']]);from object_payloads import tags
      tags(pr,m['names']);pr.compact();nextref=pr.compact();dimension=pr.u32();flags=pr.u32();name(pr,m['names'])
      if flags&0x20:pr.read(2)
      kind=field['class']
      if kind=='StructProperty':
       typeref=pr.compact();table=m['exports'] if typeref>0 else m['imports'];kind=table[abs(typeref)-1]['name']
      fields.append({'export':ref,'name':field['name'],'type':kind,'flags':flags,'dimension':dimension});ref=nextref
     args=[f for f in fields if f['flags']&0x80 and not f['flags']&0x400];returns=[f for f in fields if f['flags']&0x400]
     expected={'Acos':(['FloatProperty'],'FloatProperty',0),'Asin':(['FloatProperty'],'FloatProperty',0),'Greater_StrStr':(['StrProperty','StrProperty'],'BoolProperty',116),'GetLastStateName':([],'NameProperty',285),'QuatProduct':(['Quat','Quat'],'Quat',0),'QuatRotateVector':(['Quat','Vector'],'Vector',0),'QuatFromAxisAndAngle':(['Vector','FloatProperty'],'Quat',0)}[e['name']]
     assert ([f['type'] for f in args],returns[0]['type'],stub['nativeIndex'])==expected
     assert stub['parameters']==[f['export'] for f in args] and all(f['dimension']==1 for f in fields)
     rows[e['name']]['declaration']={'export':i,'arguments':args,'return':returns[0],**stub}
    for t in d.tokens:
     n=None;kind=None
     if t.get('nativeIndex') in (116,285):n={116:'Greater_StrStr',285:'GetLastStateName'}[t['nativeIndex']];kind='native-index'
     elif t.get('functionName') in rows:n=t['functionName'];kind='named-candidate'
     elif t['opcode']==0x1c:
      target=registry.resolve(p.stem,t['reference'])
      if target and target['package']=='core' and target['entry']['name'] in rows:n=target['entry']['name'];kind='resolved-core-final'
     if n:rows[n]['callSites'].append({'package':p.name,'function':e['name'],'caller':registry.path(p.stem,i),'export':i,'offset':t['offset'],'kind':kind})
   elif e['class']=='Class':
    r=Reader(raw)
    for _ in range(4):r.compact()
    name(r,m['names']);r.compact();r.u32();r.u32();size=r.u32();Decoder(r,m['names'],len(m['imports']),len(m['exports']),size).decode();r.read(22+4+16)
    for _ in range(r.compact()):r.compact();r.read(8)
    for _ in range(r.compact()):r.compact()
    r.compact();config=name(r,m['names'])
    if config.lower()!='none':configs[config].append(p.stem+'.'+e['name'])
 assert all(v['declaration'] for v in rows.values())
 files={p.name.lower() for p in system.iterdir() if p.is_file()}
 return {'decodedFunctions':functions,'packages':models,'bindings':rows,'classConfigs':[{'name':n,'present':n.lower()+'.ini' in files,'classes':v} for n,v in sorted(configs.items())],'runtimeGameplayVerified':False,'namedCallScope':'Candidates require runtime receiver resolution; indexed calls are exact EU indices.'}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('system',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args();r=audit(a.system);a.output.write_text(json.dumps(r,indent=2)+'\n');print('Decoded functions',r['decodedFunctions']);print({n:len(v['callSites']) for n,v in r['bindings'].items()});print('Missing class configs',[v['name'] for v in r['classConfigs'] if not v['present']])
