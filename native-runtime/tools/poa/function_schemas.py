"""Read function property chains and verify native argument order, not ABI compatibility."""
import argparse,pathlib,json,collections
from inspect_packages import inspect,Reader
from object_payloads import tags,name,function_header,native_stub
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True);functions=[];errors=[];natives=0
 for path in sorted((pathlib.Path(a.game)/'system').glob('*.u')):
  m=inspect(path);data=path.read_bytes();cache={}
  def property(ref,owner):
   if not 0<ref<=len(m['exports']):raise ValueError('Property reference outside exports')
   e=m['exports'][ref-1]
   if e['outer']!=owner or not e['class'].endswith('Property'):raise ValueError('Function child is not an owned property')
   if ref in cache:return cache[ref]
   r=Reader(data[e['offset']:e['offset']+e['size']]);tags(r,m['names']);base,next=r.compact(),r.compact();dimension,flags=r.u32(),r.u32();category=name(r,m['names'])
   if flags&0x20:r.read(2)
   if not 1<=dimension<=1048576 or not 0<=next<=len(m['exports']):raise ValueError('Invalid property dimension or child link')
   item={'exportIndex':ref,'name':e['name'],'type':e['class'],'dimension':dimension,'flags':flags,'category':category,'next':next,'typeSpecificBytes':r.read(len(r.data)-r.p).hex(),'parameter':bool(flags&0x80),'out':bool(flags&0x100),'return':bool(flags&0x400),'optional':bool(flags&0x10)};cache[ref]=item;return item
  for i,e in enumerate(m['exports'],1):
   if e['class']!='Function':continue
   try:
    r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);seen=set();fields=[];ref=h['children']
    while ref:
     if ref in seen or len(seen)>=10000:raise ValueError('Cyclic or excessive property chain')
     seen.add(ref);item=property(ref,i);fields.append(item);ref=item['next']
    args=[f for f in fields if f['parameter'] and not f['return']];returns=[f for f in fields if f['return']]
    if len(returns)>1:raise ValueError('Multiple return properties')
    stub=native_stub(r,h)
    if stub and stub['native']:
     if stub['parameters']!=[f['exportIndex'] for f in args]:raise ValueError('Native parameter order differs from schema')
     natives+=1
    owner=m['exports'][e['outer']-1]['name'] if e['outer']>0 else None
    functions.append({'package':path.name,'owner':owner,'name':e['name'],'exportIndex':i,'arguments':args,'returnProperty':returns[0] if returns else None,'locals':[f for f in fields if not f['parameter']],'nativeArgumentOrderVerified':bool(stub and stub['native'])})
   except Exception as error:errors.append({'package':path.name,'name':e['name'],'error':str(error)})
 (out/'function-schemas.json').write_text(json.dumps(functions,indent=2));(out/'schema-errors.json').write_text(json.dumps(errors,indent=2));print('Function schemas',len(functions),'native orders verified',natives,'errors',len(errors),flush=True)
if __name__=='__main__':main()

