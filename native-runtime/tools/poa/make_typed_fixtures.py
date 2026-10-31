"""Build a primitive-only isolated call corpus, excluding unsupported frame layouts."""
import pathlib,json,argparse,struct,collections
from inspect_packages import inspect
from object_payloads import function_header
from bytecode import Decoder
KINDS={'BoolProperty':1,'IntProperty':2,'FloatProperty':3,'ByteProperty':4}
INTRINSICS={'Add_IntInt':1,'Subtract_IntInt':2,'Multiply_IntInt':3,'Less_IntInt':4,'Greater_IntInt':5,'EqualEqual_IntInt':6,'NotEqual_IntInt':7,'Not_PreBool':8,'Add_FloatFloat':9,'Subtract_FloatFloat':10,'Multiply_FloatFloat':11}
def supported(p):return p is None or (p['type'] in KINDS and p['dimension']==1 and (p['type']!='ByteProperty' or p['typeSpecificBytes']=='00'))
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('audit');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);audit=pathlib.Path(a.audit);schema={(x['package'],x['exportIndex']):x for x in json.loads((audit/'function-schemas.json').read_text())};groups=[];ids={};symbols={};nextid=1;excluded=collections.Counter()
 for path in sorted((root/'system').glob('*.u')):
  m=inspect(path);data=path.read_bytes();cases=[]
  def object_path(ref,seen=None):
   if not ref:return ''
   seen=set() if seen is None else seen
   if ref in seen:raise ValueError('Cyclic object outer')
   seen.add(ref);e=m['exports'][ref-1] if ref>0 else m['imports'][-ref-1];parent=object_path(e['outer'],seen)
   return (parent+'.' if parent else (path.stem+'.' if ref>0 else ''))+e['name']
  for index,e in enumerate(m['exports'],1):
   if e['class']!='Function':continue
   uid=nextid;nextid+=1;qualified=object_path(index);key=(path.name,index);ids[key]=uid
   if qualified.lower() in symbols:raise ValueError('Ambiguous function symbol')
   symbols[qualified.lower()]=uid
   s=schema[key]
   if not all(supported(x) for x in s['arguments']+s['locals']) or not supported(s['returnProperty']):excluded['unsupportedFrameType']+=1;continue
   if any(x['out'] or x['flags']&0x200 for x in s['arguments']):excluded['outOrLazyArgument']+=1;continue
   raw=data[e['offset']:e['offset']+e['size']];r,h=function_header(raw,m['names']);d=Decoder(r,m['names'],len(m['imports']),len(m['exports']),h['declaredBytecodeMemorySize']);d.decode();native=int.from_bytes(r.read(2),'little');r.read(1);flags=r.u32()
   intrinsic=INTRINSICS.get(e['name'],0) if qualified.startswith('Core.Object.') else 0
   if flags&0x400 and not intrinsic:excluded['unimplementedNative']+=1;continue
   links=[]
   for ref in sorted({c['reference'] for c in d.calls if c['kind']=='final'}):links.append((ref,object_path(ref)))
   cases.append({'uid':uid,'qualified':qualified,'raw':raw,'schema':s,'intrinsic':intrinsic,'native':bool(flags&0x400),'links':links})
  if cases:groups.append((m,cases,path.name))
 linked=0;unresolved=[];registered=sum(len(c) for m,c,p in groups);native_count=sum(c['native'] for m,cases,p in groups for c in cases)
 with open(a.output,'wb') as out:
  def u32(n):out.write(struct.pack('<I',n))
  def blob(b):u32(len(b));out.write(b)
  def slot(s):
   if s is None:
    for n in [0,0,0,0]:u32(n)
   else:
    for n in [s['exportIndex'],KINDS[s['type']],int(s['optional']),s['flags']]:u32(n)
  u32(len(groups))
  for m,cases,package in groups:
   u32(len(m['names']))
   for n in m['names']:blob(n.encode('utf8'))
   u32(len(m['imports']));u32(len(m['exports']));u32(len(cases))
   for c in cases:
    u32(c['uid']);blob(c['qualified'].encode('utf8'));blob(c['raw']);u32(c['intrinsic']);s=c['schema']
    for key in ['arguments','locals']:
     u32(len(s[key]))
     for item in s[key]:slot(item)
    slot(s['returnProperty']);links=[]
    for ref,symbol in c['links']:
     target=symbols.get(symbol.lower())
     if target is None:unresolved.append({'caller':c['qualified'],'reference':ref,'symbol':symbol})
     else:links.append((ref,target));linked+=1
    u32(len(links))
    for ref,target in links:out.write(struct.pack('<iI',ref,target))
 report={'registeredFunctions':registered,'scripts':registered-native_count,'explicitCoreIntrinsics':native_count,'resolvedFinalReferences':linked,'unresolvedFinalReferences':unresolved,'excluded':dict(excluded),'runtimeIntegrated':False,'scope':'Primitive-only isolated frames; layouts with objects, containers, structs, enums, out/lazy parameters are excluded. Resolved callees may still be unregistered and execution must fail explicitly.'}
 pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(report,indent=2));print(report,flush=True)
if __name__=='__main__':main()
