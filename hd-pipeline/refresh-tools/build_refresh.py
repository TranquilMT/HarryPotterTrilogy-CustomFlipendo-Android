"""Build game DDS replacement modules from the recovered v0.1 texture backup.
Requires Pillow, numpy and opencv-contrib-python-headless. Run from project root.
This recovery build uses decoded v0.1 DDS, not pristine disc pixels.
"""
import pathlib,json,sys,io,hashlib,struct,time
from concurrent.futures import ProcessPoolExecutor
import cv2,numpy as np
from PIL import Image
ROOT=pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/'texture-base/tools'))
from build_hd_pack import dds_chain
OUT=ROOT/'refresh-work'
def category(t):
 p=t['path'].lower();pkg=p.split('/')[1];n=p.split('/')[-1]
 if pkg=='harrypotter' and n.startswith(('skharry','skrem','sksmall','skquid')):return '01-Harry'
 if pkg in ('hp_skins','characters') or (pkg=='harrypotter' and n.startswith('sk') and not any(x in n for x in ('boulder','log','statue'))):return '02-NPCs-Creatures'
 if pkg in ('hpmenu','hpfonts','uwindow','uwindowfonts','storybooktest','menuart','hp_logo','ubrowser'):return '06-Interface-Storybooks'
 if pkg in ('genfx','hpparticle','hp_fx','particles','fractalfx','flarefx','fireeng','hp_water','palettes'):return '05-Effects'
 if pkg in ('hprops','dprops','hpmodels','hpbase','detail','hp_portraits2','hp_portrait') or any(x in p for x in ('/detail/','/skins/','/skin/')):return '04-Props-Details'
 return '03-Environment'
def init():
 global net
 cv2.setNumThreads(1);net=cv2.dnn_superres.DnnSuperResImpl_create();net.readModel(str(ROOT/'refresh-tools/FSRCNN_x4.pb'));net.setModel('fsrcnn',4)
def process(t):
 cat=category(t);src=ROOT/'texture-base'/t['path'];im=Image.open(src).convert('RGBA');w,h=im.size
 # Estimate original signal from the only complete saved texture backup.
 small=im.convert('RGB').resize(tuple(t['originalSize']),Image.Resampling.LANCZOS)
 ar=np.asarray(small);pad=np.pad(ar,((8,8),(8,8),(0,0)),mode='reflect')
 nn=net.upsample(cv2.cvtColor(pad,cv2.COLOR_RGB2BGR))[32:-32,32:-32]
 nn=Image.fromarray(cv2.cvtColor(nn,cv2.COLOR_BGR2RGB)).resize((w,h),Image.Resampling.LANCZOS)
 base=np.asarray(im.convert('RGB')).astype(np.float32)
 neural=np.asarray(nn).astype(np.float32)
 strength=.42 if cat in ('05-Effects','06-Interface-Storybooks') else .78
 refreshed=np.clip(base+np.clip(neural-base,-22,22)*strength,0,255).astype(np.uint8)
 out=Image.fromarray(refreshed).convert('RGBA');out.putalpha(im.getchannel('A'))
 dest=OUT/cat/t['path'];dest.parent.mkdir(parents=True,exist_ok=True);data,levels=dds_chain(out);dest.write_bytes(data)
 check=Image.open(dest);assert check.size==(w,h)
 if t['path'] in SAMPLES:
  d=OUT/'samples';d.mkdir(exist_ok=True);im.save(d/(cat+'-before.png'));out.save(d/(cat+'-after.png'))
 return {'path':t['path'],'category':cat,'size':[w,h],'mips':levels,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'baseSHA256':hashlib.sha256(src.read_bytes()).hexdigest(),'meanPixelChange':float(np.abs(refreshed.astype(float)-base).mean())}
SAMPLES=set()
def main():
 m=json.loads((ROOT/'texture-base/texture-manifest.json').read_text());rows={t['path']:t for p in m['packages'] for t in p['textures']};OUT.mkdir(exist_ok=True)
 start=time.time();results=[]
 with ProcessPoolExecutor(max_workers=4,initializer=init) as pool:
  for i,r in enumerate(pool.map(process,rows.values()),1):
   results.append(r)
   if i%100==0:print(i,'/',len(rows),round(time.time()-start),'seconds',flush=True)
 manifest={'version':'0.2-recovery','method':'FSRCNN x4 bounded reconstruction blended with recovered v0.1 DDS; alpha preserved before BC3 recompression','inputLimitation':'Complete input is recovered lossy v0.1 DDS, downsampled to original dimensions; not pristine original disc pixels.','modelSHA256':hashlib.sha256((ROOT/'refresh-tools/FSRCNN_x4.pb').read_bytes()).hexdigest(),'textures':results,'count':len(results)}
 (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2));print('COMPLETE',len(results),flush=True)
if __name__=='__main__':main()
