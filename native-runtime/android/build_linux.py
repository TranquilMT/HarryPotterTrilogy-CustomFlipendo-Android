#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Linux ARM64 native build/packager. Requires prepared, patched source/dependencies.
Optional private game-data bundle. Reuse the same keystore and password file for every release.
"""
import argparse,pathlib,subprocess,os,shutil,zipfile,json,hashlib,struct
p=argparse.ArgumentParser();p.add_argument('--sdk',required=True);p.add_argument('--ndk',required=True);p.add_argument('--jdk',required=True);p.add_argument('--cmake',default='cmake');p.add_argument('--ninja',default='ninja');p.add_argument('--keystore',required=True);p.add_argument('--password-file',required=True);p.add_argument('--jobs',default='4');p.add_argument('--skip-native',action='store_true');p.add_argument('--game-data');a=p.parse_args();root=pathlib.Path(__file__).resolve().parent.parent;sdk=pathlib.Path(a.sdk).resolve();ndk=pathlib.Path(a.ndk).resolve();jdk=pathlib.Path(a.jdk).resolve();key=pathlib.Path(a.keystore).resolve();password=pathlib.Path(a.password_file).resolve();game=pathlib.Path(a.game_data).resolve() if a.game_data else None;os.chdir(root);env=os.environ.copy();env['JAVA_HOME']=str(jdk);env['PATH']=str(jdk/'bin')+os.pathsep+env['PATH'];bt=sdk/'build-tools/36.0.0';aj=sdk/'platforms/android-36/android.jar'
def run(args):subprocess.run(list(map(str,args)),env=env,check=True)
build=root/'build-android'
if not a.skip_native:
 run([a.cmake,'-S','android','-B',build,'-G','Ninja','-DCMAKE_MAKE_PROGRAM='+str(pathlib.Path(a.ninja).resolve()),'-DCMAKE_TOOLCHAIN_FILE='+str(ndk/'build/cmake/android.toolchain.cmake'),'-DANDROID_ABI=arm64-v8a','-DANDROID_PLATFORM=android-29','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384','-DCMAKE_POLICY_VERSION_MINIMUM=3.5'])
 run([a.cmake,'--build',build,'--target','main','-j',a.jobs])
stage=root/'android/linux-apk';stage.mkdir(parents=True,exist_ok=True);classes=stage/'classes';classes.mkdir(exist_ok=True);assets=stage/'assets';assets.mkdir(exist_ok=True);lib=stage/'lib/arm64-v8a';lib.mkdir(parents=True,exist_ok=True);dist=root/'android/dist';dist.mkdir(exist_ok=True)
strip=ndk/'toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip'
libs=[build/'libmain.so',build/'SDL3/libSDL3.so',build/'engine/Thirdparty/openal-soft/libopenal.so',build/'engine/libSurrealVideo.so']
for src in libs:
 dst=lib/src.name;shutil.copyfile(src,dst);run([strip,'--strip-unneeded',dst])
with zipfile.ZipFile(assets/'SurrealEngine.pk3','w',zipfile.ZIP_DEFLATED) as z:
 for f in sorted((root/'src/engine/Resources').rglob('*')):
  if f.is_file():z.write(f,f.relative_to(root/'src/engine/Resources'))
shutil.copytree(root/'images/android',assets/'icons',dirs_exist_ok=True)
ru=assets/'rutext';ru.mkdir(exist_ok=True)
for f in (root/'android/lang/ru-text').glob('*.rus'):(ru/(f.stem+'.int')).write_bytes(f.read_text(encoding='utf-16').encode('cp1251'))
(assets/'build-info.txt').write_text('Harry Potter TQ native test 0.1.1; based on Flipendo Droid 7508365a02fc0f56d1f9303e37de446891534cc7\n')
sources=list((root/'android/deps/SDL/android-project/app/src/main/java/org/libsdl/app').glob('*.java'))+list((root/'android/java/io/github/flipendo/spike').glob('*.java'))
run([jdk/'bin/javac','-source','8','-target','8','-Xlint:-options','-cp',aj,'-d',classes,*sources]);classfiles=sorted(classes.rglob('*.class'));argfile=stage/'classes.list';argfile.write_text('\n'.join(str(f) for f in classfiles));run([bt/'d8','--lib',aj,'--output',stage,'@'+str(argfile)])
run([bt/'aapt2','compile','--dir',root/'android/res','-o',stage/'res.zip']);run([bt/'aapt2','link','-o',stage/'base.apk','--manifest',root/'android/AndroidManifest.xml','-I',aj,'-R',stage/'res.zip','--min-sdk-version','29','--target-sdk-version','35','--version-code','2','--version-name','0.1.1-tq-native'])
unsigned=stage/'unsigned.apk';shutil.copyfile(stage/'base.apk',unsigned)
with zipfile.ZipFile(unsigned,'a',zipfile.ZIP_DEFLATED) as z:
 z.write(stage/'classes.dex','classes.dex')
 for parent in (lib.parent,assets):
  for f in sorted(parent.rglob('*')):
   if f.is_file():z.write(f,f.relative_to(stage))
if game:
 if not (game/'System/HP.exe').is_file() or not (game/'System/HarryPotter.u').is_file():raise ValueError('Not a recovered HP1 game folder')
 with zipfile.ZipFile(unsigned,'a',zipfile.ZIP_DEFLATED) as z:
  stamp=hashlib.sha256()
  for f in sorted(game.rglob('*')):
   if f.is_file() and not any(part.startswith('.') for part in f.relative_to(game).parts):
    stamp.update(str(f.relative_to(game)).encode());stamp.update(hashlib.sha256(f.read_bytes()).digest());z.write(f,pathlib.PurePosixPath('assets/gamedata')/f.relative_to(game))
  z.writestr('assets/gamedata/.stamp',stamp.hexdigest())
run([bt/'zipalign','-f','-P','16','4',unsigned,stage/'aligned.apk'])
if not key.exists():
 key.parent.mkdir(parents=True,exist_ok=True)
 run([jdk/'bin/keytool','-genkeypair','-keystore',key,'-storepass:file',password,'-keypass:file',password,'-alias','hp-native','-keyalg','RSA','-keysize','3072','-validity','10000','-dname','CN=William Utting Native HP Test'])
out=dist/('Harry-Potter-TQ-Native-0.1.1-Bundled.apk' if game else 'Harry-Potter-TQ-Native-0.1.1.apk');run([bt/'apksigner','sign','--ks',key,'--ks-key-alias','hp-native','--ks-pass','file:'+str(password),'--out',out,stage/'aligned.apk']);run([bt/'apksigner','verify','--verbose',out]);run([bt/'zipalign','-c','-P','16','4',out])
elfReports=[]
with zipfile.ZipFile(out) as z:
 assert z.testzip() is None
 for name in sorted(n for n in z.namelist() if n.endswith('.so')):
  data=z.read(name);assert data[:4]==b'\x7fELF' and data[4]==2 and struct.unpack_from('<H',data,18)[0]==183
  phoff=struct.unpack_from('<Q',data,32)[0];phsize,phcount=struct.unpack_from('<HH',data,54);alignments=[]
  for i in range(phcount):
   typ,fl,off,va,pa,fs,ms,align=struct.unpack_from('<II6Q',data,phoff+i*phsize)
   if typ==1:
    assert align>=16384 and off%align==va%align
    alignments.append(align)
  assert alignments;elfReports.append(dict(path=name,architecture='AArch64',loadAlignments=alignments))
report=dict(apk=str(out),bytes=out.stat().st_size,sha256=hashlib.sha256(out.read_bytes()).hexdigest(),nativeLibraries=[x.name for x in libs],elfVerification=elfReports,deviceExecutionTested=False,isoImporterHostTested=True,bundledGameData=bool(game));(dist/'build-verification.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
