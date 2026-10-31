from pathlib import Path
import zipfile,json,struct,copy,hashlib,time
out=Path('apk-work/deliverables');out.mkdir(exist_ok=True)
def raw_repack(src,dst):
 with zipfile.ZipFile(src) as old,zipfile.ZipFile(dst,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z,open(src,'rb') as raw:
  meta=json.loads(old.read('pack.json'));meta['game']='HP1';meta['version']='0.3-launcher';z.writestr('pack.json',json.dumps(meta,indent=2));z.writestr('README.txt','HP1 HD refresh module. In Harry Potter TQ 0.3.0 select HP1 > Manage HD textures > Install HD pack ZIPs. Original game files remain unchanged. This release adds explicit game compatibility metadata to the existing v0.2 DDS assets.\n')
  for info in old.infolist():
   if info.filename in ('pack.json','README.txt','README.md'):continue
   assert not info.flag_bits&8
   raw.seek(info.header_offset);header=raw.read(30);n,e=struct.unpack_from('<HH',header,26);raw.seek(info.header_offset);data=raw.read(30+n+e+info.compress_size);assert len(data)==30+n+e+info.compress_size
   new=copy.copy(info);new.header_offset=z.fp.tell();z.fp.write(data);z.filelist.append(new);z.NameToInfo[new.filename]=new;z.start_dir=z.fp.tell();z._didModify=True
 with zipfile.ZipFile(dst) as z:
  assert z.testzip() is None
  for row in meta['textures']:assert hashlib.sha256(z.read(row['path'])).hexdigest()==row['sha256']
 return len(meta['textures'])
for src in sorted(Path('apk-work/hd-restored').glob('HP1-HD-Refresh-0*.zip')):
 dst=out/src.name.replace('v0.2','v0.3');print(dst.name,raw_repack(src,dst),flush=True)
# Wait for the original-asset neural pass, then package all successfully decoded texture exports.
manifest=Path('apk-work/hp2-hd-work/manifest.json')
while not manifest.exists():time.sleep(2)
data=json.loads(manifest.read_text());assert data['errors']==0,data['errors'];rows=[row for p in data['packages'] for row in p['textures']];assert len(rows)>3000
cats=['01-Harry','02-NPCs-Creatures','03-Environment','04-Props-Details','05-Effects','06-Interface-Storybooks']
def category(row):
 obj=row['object'].lower();pkg=obj.split('.')[0];name=obj.split('.')[-1]
 if pkg=='hpmodels':return cats[0] if name.startswith('skharry') else cats[1]
 if pkg in ('hgame','uwindow','menuart','hp_logo','3dwizardcards') or 'font' in obj or '.icons.' in obj:return cats[5]
 if pkg in ('hpparticle','diffindo','skurge','spongify','rictusempra','genfx','genfluid','hp_fx','particles','palettes','fireeng'):return cats[4]
 if pkg=='hprops' or 'detail' in pkg or 'portrait' in pkg:return cats[3]
 return cats[2]
for cat in cats:
 selected=[r for r in rows if category(r)==cat];dst=out/('HP2-HD-Refresh-'+cat+'-v0.3.zip')
 with zipfile.ZipFile(dst,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  z.writestr('pack.json',json.dumps({'game':'HP2','version':'0.3','category':cat,'method':data['method'],'textures':selected},indent=2));z.writestr('README.txt','HP2 only. Select Chamber of Secrets > Manage HD textures > Install HD pack ZIPs. Conservative FSRCNN x4 detail refinement, up to 1024 pixels per texture, full mip chains, original alpha retained. Procedural textures continue to use the original engine. Optional modules can be removed in the same screen.\n')
  for row in selected:
   f=Path('apk-work/hp2-hd-work/pack')/row['path'];assert hashlib.sha256(f.read_bytes()).hexdigest()==row['sha256'];z.write(f,row['path'])
 with zipfile.ZipFile(dst) as z:assert z.testzip() is None
 print(dst.name,len(selected),dst.stat().st_size,flush=True)
report={'HP1':{},'HP2':{},'HP2ProceduralRetained':sum(len(p['skipped']) for p in data['packages']),'HP2SourceDecodeErrors':data['errors']}
for f in sorted(out.glob('HP*-HD-Refresh-0*.zip')):
 with zipfile.ZipFile(f) as z:m=json.loads(z.read('pack.json'));report[m['game']][f.name]={'textures':len(m['textures']),'bytes':f.stat().st_size}
(out/'HD-texture-verification.json').write_text(json.dumps(report,indent=2))
with zipfile.ZipFile(out/'HP1-HP2-HD-Texture-Collection-v0.3.zip','w',zipfile.ZIP_STORED) as z:
 z.writestr('README.txt','Harry Potter TQ 0.3.0 HD texture collection\n\nSelect a game in the launcher, open Manage HD textures, then choose this entire ZIP or individual module ZIPs. Only modules marked for the selected game are installed. Repeat for the other game. All six modules cover characters, creatures, environment, props/details, effects, and interface/storybooks. The APK includes HP2 original data; HP1 uses your existing installation/import. HD modules are optional and require extra storage and memory.\n\nTextures are refreshed up to 4x, capped at 1024 pixels per texture. This is independent of the game screen resolution; 1080p is not a texture size. Animated/procedural effects retain original engine behavior. HP1 DDS assets are unchanged from v0.2 with explicit HP1 compatibility metadata; HP2 was rebuilt from original assets using bounded FSRCNN neural refinement.\n')
 for f in sorted(out.glob('HP*-HD-Refresh-0*.zip')):z.write(f,('HP1/' if f.name.startswith('HP1') else 'HP2/')+f.name)
print('COLLECTION COMPLETE',flush=True)
