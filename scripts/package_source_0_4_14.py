import pathlib,json,hashlib,zipfile,sys
root,repo,out=map(pathlib.Path,sys.argv[1:])
for name in ('native-runtime/android/AndroidManifest.xml','native-runtime/android/VERSION','native-runtime/android/build_linux.py'):
    (root/name).write_bytes((repo/name).read_bytes())
p=root/'docs/POA-SOURCE-LOCK.json';m=json.loads(p.read_text());m['checkpoint']='local-0.4.14-eu-range-defaults-version-corrected'
for n in m['files']:
    f=root/n
    with f.open('rb') as stream:h=hashlib.file_digest(stream,'sha256').hexdigest()
    m['files'][n]={'bytes':f.stat().st_size,'sha256':h}
p.write_text(json.dumps(m,indent=2)+'\n')
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for f in sorted(root.rglob('*')):
        if f.is_file() and '__pycache__' not in f.parts:
            info=zipfile.ZipInfo(f.relative_to(root).as_posix(),date_time=(2026,11,8,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o100644<<16
            z.writestr(info,f.read_bytes(),compresslevel=6)
with out.open('rb') as f:print(hashlib.file_digest(f,'sha256').hexdigest(),out)
