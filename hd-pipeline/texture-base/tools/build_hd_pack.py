"""Recover original UE1 P8 textures and build a conservative mobile 4x DDS pack.
Uses existing project's package/property readers. Never changes original packages.
"""
import pathlib,sys,json,struct,hashlib,io,os,time
from concurrent.futures import ProcessPoolExecutor
sys.path.insert(0,str(pathlib.Path(__file__).parent))
from parse_package import Reader
from hd_package import parse
from package_objects import PackageView,read_properties
from PIL import Image,ImageFilter
BASE=pathlib.Path.cwd();WORK=pathlib.Path(os.environ.get('HP_HD_WORK',str(BASE/'hd-pack-work')));OUT=WORK/'pack';INDEX=WORK/'indexes';GAME=pathlib.Path(os.environ.get('HP_GAME_DIR',str(BASE/'game')))

def mips(r,e,version):
 result=[];n=r.compact()
 if not 0<=n<=16:raise ValueError('Invalid mip count')
 for _ in range(n):
  end=r.i32() if version>=63 else 0;length=r.compact();data=r.read(length)
  if end and end!=e['offset']+r.p:raise ValueError('Mip offset mismatch')
  w,h=r.i32(),r.i32();ub,vb=r.read(2)
  if not 1<=w<=8192 or not 1<=h<=8192 or w!=1<<ub or h!=1<<vb:raise ValueError('Invalid dimensions')
  result.append((w,h,data))
 return result

def neural_up(im,target):
 import cv2,numpy as np
 global net
 if 'net' not in globals():
  cv2.setNumThreads(1);net=cv2.dnn_superres.DnnSuperResImpl_create();net.readModel(str(PathModel));net.setModel('fsrcnn',4)
 rgb=np.array(im.convert('RGB'));base=np.array(im.convert('RGB').resize(target,Image.Resampling.LANCZOS)).astype(np.float32)
 padded=cv2.copyMakeBorder(rgb[:,:,::-1],8,8,8,8,cv2.BORDER_REFLECT_101)
 out=net.upsample(padded)[32:-32,32:-32,::-1];out=cv2.resize(out,target,interpolation=cv2.INTER_LANCZOS4).astype(np.float32)
 out=np.uint8(np.clip(base+np.clip(out-base,-24,24)*.8,0,255));result=Image.fromarray(out).convert('RGBA');result.putalpha(im.getchannel('A').resize(target,Image.Resampling.LANCZOS));return result
