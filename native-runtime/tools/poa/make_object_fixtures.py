import pathlib,argparse,struct
from inspect_packages import inspect,Reader
from object_payloads import tags,function_header,native_stub
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();groups=[];total=0
 for path in sorted(pathlib.Path(a.game).rglob('*')):
  if path.suffix.lower() not in ('.u','.unr','.utx','.usx','.ukx','.uax') or any(x.startswith('.') for x in path.parts):continue
  m=inspect(path);data=path.read_bytes();cases=[];selected=0
  for e in m['exports']:
   if not e['size'] or e['class']=='Class' or e['flags']&0x02000000:continue
   raw=data[e['offset']:e['offset']+e['size']]
   if e['class']=='Function':
    r,h=function_header(raw,m['names']);n=native_stub(r,h)
    if n and n['native']:cases.append((2,raw,n['nativeIndex'],len(n['parameters'])))
   elif selected<12:
    r=Reader(raw);t=tags(r,m['names'])
    if t:cases.append((1,raw[:r.p],r.p,len(t)));selected+=1
  if cases:groups.append((m['names'],cases));total+=len(cases)
 with open(a.output,'wb') as out:
  def u32(n):out.write(struct.pack('<I',n))
  u32(len(groups))
  for names,cases in groups:
   u32(len(names))
   for n in names:b=n.encode('utf8');u32(len(b));out.write(b)
   u32(len(cases))
   for kind,raw,x,y in cases:u32(kind);u32(len(raw));out.write(raw);u32(x);u32(y)
 print('Real-object fixtures',total,'packages',len(groups),flush=True)
if __name__=='__main__':main()
