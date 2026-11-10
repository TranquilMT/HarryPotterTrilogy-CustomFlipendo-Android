"""Read original UE2 startup signatures; does not execute Login or Possess."""
import argparse,json,pathlib
from inspect_packages import Reader,inspect
from object_payloads import function_header,tags

def audit(system):
 result={'scope':'Original EU package metadata, not startup execution or compatibility acceptance','packages':{},'classes':[],'functions':[]}
 for filename in ('Engine.u','kwGame.u','HGame.u'):
  path=system/filename;m=inspect(path);data=path.read_bytes();ex=m['exports'];im=m['imports'];result['packages'][filename]=m['sha256']
  def ref(n):
   if not n:return None
   table=ex if n>0 else im;i=n-1 if n>0 else -n-1
   if not 0<=i<len(table):raise ValueError('Startup object reference outside package')
   return table[i]['name']
  for idx,e in enumerate(ex,1):
   owner=ref(e['outer'])
   if e['class']=='Class'and e['name']in ('Controller','PlayerController','KWHeroController','HPHeroController','HarryController'):
    result['classes'].append({'package':filename,'name':e['name'],'base':ref(e['superIndex'])})
   if e['class']!='Function' or (owner,e['name'])not in [('GameInfo','PreLogin'),('GameInfo','Login'),('GameInfo','PostLogin'),('KWGame','Login'),('KWGame','PostLogin'),('Controller','Possess'),('PlayerController','Possess'),('KWHeroController','Possess')]:continue
   _,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);fields=[];n=h['children'];seen=set()
   while n:
    if n in seen or n<1 or n>len(ex):raise ValueError('Invalid startup function child chain')
    seen.add(n);p=ex[n-1]
    if p['outer']!=idx or not p['class'].endswith('Property'):raise ValueError('Unexpected startup function child')
    r=Reader(data[p['offset']:p['offset']+p['size']]);tags(r,m['names']);r.compact();n=r.compact();dimension=r.u32();flags=r.u32();category=r.compact()
    if flags&0x20:r.read(2)
    target=ref(r.compact())if p['class']in ('ObjectProperty','ClassProperty')else None
    if flags&0x80:fields.append({'name':p['name'],'type':p['class'],'objectClass':target,'out':bool(flags&0x100),'return':bool(flags&0x400),'optional':bool(flags&0x10)})
   result['functions'].append({'package':filename,'owner':owner,'name':e['name'],'parameters':fields})
 return result
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('system',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args();r=audit(a.system);a.output.write_text(json.dumps(r,indent=2)+'\n');print(len(r['functions']),'original startup signatures')