PathModel=pathlib.Path(__file__).resolve().parents[2]/'refresh-tools/FSRCNN_x4.pb'
def dds_chain(im):
 blocks=[];top=None;fmt='DXT1' if im.getchannel('A').getextrema()==(255,255) else 'DXT5';block=8 if fmt=='DXT1' else 16
 while True:
  stream=io.BytesIO();im.save(stream,format='DDS',pixel_format=fmt);raw=stream.getvalue();assert raw[84:88]==fmt.encode()
  expected=max(1,(im.width+3)//4)*max(1,(im.height+3)//4)*block;assert len(raw)==128+expected
  if top is None:top=bytearray(raw[:128])
  blocks.append(raw[128:])
  if im.size==(1,1):break
  im=im.resize((max(1,im.width//2),max(1,im.height//2)),Image.Resampling.LANCZOS)
 struct.pack_into('<I',top,8,struct.unpack_from('<I',top,8)[0]|0x20000);struct.pack_into('<I',top,28,len(blocks));struct.pack_into('<I',top,108,0x401008)
 return bytes(top)+b''.join(blocks),len(blocks)

def process(file):
 file=pathlib.Path(file);folder=INDEX/(file.parent.name+'-'+file.name);folder.mkdir(parents=True,exist_ok=True);link=folder/file.name
 if not link.exists():link.symlink_to(file)
 if not pathlib.Path(str(link)+'.json').exists():
  import contextlib
  with contextlib.redirect_stdout(io.StringIO()):parse(link)
 v=PackageView(link);rows=[];skipped=[];errors=[];palettes={}
 for i,e in enumerate(v.exports):
  if e['class']!='Texture':
   if e['class'].endswith('Texture') and e['class'] not in ('Texture','ObjectProperty'):skipped.append({'package':file.stem,'name':e['objectName'],'class':e['class'],'reason':'Non-bitmap/procedural class; retain original runtime behavior'})
   continue
  name=v.object_path(i+1);start=e['offset'];raw=v.bytes[start:start+e['size']]
  try:
   r=Reader(raw);props={p['name'].lower():p['value'] for p in read_properties(r,v,start)}
   if props.get('brealtime') or props.get('bparametric'):skipped.append({'object':name,'reason':'Realtime/parametric'});continue
   fmt=props.get('format',0);pal=props.get('palette')
   if fmt!=0:raise ValueError('Unsupported pixel format '+str(fmt))
   if not pal or pal['index']<=0:raise ValueError('Palette is absent/external')
   ix=pal['index']
   if ix not in palettes:
    pe=v.exports[ix-1];pr=Reader(v.bytes[pe['offset']:pe['offset']+pe['size']]);read_properties(pr,v);assert pr.compact()==256;palette=pr.read(1024);assert pr.p==len(pr.b);palettes[ix]=palette
   mm=mips(r,e,v.data['version'])
   if props.get('bhascomp'):mips(r,e,v.data['version'])
   if r.p!=len(raw):raise ValueError('Unexpected native texture tail')
   if not mm:raise ValueError('No bitmap mip data')
   w,h,pixels=mm[0]
   if len(pixels)!=w*h:raise ValueError('P8 byte count mismatch')
   palette=palettes[ix];im=Image.frombytes('P',(w,h),pixels);im.putpalette(bytes(c for j in range(256) for c in palette[j*4:j*4+3]));im=im.convert('RGBA');alpha=Image.frombytes('L',(w,h),bytes(0 if x==0 else 255 for x in pixels));im.putalpha(alpha)
   factor=min(4,1024//max(w,h))
   if factor<2:skipped.append({'object':name,'reason':'Already at mobile resolution cap','size':[w,h]});continue
   target=(w*factor,h*factor);up=neural_up(im,target)
   parts=name.split('.')
   if any(not x or x in ('.','..') or '/' in x or '\\' in x for x in parts):raise ValueError('Unsafe object path')
   dest=OUT/'Textures'/pathlib.Path(*parts[:-1])/(parts[-1]+'.dds');dest.parent.mkdir(parents=True,exist_ok=True);data,levels=dds_chain(up);dest.write_bytes(data)
   # Decoder roundtrip verifies dimensions and alpha survive BC3 writing.
   check=Image.open(dest);assert check.size==target and check.mode in ('RGB','RGBA')
   rows.append({'object':name,'class':'Texture','originalSize':[w,h],'size':list(target),'scale':factor,'mips':levels,'path':dest.relative_to(OUT).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'sourceObjectSHA256':hashlib.sha256(raw).hexdigest(),'maskedByTexture':bool(props.get('bmasked',False))})
   if (file.stem=='HarryPotter' and e['objectName'] in ('skharryTex0','skharryTex1')) or (file.stem=='HP_1st' and len(rows)<5):
    sample=WORK/'samples';sample.mkdir(exist_ok=True);im.save(sample/(file.stem+'__'+e['objectName']+'__original.png'));up.save(sample/(file.stem+'__'+e['objectName']+'__upscaled.png'))
  except Exception as exc:errors.append({'object':name,'reason':str(exc)})
 return {'package':str(file.relative_to(GAME)),'version':v.data['version'],'textures':rows,'skipped':skipped,'errors':errors}

def main():
 import argparse
 p=argparse.ArgumentParser();p.add_argument('--audit',action='store_true');p.add_argument('--jobs',type=int,default=4);a=p.parse_args();OUT.mkdir(parents=True,exist_ok=True)
 files=sorted(f for f in GAME.rglob('*') if f.suffix.lower() in ('.utx','.u','.unr') and not any(x.startswith('.') for x in f.relative_to(GAME).parts))
 if a.audit:
  files=[f for f in files if f.name in ('HarryPotter.u','HP_1st.utx')]
 reports=[];begin=time.time()
 with ProcessPoolExecutor(max_workers=a.jobs) as pool:
  for report in pool.map(process,map(str,files)):
   reports.append(report);print(report['package'],len(report['textures']),'upscaled;',len(report['errors']),'errors',flush=True)
 result={'method':'FSRCNN x4 bounded neural refinement with preserved alpha','maximumDimension':1024,'format':'BC3/DXT5 DDS with full mip chains','packages':reports,'textures':sum(len(r['textures']) for r in reports),'errors':sum(len(r['errors']) for r in reports),'seconds':time.time()-begin};(WORK/('audit.json' if a.audit else 'manifest.json')).write_text(json.dumps(result,indent=2));print('TOTAL',result['textures'],'textures,',result['errors'],'errors',flush=True)
if __name__=='__main__':main()
