#!/usr/bin/env python3
"""Package HD modules into a signed APK, reusing compressed game/runtime assets.
Requires a verified 0.3.0 base APK, the twelve profile-labelled HD module ZIPs,
Android SDK 36.0.0/platform 36, Java 17, and the existing private signing key.
"""
import argparse,pathlib,subprocess,zipfile,hashlib,shutil,struct,copy,json
p=argparse.ArgumentParser();p.add_argument('--sdk',required=True);p.add_argument('--base-apk',required=True);p.add_argument('--packs',required=True);p.add_argument('--keystore',required=True);p.add_argument('--password-file',required=True);p.add_argument('--output',required=True);a=p.parse_args()
r=pathlib.Path(__file__).resolve().parent;sdk=pathlib.Path(a.sdk).resolve();bt=sdk/'build-tools/36.0.0';jar=sdk/'platforms/android-36/android.jar';stage=r/'bundled-hd-build';stage.mkdir(exist_ok=True);classes=stage/'classes';shutil.rmtree(classes,ignore_errors=True);classes.mkdir();out=pathlib.Path(a.output).resolve();out.parent.mkdir(parents=True,exist_ok=True)
def run(args):subprocess.run(list(map(str,args)),check=True)
sources=sorted((r/'java').rglob('*.java'))+sorted((r/'deps/SDL/android-project/app/src/main/java/org/libsdl/app').glob('*.java'))
run(['java','-m','jdk.compiler/com.sun.tools.javac.Main','-source','8','-target','8','-Xlint:-options','-cp',jar,'-d',classes,*sources]);arg=stage/'classes.list';arg.write_text('\n'.join(str(f) for f in sorted(classes.rglob('*.class'))));run([bt/'d8','--lib',jar,'--min-api','29','--output',stage,'@'+str(arg)])
run([bt/'aapt2','compile','--dir',r/'res','-o',stage/'res.zip']);run([bt/'aapt2','link','-o',stage/'base.apk','--manifest',r/'AndroidManifest.xml','-I',jar,'-R',stage/'res.zip','--min-sdk-version','29','--target-sdk-version','35','--version-code','6','--version-name','0.3.1-HD-bundled'])
unsigned=stage/'unsigned.apk';shutil.copyfile(stage/'base.apk',unsigned);modules=[];native={}
with zipfile.ZipFile(a.base_apk) as old,open(a.base_apk,'rb') as raw,zipfile.ZipFile(unsigned,'a',zipfile.ZIP_DEFLATED,allowZip64=False) as z:
 # Preserve the previously verified compressed assets and native binaries verbatim.
 for info in old.infolist():
  if not info.filename.startswith(('assets/','lib/')) or info.filename=='assets/build-info.txt' or info.filename.startswith('assets/bundled-hd/'):continue
  assert not info.flag_bits&8
  raw.seek(info.header_offset);header=raw.read(30);n,e=struct.unpack_from('<HH',header,26);raw.seek(info.header_offset);remaining=30+n+e+info.compress_size;new=copy.copy(info);new.header_offset=z.fp.tell()
  while remaining:
   data=raw.read(min(1024*1024,remaining));assert data;z.fp.write(data);remaining-=len(data)
  z.filelist.append(new);z.NameToInfo[new.filename]=new;z.start_dir=z.fp.tell();z._didModify=True
  if info.filename.startswith('lib/'):native[info.filename]=hashlib.sha256(old.read(info)).hexdigest()
 z.write(stage/'classes.dex','classes.dex');z.writestr('assets/build-info.txt','Harry Potter TQ 0.3.1: HP1 and HP2 HD texture modules embedded and optional via per-game launcher switches. Default.ini repair; POA disabled. Native runtime unchanged.\n')
 for game in ('HP1','HP2'):
  packs=sorted(pathlib.Path(a.packs).glob(game+'-HD-Refresh-0*-v0.3.zip'));assert len(packs)==6;stamp=hashlib.sha256()
  for pack in packs:
   with zipfile.ZipFile(pack) as check:meta=json.loads(check.read('pack.json'));assert meta['game']==game
   sha=hashlib.file_digest(pack.open('rb'),'sha256').hexdigest();stamp.update(sha.encode());name='assets/bundled-hd/'+game.lower()+'/'+pack.name
   # The modules are already compressed ZIPs; store without a second compression layer.
   z.write(pack,name,compress_type=zipfile.ZIP_STORED);modules.append(dict(path=name,sha256=sha,textures=len(meta['textures']),bytes=pack.stat().st_size));print('Embedded',pack.name,flush=True)
  z.writestr('assets/bundled-hd/'+game.lower()+'/.stamp',stamp.hexdigest())
with zipfile.ZipFile(unsigned) as check:assert check.testzip() is None
run([bt/'zipalign','-f','-P','16','4',unsigned,stage/'aligned.apk']);run([bt/'apksigner','sign','--ks',pathlib.Path(a.keystore).resolve(),'--ks-key-alias','hp-native','--ks-pass','file:'+str(pathlib.Path(a.password_file).resolve()),'--out',out,stage/'aligned.apk']);run([bt/'apksigner','verify','--verbose','--print-certs',out]);run([bt/'zipalign','-c','-P','16','4',out])
with zipfile.ZipFile(out) as z:
 assert z.testzip() is None
 for n,h in native.items():assert hashlib.sha256(z.read(n)).hexdigest()==h
 for m in modules:
  with z.open(m['path']) as f:assert hashlib.file_digest(f,'sha256').hexdigest()==m['sha256']
 assert z.read('classes.dex')==(stage/'classes.dex').read_bytes()
report=dict(version='0.3.1-HD-bundled',bytes=out.stat().st_size,sha256=hashlib.file_digest(out.open('rb'),'sha256').hexdigest(),embeddedModules=modules,totalTextures=sum(m['textures'] for m in modules),nativeLibrariesUnchanged=native,deviceExecutionTested=False)
out.with_suffix('.verification.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2),flush=True)
