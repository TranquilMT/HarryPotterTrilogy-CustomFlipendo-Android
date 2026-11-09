"""Prepare an unsigned three-game APK with current native libraries.
Signing is a separate step requiring the original private key. Never copies signatures.
"""
import argparse,pathlib,shutil,zipfile
LIBRARIES=('libmain.so','libSDL3.so','libopenal.so','libSurrealVideo.so')
def prepare(base,libraries,manifest,output,commit):
 libraries=pathlib.Path(libraries)
 if len(commit)!=40 or any(c not in '0123456789abcdef'for c in commit):raise ValueError('Expected source commit SHA')
 replacements={}
 for name in LIBRARIES:
  p=libraries/name
  with p.open('rb')as f:header=f.read(20)
  if len(header)<20 or header[:6]!=b'\x7fELF\x02\x01' or int.from_bytes(header[18:20],'little')!=183:raise ValueError('Expected ARM64 ELF: '+name)
  replacements['lib/arm64-v8a/'+name]=p
 with zipfile.ZipFile(base)as src,zipfile.ZipFile(manifest)as resources:
  names=src.namelist()
  if len(names)!=len(set(names)):raise ValueError('Duplicate base APK entries')
  for game in ('hp1data','hp2data','hp3data'):
   if not any(n.startswith('assets/'+game+'/')for n in names):raise ValueError('Missing bundled '+game)
  if 'classes.dex'not in names:raise ValueError('Missing launcher dex')
  resource_names=[n for n in resources.namelist()if n in ('AndroidManifest.xml','resources.arsc')or n.startswith('res/')]
  if not {'AndroidManifest.xml','resources.arsc'}.issubset(resource_names):raise ValueError('Incomplete compiled Android resources')
  with zipfile.ZipFile(output,'w',allowZip64=True)as dst:
   for info in src.infolist():
    n=info.filename
    if n.startswith('META-INF/')or n.startswith('res/')or n in ('AndroidManifest.xml','resources.arsc','assets/build-info.txt')or n in replacements:continue
    with src.open(info)as r,dst.open(info,'w',force_zip64=True)as w:shutil.copyfileobj(r,w,1024*1024)
   for n in resource_names:dst.writestr(resources.getinfo(n),resources.read(n))
   for n,p in replacements.items():dst.write(p,n,compress_type=zipfile.ZIP_STORED)
   dst.writestr('assets/build-info.txt','HP:Trilogy Mobile 0.4.15-test; source '+commit+'; unsigned preparation; device acceptance pending\n')
 if pathlib.Path(output).stat().st_size==0:raise ValueError('Empty output')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('base',type=pathlib.Path);p.add_argument('libraries',type=pathlib.Path);p.add_argument('manifest',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);p.add_argument('commit');a=p.parse_args();prepare(a.base,a.libraries,a.manifest,a.output,a.commit)
