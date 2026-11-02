"""Bounded HP3 tagged-property and function-header readers.

These deserialize metadata, not live actors or executable script statements.
"""
import struct,pathlib,json,collections,argparse
from inspect_packages import Reader,inspect
def name(r,names):
 i=r.compact()
 if not 0<=i<len(names):raise ValueError('Name outside table')
 return names[i]
def tags(r,names):
 result=[]
 for _ in range(100000):
  key=name(r,names)
  if key=='None':return result
  info=r.read(1)[0];kind=info&15
  if not 1<=kind<=15:raise ValueError('Invalid property type')
  struct_name=name(r,names) if kind==10 else None
  code=(info>>4)&7;size=[1,2,4,12,16,None,None,None][code]
  if size is None:size=int.from_bytes(r.read([1,2,4][code-5]),'little')
  index=0
  if info&128 and kind!=3:
   b=r.read(1)[0]
   if b<128:index=b
   elif b&64:index=int.from_bytes(bytes([b])+r.read(3),'big')&0x3fffffff
   else:index=int.from_bytes(bytes([b])+r.read(1),'big')&0x3fff
  start=r.p
  if kind!=3:r.read(size)
  result.append({'name':key,'type':kind,'struct':struct_name,'arrayIndex':index,'offset':start,'size':0 if kind==3 else size,'boolValue':bool(info&128) if kind==3 else None})
 raise ValueError('Too many properties')
def function_header(raw,names):
 r=Reader(raw);props=tags(r,names);refs=[r.compact() for _ in range(4)];friendly=name(r,names);extra=r.read(1)[0]
 if extra!=0:raise ValueError('Unverified HP3 struct extension byte')
 line,text,size=r.u32(),r.u32(),r.u32()
 if size>64*1024*1024:raise ValueError('Excessive bytecode size')
 return r,{'super':refs[0],'next':refs[1],'scriptText':refs[2],'children':refs[3],'friendlyName':friendly,'hp3ExtensionByte':extra,'line':line,'textPosition':text,'declaredBytecodeMemorySize':size,'serializedBytecodeOffset':r.p,'propertyCount':len(props)}
def native_stub(r,header):
 mem=0;parms=[]
 while mem<header['declaredBytecodeMemorySize']:
  op=r.read(1)[0];mem+=1
  if op==0x29:parms.append(r.compact());mem+=4
  elif op!=0x0b:return None
 if mem!=header['declaredBytecodeMemorySize']:raise ValueError('Bytecode memory-size mismatch')
 index=int.from_bytes(r.read(2),'little');precedence=r.read(1)[0];flags=r.u32();replication=int.from_bytes(r.read(2),'little') if flags&0x40 else None
 if r.p!=len(r.data):raise ValueError('Unconsumed function bytes')
 return {'nativeIndex':index,'precedence':precedence,'flags':flags,'replicationOffset':replication,'native':bool(flags&0x400),'parameters':parms}
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True);summaries=[];native=[];headers=[];failures=[]
 for path in sorted(root.rglob('*')):
  if path.suffix.lower() not in ('.u','.unr','.utx','.usx','.ukx','.uax') or any(x.startswith('.') for x in path.relative_to(root).parts):continue
  m=inspect(path);data=path.read_bytes();names=m['names'];stats=collections.Counter();kinds=collections.Counter()
  for i,e in enumerate(m['exports'],1):
   if not e['size']:continue
   raw=data[e['offset']:e['offset']+e['size']]
   if e['class']=='Class':stats['classDefaultsPending']+=1;continue
   if e['flags']&0x02000000:stats['stateFramePending']+=1;continue
   try:
    if e['class']=='Function':
     r,h=function_header(raw,names)
     for key in ['super','next','scriptText','children']:
      ref=h[key]
      if ref>len(m['exports']) or ref < -len(m['imports']):raise ValueError('Function object reference outside table')
     owner=m['exports'][e['outer']-1]['name'] if e['outer']>0 else None
     h.update(package=str(path.relative_to(root)),owner=owner,object=e['name'],exportIndex=i);headers.append(h);stats['functionHeaders']+=1;n=native_stub(r,h)
     if n is None:stats['scriptBodiesPending']+=1
     elif n['native']:
      for ref in n['parameters']:
       if ref<=0 or ref>len(m['exports']) or not m['exports'][ref-1]['class'].endswith('Property'):raise ValueError('Native parameter is not a property export')
      native.append(dict(h,**n));stats['nativeStubs']+=1
     else:stats['emptyScriptBodies']+=1
    else:
     r=Reader(raw);t=tags(r,names);stats['taggedPrefixes']+=1;stats['propertyTags']+=len(t);kinds.update(str(x['type']) for x in t)
   except Exception as error:failures.append({'package':str(path.relative_to(root)),'object':e['name'],'class':e['class'],'error':str(error)});stats['errors']+=1
  summaries.append({'package':str(path.relative_to(root)),'counts':dict(stats),'propertyTypes':dict(kinds)})
 (out/'object-summary.json').write_text(json.dumps(summaries,indent=2));(out/'function-headers.json').write_text(json.dumps(headers,indent=2));(out/'native-bindings.json').write_text(json.dumps(native,indent=2));(out/'object-errors.json').write_text(json.dumps(failures,indent=2))
 totals=collections.Counter()
 for x in summaries:totals.update(x['counts'])
 (out/'totals.json').write_text(json.dumps(dict(totals),indent=2));print(dict(totals),flush=True)
if __name__=='__main__':main()

