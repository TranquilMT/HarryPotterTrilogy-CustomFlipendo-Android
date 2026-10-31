import pathlib,json,hashlib,io,struct,zipfile
from concurrent.futures import ProcessPoolExecutor
from PIL import Image
ROOT=pathlib.Path(__file__).resolve().parent.parent;WORK=ROOT/'refresh-work';OUT=ROOT/'HP1-HD-Refresh-v0.2'
def optimize(t):
 p=WORK/t['category']/t['path'];im=Image.open(p).convert('RGBA');fmt='DXT1' if im.getchannel('A').getextrema()==(255,255) else 'DXT5'
 if fmt=='DXT1':
  blocks=[];head=None
  while True:
   b=io.BytesIO();im.save(b,format='DDS',pixel_format=fmt);raw=b.getvalue()
   if head is None:head=bytearray(raw[:128])
   blocks.append(raw[128:])
   if im.size==(1,1):break
   im=im.resize((max(1,im.width//2),max(1,im.height//2)),Image.Resampling.LANCZOS)
  struct.pack_into('<I',head,8,struct.unpack_from('<I',head,8)[0]|0x20000);struct.pack_into('<I',head,28,len(blocks));struct.pack_into('<I',head,108,0x401008)
  p.write_bytes(bytes(head)+b''.join(blocks))
 data=p.read_bytes();assert data[84:88]==fmt.encode();w,h=struct.unpack_from('<II',data,12)[::-1];n=struct.unpack_from('<I',data,28)[0];size=128
 for _ in range(n):size+=max(1,(w+3)//4)*max(1,(h+3)//4)*(8 if fmt=='DXT1' else 16);w=max(1,w//2);h=max(1,h//2)
 assert len(data)==size and (w,h)==(1,1)
 check=Image.open(p);assert list(check.size)==t['size']
 t.update(format=fmt,bytes=len(data),sha256=hashlib.sha256(data).hexdigest());return t
README='''HP1 HD Refresh v0.2 — recovery edition

3,505 unique bitmap textures processed: Harry, NPCs/creatures, environment,
props/details, effects and interface/storybooks. Power-of-two sizes up to
1024 pixels, original UV layouts and aspect ratios; complete mip chains.
BC1 for fully opaque assets, BC3 for alpha assets. Not a 1080p render setting.

Actual FSRCNN x4 neural inference with a bounded blend; no newly painted
objects or claimed photoreal remaster. This edition is derived from the
saved v0.1 DDS backup, reconstructed at original dimensions before inference.
It is NOT built from pristine disc pixel data. Lost source detail cannot be
guaranteed recovered. Character mesh geometry, animations, audio and 217
procedural/realtime texture objects remain original.

INSTALL: the six module ZIPs together contain the complete texture set.
Use the launcher's HD textures option, choose a module ZIP, then Play.
Wait for import. Keep each download accessible during setup.
The saved 0.1.1 importer adds each selected module; import one, Play to finish
installation, then return and repeat for the next. A future launcher may
offer multiple selection. Disabling HD removes installed overrides.

Original game files are not replaced. This pack does not repair game startup.
Runtime appearance, performance and gameplay remain unverified on a phone.
Use only with your game copy. No Chamber of Secrets assets are included.
'''
def writezip(name,rows):
 p=OUT/name
 with zipfile.ZipFile(p,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  z.writestr('README.txt',README);z.writestr('pack.json',json.dumps({'version':'0.2-recovery','textures':rows},indent=2))
  for t in rows:z.write(WORK/t['category']/t['path'],t['path'])
 with zipfile.ZipFile(p) as z:assert z.testzip() is None
 print(name,p.stat().st_size,flush=True)
 return p
def main():
 OUT.mkdir(exist_ok=True);m=json.loads((WORK/'manifest.json').read_text())
 with ProcessPoolExecutor(max_workers=4) as pool:rows=list(pool.map(optimize,m['textures']))
 assert len(rows)==3505 and len({t['path'] for t in rows})==3505
 m['textures']=rows;(OUT/'manifest.json').write_text(json.dumps(m,indent=2));(OUT/'README.txt').write_text(README)
 files=[]
 for cat in sorted({t['category'] for t in rows}):files.append(writezip('HP1-HD-Refresh-'+cat+'-v0.2.zip',[t for t in rows if t['category']==cat]))
 assert all(p.stat().st_size<700_000_000 for p in files)
 (OUT/'SHA256.txt').write_text('\n'.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name for p in files)+'\n')
 print('ALL VERIFIED',len(rows),flush=True)
if __name__=='__main__':main()
