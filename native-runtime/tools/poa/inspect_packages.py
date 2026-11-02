"""Read UE2 package tables for compatibility analysis; never execute objects.

This intentionally does not extend the runtime's UE1 deserializers.
"""
import pathlib,struct,json,collections,argparse,hashlib
class Reader:
 def __init__(self,data,offset=0):self.data=data;self.p=offset
 def read(self,n):
  if n<0 or self.p+n>len(self.data):raise ValueError('Read outside package')
  b=self.data[self.p:self.p+n];self.p+=n;return b
 def u32(self):return struct.unpack('<I',self.read(4))[0]
 def i32(self):return struct.unpack('<i',self.read(4))[0]
 def compact(self):
  c=self.read(1)[0];negative=c&128;value=c&63;shift=6;more=c&64
  for i in range(4):
   if not more:break
   c=self.read(1)[0];value|=(c&(31 if i==3 else 127))<<shift;shift+=7;more=c&128
  if more:raise ValueError('Invalid compact index')
  return -value if negative else value
 def string(self):
  n=self.compact()
  if abs(n)>1048576:raise ValueError('Invalid string length')
  return self.read(abs(n)*(2 if n<0 else 1)).decode('utf-16-le' if n<0 else 'latin1').rstrip('\0')
def inspect(path):
 data=path.read_bytes()
 if len(data)<36 or data[:4]!=bytes.fromhex('c1832a9e'):raise ValueError('Invalid package signature')
 version,licensee=struct.unpack_from('<HH',data,4);flags,nc,no,ec,eo,ic,io=struct.unpack_from('<7I',data,8)
 if max(nc,ec,ic)>1000000 or any(o<36 or o>len(data) for c,o in [(nc,no),(ec,eo),(ic,io)] if c):raise ValueError('Invalid table bounds')
 r=Reader(data,no);names=[]
 for _ in range(nc):names.append(r.string());r.u32()
 def name(i):
  if not 0<=i<len(names):raise ValueError('Invalid name index')
  return names[i]
 r=Reader(data,io);imports=[]
 for _ in range(ic):imports.append({'classPackage':name(r.compact()),'class':name(r.compact()),'outer':r.i32(),'name':name(r.compact())})
 r=Reader(data,eo);exports=[]
 for _ in range(ec):
  e={'classIndex':r.compact(),'superIndex':r.compact(),'outer':r.i32(),'name':name(r.compact()),'flags':r.u32(),'size':r.compact()};e['offset']=r.compact() if e['size'] else 0
  if e['size']<0 or e['offset']<0 or e['offset']+e['size']>len(data):raise ValueError('Export range outside package')
  exports.append(e)
 def ref(index):
  if not index:return 'Class'
  table=exports if index>0 else imports;i=index-1 if index>0 else -index-1
  if not 0<=i<len(table):raise ValueError('Invalid object reference')
  return table[i]['name']
 for e in exports:e['class']=ref(e['classIndex']);e['super']=ref(e['superIndex']) if e['superIndex'] else None
 return {'file':path.name,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'version':version,'licenseeVersion':licensee,'names':names,'classCounts':dict(collections.Counter(e['class'] for e in exports)),'imports':imports,'exports':exports}
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True);reports=[]
 for path in sorted(root.rglob('*')):
  if path.suffix.lower() not in ('.u','.unr','.utx','.usx','.ukx','.uax') or any(s.startswith('.') for s in path.relative_to(root).parts):continue
  try:
   report=inspect(path);dest=out/path.relative_to(root);dest=dest.with_suffix(dest.suffix+'.json');dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2));reports.append({'path':str(path.relative_to(root)),'version':report['version'],'licenseeVersion':report['licenseeVersion'],'classCounts':report['classCounts'],'exports':len(report['exports'])})
  except Exception as e:reports.append({'path':str(path.relative_to(root)),'error':str(e)})
 (out/'summary.json').write_text(json.dumps(reports,indent=2));print('Packages',len(reports),'errors',sum('error' in r for r in reports),flush=True)
if __name__=='__main__':main()

