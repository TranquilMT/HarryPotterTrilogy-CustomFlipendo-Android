import struct,pathlib,json,collections,hashlib,sys
class Reader:
 def __init__(self,b,p=0):self.b=b;self.p=p
 def read(self,n):
  if n<0 or self.p+n>len(self.b):raise ValueError('Read outside package at '+str(self.p))
  v=self.b[self.p:self.p+n];self.p+=n;return v
 def u32(self):return struct.unpack('<I',self.read(4))[0]
 def i32(self):return struct.unpack('<i',self.read(4))[0]
 def compact(self):
  c=self.read(1)[0];neg=c&128;v=c&63;shift=6;more=c&64
  for i in range(4):
   if not more:break
   c=self.read(1)[0];v|=(c&(31 if i==3 else 127))<<shift;shift+=7;more=c&128
  else:
   if more:raise ValueError('Bad compact integer')
  return -v if neg else v
 def string(self):
  n=self.compact();raw=self.read(abs(n)*(2 if n<0 else 1));return raw.decode('utf-16-le' if n<0 else 'latin1').rstrip('\0')
def parse(path):
 b=path.read_bytes();sig,ver,license=struct.unpack_from('<IHH',b);assert sig==0x9e2a83c1
 flags,nc,no,ec,eo,ic,io=struct.unpack_from('<7I',b,8);r=Reader(b,no);names=[]
 for _ in range(nc):names.append(r.string());r.u32()
 r=Reader(b,io);imports=[]
 for _ in range(ic):imports.append(dict(classPackage=r.compact(),className=r.compact(),outer=r.i32(),name=r.compact()))
 r=Reader(b,eo);exports=[]
 for _ in range(ec):
  e=dict(classIndex=r.compact(),superIndex=r.compact(),outer=r.i32(),name=r.compact(),flags=r.u32(),size=r.compact());e['offset']=r.compact() if e['size'] else 0
  if e['size']<0 or e['offset']<0 or e['offset']+e['size']>len(b):raise ValueError('Invalid export range')
  exports.append(e)
 def name(i):
  if i<0 or i>=len(names):raise ValueError('Bad name reference')
  return names[i]
 def ref(i):
  if not i:return 'Class'
  return name((exports[i-1] if i>0 else imports[-i-1])['name'])
 for e in imports:e['objectName']=name(e['name']);e['class']=name(e['className']);e['package']=name(e['classPackage'])
 for e in exports:e['objectName']=name(e['name']);e['class']=ref(e['classIndex']);e['super']=ref(e['superIndex']) if e['superIndex'] else None;e['outerName']=ref(e['outer']) if e['outer'] else None
 result=dict(file=path.name,bytes=len(b),sha256=hashlib.sha256(b).hexdigest(),version=ver,licenseeVersion=license,names=names,imports=imports,exports=exports,counts=dict(collections.Counter(e['class'] for e in exports)))
 path.with_suffix(path.suffix+'.json').write_text(json.dumps(result,indent=2));print(path.name,'v',ver,'names',nc,'imports',ic,'exports',ec,'types',result['counts'])
 return result
if __name__=='__main__':
 for arg in sys.argv[1:]:parse(pathlib.Path(arg))
