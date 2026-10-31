import pathlib,struct
p=pathlib.Path('/tmp/iso-fixtures');p.mkdir(exist_ok=True)
def record(name,lba,size,directory=False):
 b=name if isinstance(name,bytes) else name.encode();n=33+len(b)+(len(b)%2==0);r=bytearray(n);r[0]=n;r[2:6]=struct.pack('<I',lba);r[6:10]=struct.pack('>I',lba);r[10:14]=struct.pack('<I',size);r[14:18]=struct.pack('>I',size);r[25]=2 if directory else 0;r[28:32]=b'\x01\x00\x00\x01';r[32]=len(b);r[33:33+len(b)]=b;return r
b=bytearray(30*2048)
for lba,typ in ((16,1),(17,255)):b[lba*2048]=typ;b[lba*2048+1:lba*2048+6]=b'CD001';b[lba*2048+6]=1
b[16*2048+128:16*2048+132]=b'\x00\x08\x08\x00';root=record(b'\0',20,2048,True);b[16*2048+156:16*2048+156+len(root)]=root
rd=record('System',21,2048,True)+record('Maps',22,2048,True);b[20*2048:20*2048+len(rd)]=rd
rd=record('HP.exe;1',23,3)+record('HarryPotter.u;1',24,4);b[21*2048:21*2048+len(rd)]=rd
rd=record('test.unr;1',25,5);b[22*2048:22*2048+len(rd)]=rd
b[23*2048:23*2048+3]=b'EXE';b[24*2048:24*2048+4]=b'PACK';b[25*2048:25*2048+5]=b'MAP!!';(p/'valid.iso').write_bytes(b)
c=bytearray(b);c[21*2048+2:21*2048+6]=struct.pack('<I',99999);(p/'outside.iso').write_bytes(c)
c=bytearray(b);c[20*2048+32]=2;c[20*2048+33:20*2048+35]=b'..';(p/'traversal.iso').write_bytes(c)
(p/'truncated.iso').write_bytes(b[:40000])
