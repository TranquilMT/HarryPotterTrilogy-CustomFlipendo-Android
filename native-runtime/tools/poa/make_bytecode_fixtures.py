import pathlib,argparse,struct
from inspect_packages import inspect
from object_payloads import function_header
from bytecode import Decoder
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('output');a=p.parse_args();groups=[];count=0
 for path in sorted((pathlib.Path(a.game)/'system').glob('*.u')):
  m=inspect(path);data=path.read_bytes();cases=[]
  for e in m['exports']:
   if e['class']!='Function':continue
   raw=data[e['offset']:e['offset']+e['size']];r,h=function_header(raw,m['names']);d=Decoder(r,m['names'],len(m['imports']),len(m['exports']),h['declaredBytecodeMemorySize']);d.decode()
   native=int.from_bytes(r.read(2),'little');precedence=r.read(1)[0];flags=r.u32()
   if flags&0x40:r.read(2)
   assert r.p==len(raw);cases.append((raw,d,native,precedence,flags))
  if cases:groups.append((m,cases));count+=len(cases)
 with open(a.output,'wb') as out:
  def u32(n):out.write(struct.pack('<I',n))
  def blob(b):u32(len(b));out.write(b)
  u32(len(groups))
  for m,cases in groups:
   u32(len(m['names']))
   for n in m['names']:blob(n.encode('utf8'))
   u32(len(m['imports']));u32(len(m['exports']));u32(len(cases))
   for raw,d,native,precedence,flags in cases:
    blob(raw);blob(d.expanded);u32(len(d.tokens))
    for t in d.tokens:
     for k in ['offset','serializedOffset','end','depth']:u32(t[k])
     u32(t['opcode'])
    u32(len(d.jumps));u32(native);u32(precedence);u32(flags)
 print('Full-function fixtures',count,'packages',len(groups),flush=True)
if __name__=='__main__':main()

