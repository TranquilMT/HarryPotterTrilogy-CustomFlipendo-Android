#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Verify actual bundled payload against the original-disc extraction manifest."""
import pathlib,json,zipfile,hashlib,subprocess,re,os,argparse
p=argparse.ArgumentParser();p.add_argument('apk');p.add_argument('--prior',required=True);p.add_argument('--sdk',required=True);p.add_argument('--jdk',required=True);a=p.parse_args();root=pathlib.Path(__file__).resolve().parent.parent;apk=pathlib.Path(a.apk).resolve();report=json.load(open(root/'android/bundled-game-verification.json'));prefix='assets/gamedata/'
with zipfile.ZipFile(apk) as z:
 names=[n for n in z.namelist() if n.startswith(prefix) and not n.endswith('/') and n!=prefix+'.stamp'];assert len(names)==len(set(names))==report['fileCount']==330
 assert len(z.read(prefix+'.stamp'))==64
 for f in report['files']:assert hashlib.sha256(z.read(prefix+f['path'])).hexdigest()==f['sha256']
 assert z.testzip() is None
env=dict(os.environ,JAVA_HOME=str(pathlib.Path(a.jdk).resolve()));bt=pathlib.Path(a.sdk).resolve()/'build-tools/36.0.0';certs=[]
for f in (pathlib.Path(a.prior).resolve(),apk):
 t=subprocess.check_output([str(bt/'apksigner'),'verify','--print-certs',str(f)],env=env,text=True);certs.append(re.search(r'Signer #1 certificate SHA-256 digest: (\w+)',t).group(1))
assert certs[0]==certs[1]
badging=subprocess.check_output([str(bt/'aapt2'),'dump','badging',str(apk)],text=True);assert "name='com.tranquil.hp1native'" in badging and "versionCode='2'" in badging and 'application-debuggable' not in badging
verification={'bundledFiles':330,'bundledBytes':report['totalBytes'],'allBundledFileHashesMatch':True,'firstLaunchStampPresent':True,'sameSigningCertificateAs010':True,'certificateSHA256':certs[0],'package':'com.tranquil.hp1native','versionCode':2,'deviceExecutionTested':False,'apkBytes':apk.stat().st_size,'apkSHA256':hashlib.sha256(apk.read_bytes()).hexdigest()};(root/'android/bundled-apk-verification.json').write_text(json.dumps(verification,indent=2));print(json.dumps(verification,indent=2))
