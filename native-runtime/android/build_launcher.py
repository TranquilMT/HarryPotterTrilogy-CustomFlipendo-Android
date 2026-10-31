#!/usr/bin/env python3
"""Build launcher release using verified unchanged native libraries from a prior APK.
Private signing files and original game data are supplied separately, never in source.
"""
import argparse,pathlib,subprocess,os,shutil,zipfile,hashlib,json
p=argparse.ArgumentParser();p.add_argument('--sdk',required=True);p.add_argument('--runtime-apk',required=True);p.add_argument('--hp2-data',required=True);p.add_argument('--keystore',required=True);p.add_argument('--password-file',required=True);p.add_argument('--output',required=True);a=p.parse_args()
root=pathlib.Path(__file__).resolve().parent;bt=pathlib.Path(a.sdk).resolve()/'build-tools/36.0.0';jar=pathlib.Path(a.sdk).resolve()/'platforms/android-36/android.jar';stage=root/'launcher-build';shutil.rmtree(stage,ignore_errors=True);stage.mkdir();classes=stage/'classes';classes.mkdir();out=pathlib.Path(a.output).resolve();out.parent.mkdir(parents=True,exist_ok=True)
def run(args):subprocess.run(list(map(str,args)),check=True)
sources=sorted((root/'java').rglob('*.java'))+sorted((root/'deps/SDL/android-project/app/src/main/java/org/libsdl/app').glob('*.java'))
run(['java','-m','jdk.compiler/com.sun.tools.javac.Main','-source','8','-target','8','-Xlint:-options','-cp',jar,'-d',classes,*sources]);arg=stage/'classes.list';arg.write_text('\n'.join(str(x) for x in sorted(classes.rglob('*.class'))));run([bt/'d8','--lib',jar,'--min-api','29','--output',stage,'@'+str(arg)])
run([bt/'aapt2','compile','--dir',root/'res','-o',stage/'res.zip']);run([bt/'aapt2','link','-o',stage/'base.apk','--manifest',root/'AndroidManifest.xml','-I',jar,'-R',stage/'res.zip','--min-sdk-version','29','--target-sdk-version','35','--version-code','5','--version-name','0.3.0-tq'])
unsigned=stage/'unsigned.apk';shutil.copyfile(stage/'base.apk',unsigned);native={}
with zipfile.ZipFile(a.runtime_apk) as old,zipfile.ZipFile(unsigned,'a',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for name in old.namelist():
  if name.startswith(('lib/','assets/')) and not name.endswith('/') and not name.startswith(('assets/gamedata/','assets/hp2data/','assets/texpacks/')) and name!='assets/build-info.txt':
   data=old.read(name);z.writestr(name,data)
   if name.startswith('lib/'):native[name]=hashlib.sha256(data).hexdigest()
 z.write(stage/'classes.dex','classes.dex');z.writestr('assets/build-info.txt','Harry Potter TQ 0.3.0: HP1/HP2 launcher, automatic configuration repair, profile-isolated optional HD modules. POA disabled. Native runtime unchanged from 0.1.0.\n')
 for f in sorted((root/'launcher-assets').rglob('*')):
  if f.is_file():z.write(f,'assets/'+f.relative_to(root/'launcher-assets').as_posix())
 game=pathlib.Path(a.hp2_data);assert (game/'System/Game.exe').is_file() and (game/'System/HGame.u').is_file();stamp=hashlib.sha256();count=0
 for f in sorted(game.rglob('*')):
  if f.is_file() and not any(c.startswith('.') for c in f.relative_to(game).parts):
   rel=f.relative_to(game).as_posix();data=f.read_bytes();stamp.update(rel.encode());stamp.update(hashlib.sha256(data).digest());z.writestr('assets/hp2data/'+rel,data);count+=1
 z.writestr('assets/hp2data/.stamp',stamp.hexdigest())
with zipfile.ZipFile(unsigned) as check:
 assert check.testzip() is None
run([bt/'zipalign','-f','-P','16','4',unsigned,stage/'aligned.apk']);run([bt/'apksigner','sign','--ks',pathlib.Path(a.keystore).resolve(),'--ks-key-alias','hp-native','--ks-pass','file:'+str(pathlib.Path(a.password_file).resolve()),'--out',out,stage/'aligned.apk']);run([bt/'apksigner','verify','--verbose','--print-certs',out]);run([bt/'zipalign','-c','-P','16','4',out])
with zipfile.ZipFile(out) as z:
 assert z.testzip() is None
 for name,h in native.items():assert hashlib.sha256(z.read(name)).hexdigest()==h
 for game in ('hp1','hp2'):
  for config in ('Default.ini','DefUser.ini'):assert len(z.read('assets/defaults/'+game+'/'+config))>0
report={'version':'0.3.0-tq','bytes':out.stat().st_size,'sha256':hashlib.file_digest(out.open('rb'),'sha256').hexdigest(),'bundledHP2Files':count,'nativeLibrariesUnchanged':native,'deviceExecutionTested':False};out.with_suffix('.verification.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
