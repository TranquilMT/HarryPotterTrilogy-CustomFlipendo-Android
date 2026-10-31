"""Generate an evidence-based, explicitly non-playable HP3 integration profile."""
import pathlib,json,collections,hashlib,struct,argparse
def main():
 p=argparse.ArgumentParser();p.add_argument('game');p.add_argument('audit');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.game);audit=pathlib.Path(a.audit);out=pathlib.Path(a.output);out.mkdir(parents=True,exist_ok=True)
 reports=json.loads((audit/'summary.json').read_text());assert not any('error' in r for r in reports)
 paths={p.stem.lower():p for p in root.rglob('*') if p.suffix.lower() in ('.u','.unr','.utx','.usx','.ukx','.uax') and not any(x.startswith('.') for x in p.relative_to(root).parts)}
 counts=collections.Counter();deps=[]
 for row in reports:
  counts.update(row['classCounts']);m=json.loads((audit/(row['path']+'.json')).read_text());names=sorted({e['name'] for e in m['imports'] if e['class']=='Package' and e['outer']==0})
  deps.append({'package':row['path'],'rootDependencies':names,'missingRootPackages':[n for n in names if n.lower() not in paths]})
 bindings={};aliases=[];section=''
 for line in (root/'system/defuser.ini').read_text(encoding='latin1').splitlines():
  line=line.strip()
  if line.startswith('['):section=line
  elif section=='[Engine.Input]' and '=' in line and not line.startswith(';'):
   key,value=line.split('=',1)
   if key.startswith('Aliases['):aliases.append({'key':key,'value':value})
   elif value.strip():bindings[key]=value.strip()
 profile={'id':'hp3-pc-na','title':'Harry Potter and the Prisoner of Azkaban (PC)','status':'inspection-only','runtimeSupported':False,'originalExecutable':'system/hppoa.exe','exeSHA256':hashlib.sha256((root/'system/hppoa.exe').read_bytes()).hexdigest(),'packageCount':len(reports),'packageVersions':dict(collections.Counter(str(r['version'])+':'+str(r['licenseeVersion']) for r in reports)),'exportClassCounts':dict(counts),'startupMap':'HP_preamble.unr','gameClass':'HGame.HGame','consoleClass':'KWGame.KWConsole','guiController':'HGame.HPGUIController','requiredSubsystems':['UE2 serialization','UE2 UnrealScript VM and native bindings','static meshes and material graph','skeletal animation','collision and Karma-dependent behavior','emitters','GUI','audio and movie playback','save serialization','Android input adapter']}
 (out/'hp3-pc-na.json').write_text(json.dumps(profile,indent=2));(out/'package-dependencies.json').write_text(json.dumps(deps,indent=2))
 controls={'status':'mapping-only; no Android runtime integration yet','source':'system/defuser.ini','originalBindings':bindings,'originalAliases':aliases,'touchProposal':{'leftStick':['MoveForward','MoveBackward','StrafeLeft','StrafeRight'],'look':'relative camera input; engine adapter pending','cast':bindings['LeftMouse'],'jump':bindings['Space'],'interact':bindings['Enter'],'map':bindings['Tab'],'menu':bindings['Escape'],'zoom':bindings['MiddleMouse'],'walk':bindings['Shift']}}
 (out/'hp3-controls.json').write_text(json.dumps(controls,indent=2));print('Profile generated; missing dependencies:',sum(len(d['missingRootPackages']) for d in deps),flush=True)
if __name__=='__main__':main()
