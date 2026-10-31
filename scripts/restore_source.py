#!/usr/bin/env python3
"""Reassemble and verify the complete source checkpoint, then extract it safely."""
import hashlib,pathlib,zipfile,shutil,tempfile
root=pathlib.Path(__file__).resolve().parents[1]
parts=sorted((root/'source-packages/0.3.1').glob('source.zip.part-*'))
if len(parts)!=21:raise SystemExit('Expected 21 source archive parts')
out=root/'release-assets/Harry-Potter-TQ-0.3.1-HD-Bundled-Source.zip';out.parent.mkdir(exist_ok=True)
with out.open('wb') as f:
 for part in parts:
  with part.open('rb') as src:shutil.copyfileobj(src,f)
with out.open('rb') as f:actual=hashlib.file_digest(f,'sha256').hexdigest()
if actual!='13a0d31f66e9154886c0a230c6109f5f070b9669618a6378b0d1906fcc9c3a55':raise SystemExit('Source checkpoint checksum mismatch')
with zipfile.ZipFile(out) as z:
 if z.testzip() is not None:raise SystemExit('Source checkpoint CRC failure')
 for info in z.infolist():
  p=pathlib.PurePosixPath(info.filename)
  if p.is_absolute() or '..' in p.parts or not p.parts or p.parts[0] not in ('native-runtime','hd-pipeline'):raise SystemExit('Unsafe source archive member')
 z.extractall(root)
print('Verified and extracted complete Android/native/HD source:',len(z.infolist()),'entries')
