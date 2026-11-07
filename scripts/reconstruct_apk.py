"""Reconstruct and hash-check the exact signed release APK. No signing key required."""
import pathlib,sys,json,hashlib,base64,zlib
base,folder,destination=map(pathlib.Path,sys.argv[1:])
m=json.loads((folder/'manifest.json').read_text())
def digest(path):
 with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
if m['format']!=1 or digest(base)!=m['baseSHA256']:raise ValueError('Wrong base APK')
packed=bytearray()
for name in m['parts']:
 if pathlib.PurePosixPath(name).name!=name:raise ValueError('Unsafe shard name')
 packed.extend(base64.b64decode((folder/name).read_bytes(),validate=False))
if hashlib.sha256(packed).hexdigest()!=m['packedSHA256']:raise ValueError('Recovery shard checksum mismatch')
data=zlib.decompress(packed)
if len(data)!=m['literalBytes']:raise ValueError('Wrong literal length')
destination.parent.mkdir(parents=True,exist_ok=True)
temporary=destination.with_suffix(destination.suffix+'.partial')
with base.open('rb') as source,temporary.open('wb') as target:
 for kind,start,length in m['operations']:
  if start<0 or length<0:raise ValueError('Invalid range')
  if kind=='L':
   block=data[start:start+length]
   if len(block)!=length:raise ValueError('Truncated literal')
   target.write(block)
  elif kind=='C':
   source.seek(start);remaining=length
   while remaining:
    block=source.read(min(remaining,4*1024*1024))
    if not block:raise ValueError('Truncated copy range')
    target.write(block);remaining-=len(block)
  else:raise ValueError('Invalid operation')
if temporary.stat().st_size!=m['outputBytes'] or digest(temporary)!=m['outputSHA256']:
 raise ValueError('Reconstructed APK checksum mismatch')
temporary.replace(destination)
print('Verified exact signed APK:',destination.name,m['outputSHA256'])
