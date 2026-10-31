"""Shared version-76 object-reference and tagged-property decoding."""
import json,struct
from parse_package import Reader
TYPES={1:'Byte',2:'Int',3:'Bool',4:'Float',5:'Object',6:'Name',7:'String',8:'Class',9:'Array',10:'Struct',11:'Vector',12:'Rotator',13:'Str',14:'Map',15:'FixedArray'}
class PackageView:
 def __init__(self,path):
  self.path=path;self.data=json.load(open(str(path)+'.json'));self.bytes=path.read_bytes();self.names=self.data['names'];self.exports=self.data['exports'];self.imports=self.data['imports']
 def name(self,n):
  if not 0<=n<len(self.names):raise ValueError('Invalid name index')
  return self.names[n]
 def object_path(self,n,seen=None):
  if not n:return self.path.stem
  seen=set() if seen is None else seen
  if n in seen:raise ValueError('Cyclic outer')
  seen.add(n);e=self.entry(n)
  return (self.object_path(e['outer'],seen)+'.' if e['outer'] else (self.path.stem+'.' if n>0 else ''))+e['objectName']
 def entry(self,n):
  if n==0:raise ValueError('Null object has no entry')
  a=self.exports if n>0 else self.imports;i=n-1 if n>0 else -n-1
  if not 0<=i<len(a):raise ValueError('Invalid object reference')
  return a[i]
 def ref(self,n):
  if not n:return None
  e=self.entry(n);result={'index':n,'name':e['objectName'],'class':e['class'],'objectPath':self.object_path(n)}
  if n<0:result['classPackage']=e['package']
  return result
def read_properties(r,view,base_offset=0):
 name=view.name;ref=view.ref;props=[]
 while True:
  tagstart=r.p;prop=name(r.compact())
  if prop=='None':break
  info=r.read(1)[0];typ=info&15
  if typ not in TYPES:raise ValueError('Invalid property type')
  structname=name(r.compact()) if typ==10 else None
  sizecode=(info>>4)&7
  size=[1,2,4,12,16][sizecode] if sizecode<5 else (r.read(1)[0] if sizecode==5 else struct.unpack('<H',r.read(2))[0] if sizecode==6 else r.i32())
  idx=0
  if info&128 and typ!=3:
   x=r.read(1)[0]
   idx=x if not x&128 else ((x&127)<<8)|r.read(1)[0] if x&192==128 else ((x&63)<<24)|int.from_bytes(r.read(3),'big')
  payload=b'' if typ==3 else r.read(size)
  value=None;pr=Reader(payload)
  if typ==3:value=bool(info&128)
  elif typ==1 and size==1:value=payload[0]
  elif typ==2 and size==4:value=struct.unpack('<i',payload)[0]
  elif typ==4 and size==4:value=struct.unpack('<f',payload)[0]
  elif typ in (5,8):value=ref(pr.compact());assert pr.p==len(payload)
  elif typ==6:value=name(pr.compact());assert pr.p==len(payload)
  elif typ==13:value=pr.string();assert pr.p==len(payload)
  elif typ==10 and structname in ('Vector','Rotator') and size==12:value=list(struct.unpack('<fff' if structname=='Vector' else '<iii',payload))
  decoded=typ in (2,3,4,5,6,8,13) or (typ==1 and size==1) or (typ==10 and structname in ('Vector','Rotator') and size==12)
  props.append(dict(decoded=decoded,name=prop,type=TYPES[typ],struct=structname,arrayIndex=idx,size=size,value=value,rawHex=payload.hex(),packageOffset=base_offset+tagstart))
 return props
