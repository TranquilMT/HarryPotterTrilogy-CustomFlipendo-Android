"""Generate an evidence-based, explicitly non-playable HP3 integration profile."""
import pathlib,json,collections,hashlib,struct,argparse
from startup_profile import audit_startup, file_index, profile_identity
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('audit');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);audit=pathlib.Path(a.audit);out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True)
 reports=json.loads((audit/'summary.json').read_text())
 if any('error' in r for r in reports):raise ValueError('Package inspection errors must be resolved before profile generation')
 startup=audit_startup(root);files=file_index(root)
 paths={p.stem.lower():p for p in root.rglob('*') if p.suffix.lower() in ('.u','.unr','.utx','.usx','.ukx','.uax') and not any(x.startswith('.') for x in p.relative_to(root).parts)}
 counts=collections.Counter();deps=[]
 for row in reports:
  m=json.loads((audit/(row['path']+'.json')).read_text())
  key=row['path'].replace('\\','/').casefold()
  if key not in files or hashlib.sha256(files[key].read_bytes()).hexdigest()!=m['sha256']:raise ValueError('Stale package audit: '+row['path'])
  counts.update(m['classCounts']);names=sorted({e['name'] for e in m['imports'] if e['class']=='Package' and e['outer']==0})
  native_modules = [name for name in names if name.lower() not in paths
                    and 'system/' + name.lower() + '.dll' in files]
  deps.append({'package': row['path'], 'rootDependencies': names,
               'externalNativeModules': native_modules,
               'missingRootPackages': [name for name in names
                   if name.lower() not in paths and name not in native_modules]})
 bindings={};aliases=[];section=''
 for line in files['system/defuser.ini'].read_text(encoding='latin1').splitlines():
  line=line.strip()
  if line.startswith('['):section=line
  elif section=='[Engine.Input]' and '=' in line and not line.startswith(';'):
   key,value=line.split('=',1)
   if key.startswith('Aliases['):aliases.append({'key':key,'value':value})
   elif value.strip():bindings[key]=value.strip()
 profile={'id':'hp3-pc-unrecognized','title':'Harry Potter and the Prisoner of Azkaban (PC)','status':'inspection-only','runtimeSupported':False,'originalExecutable':startup['executable']['path'],'exeSHA256':startup['executable']['sha256'],'packageCount':len(reports),'packageVersions':dict(collections.Counter(str(r['version'])+':'+str(r['licenseeVersion']) for r in reports)),'exportClassCounts':dict(counts),'requiredSubsystems':['UE2 serialization','UE2 UnrealScript VM and native bindings','static meshes and material graph','skeletal animation','collision and Karma-dependent behavior','emitters','GUI','audio and movie playback','save serialization','Android input adapter']}
 profile.update(startup['route']);profile['startupEvidence']=startup
 reference=json.loads((pathlib.Path(__file__).parent/'reference-profile/hp3-pc-eu.json').read_text())
 profile['id']=profile_identity(profile['exeSHA256'],startup['scriptPackages'],reference)
 (out/(profile['id']+'.json')).write_text(json.dumps(profile,indent=2));(out/'package-dependencies.json').write_text(json.dumps(deps,indent=2))
 controls={'status':'mapping-only; no Android runtime integration yet','source':startup['configuration'][1]['path'],'originalBindings':bindings,'originalAliases':aliases,'touchProposal':{'leftStick':['MoveForward','MoveBackward','StrafeLeft','StrafeRight'],'look':'relative camera input; engine adapter pending','cast':bindings['LeftMouse'],'jump':bindings['Space'],'interact':bindings['Enter'],'map':bindings['Tab'],'menu':bindings['Escape'],'zoom':bindings['MiddleMouse'],'walk':bindings['Shift']}}
 (out/'hp3-controls.json').write_text(json.dumps(controls,indent=2));print('Profile generated; missing dependencies:',sum(len(d['missingRootPackages']) for d in deps),flush=True)
if __name__=='__main__':main()

