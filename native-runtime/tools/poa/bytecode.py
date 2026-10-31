"""HP3 UE2 bytecode decoding, not execution. Offsets are VM-memory offsets."""
import struct,json,pathlib,argparse,collections
from inspect_packages import inspect
from object_payloads import function_header
class DecodeError(ValueError):pass
class Decoder:
 def __init__(self,r,names,imports,exports,size):
  self.r=r;self.names=names;self.imports=imports;self.exports=exports;self.limit=size;self.memory=0;self.start=r.p;self.tokens=[];self.jumps=[];self.nativeCalls=collections.Counter();self.calls=[];self.expanded=bytearray()
 def raw(self,n):
  if self.memory+n>self.limit:raise DecodeError('Operand exceeds bytecode memory size')
  b=self.r.read(n);self.memory+=n;self.expanded.extend(b);return b
 def byte(self):return self.raw(1)[0]
 def word(self):return int.from_bytes(self.raw(2),'little')
 def dword(self):return int.from_bytes(self.raw(4),'little')
 def index(self,kind='object'):
  n=self.r.compact();self.memory+=4
  if self.memory>self.limit:raise DecodeError('Reference exceeds memory size')
  if kind=='name':
   if not 0<=n<len(self.names):raise DecodeError('Name reference outside table')
  elif n>self.exports or n < -self.imports:raise DecodeError('Object reference outside table')
  self.expanded.extend(struct.pack('<i',n));return n
 def expr(self,depth=0):
  if depth>=128 or len(self.tokens)>=1000000:raise DecodeError('Expression limit exceeded')
  start=self.memory;serialized=self.r.p-self.start;op=self.byte();node={'offset':start,'serializedOffset':serialized,'opcode':op,'depth':depth};self.tokens.append(node)
  def sub():return self.expr(depth+1)
  def parms():
   while sub()!=0x16:pass
  if op>=0x60:
   native=op if op>=0x70 else ((op-0x60)<<8)|self.byte();node['nativeIndex']=native;self.nativeCalls[str(native)]+=1;parms()
  elif op in (0,1,2,0x20,0x29):node['reference']=self.index()
  elif op==4:sub()
  elif op==5:self.byte();sub()
  elif op in (6,7):
   target=self.word();self.jumps.append({'from':start,'target':target,'kind':'jump'});node['target']=target
   if op==7:sub()
  elif op==9:self.word();sub()
  elif op==0x0a:
   target=self.word();node['target']=target
   if target!=65535:self.jumps.append({'from':start,'target':target,'kind':'case'});sub()
  elif op==0x0c:
   for _ in range(100000):
    name=self.index('name');target=self.dword()
    if self.names[name]=='None':break
    self.jumps.append({'from':start,'target':target,'kind':'label'})
   else:raise DecodeError('Too many labels')
  elif op in (0x0d,0x0e,0x2d,0x37):sub()
  elif op in (0x0f,0x10,0x14,0x1a,0x45):sub();sub()
  elif op==0x11:
   for _ in range(4):sub()
  elif op in (0x12,0x19):sub();self.word();self.byte();sub()
  elif op in (0x13,0x2e):self.index();sub()
  elif op==0x18:self.word();sub()
  elif op in (0x1b,0x38):
   n=self.index('name');node['functionName']=self.names[n];self.calls.append({'offset':start,'kind':'name','name':self.names[n]});parms()
  elif op==0x1c:
   ref=self.index();node['reference']=ref;self.calls.append({'offset':start,'kind':'final','reference':ref});parms()
  elif op in (0x1d,0x1e):self.raw(4)
  elif op==0x1f:
   while self.byte():pass
  elif op==0x21:self.index('name')
  elif op in (0x22,0x23):self.raw(12)
  elif op in (0x24,0x2c):self.byte()
  elif op==0x2f:
   sub();target=self.word();self.jumps.append({'from':start,'target':target,'kind':'iterator'})
  elif op in (0x32,0x33):self.index();sub();sub()
  elif op==0x34:
   while self.word():pass
  elif op==0x35:self.raw(8)
  elif op==0x36:self.index();sub()
  elif op==0x39:self.byte();sub()
  elif op==0x3a:self.index()
  elif op in (0x3b,0x3c,0x3d,0x3e):sub();sub()
  elif op in (0x40,0x41):sub();sub();sub()
  elif op==0x42:self.raw(13)
  elif op==0x43:self.index();self.index('name');parms()
  elif op==0x44:self.index('name')
  elif op==0x48:self.word();sub();self.word();sub();sub()
  elif op not in (8,0x0b,0x16,0x17,0x25,0x26,0x27,0x28,0x2a,0x30,0x31,0x3f,0x47):raise DecodeError(f'Unsupported opcode 0x{op:02x} at memory {start}, serialized {serialized}')
  node['end']=self.memory;return op
 def decode(self):
  while self.memory<self.limit:self.expr()
  if self.memory!=self.limit:raise DecodeError('Bytecode memory size mismatch')
  boundaries={t['offset'] for t in self.tokens if t['depth']==0}|{self.memory}
  invalid=[j for j in self.jumps if j['target'] not in boundaries]
  if invalid:raise DecodeError('Jump target is not an expression boundary: '+str(invalid[0]))
  return {'memoryBytes':self.memory,'serializedBytes':self.r.p-self.start,'tokens':len(self.tokens),'jumpCount':len(self.jumps),'nativeCalls':dict(self.nativeCalls),'calls':self.calls,'offsetMap':self.tokens,'jumps':self.jumps}
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True);ok=[];errors=[];ops=collections.Counter()
 for path in sorted((root/'system').glob('*.u')):
  m=inspect(path);data=path.read_bytes()
  for i,e in enumerate(m['exports'],1):
   if e['class']!='Function':continue
   row={'package':path.name,'object':e['name'],'exportIndex':i}
   try:
    r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);d=Decoder(r,m['names'],len(m['imports']),len(m['exports']),h['declaredBytecodeMemorySize']);result=d.decode()
    native=int.from_bytes(r.read(2),'little');precedence=r.read(1)[0];flags=r.u32()
    if flags&0x40:r.read(2)
    if r.p!=len(r.data):raise DecodeError('Unconsumed function export bytes')
    ops.update(f'{t["opcode"]:02x}' for t in d.tokens);row.update({k:v for k,v in result.items() if k!='offsetMap'});row.update(nativeIndex=native,flags=flags);ok.append(row)
   except Exception as ex:errors.append(dict(row,error=str(ex)))
 (out/'functions.json').write_text(json.dumps(ok,indent=2));(out/'errors.json').write_text(json.dumps(errors,indent=2));(out/'opcodes.json').write_text(json.dumps(dict(ops),indent=2));print('Decoded',len(ok),'errors',len(errors),flush=True)
if __name__=='__main__':main()
