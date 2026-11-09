"""Inventory original EU menu native declarations against source registrations.
A matching registration is evidence of a binding candidate, not runtime acceptance.
"""
import argparse,json,pathlib,re
from inspect_packages import inspect
from object_payloads import function_header,native_stub

def audit(system,source):
 pairs=set()
 for p in source.rglob('*.cpp'):
  text=p.read_text(errors='replace')
  for m in re.finditer(r'RegisterVMNativeFunc_\w+\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"',text):pairs.add(tuple(x.casefold() for x in m.groups()))
 rows=[];identities={}
 for filename in ('GUI.u','UDebugMenu.u','kwGame.u','UWindow.u','HGame.u','Engine.u'):
  p=system/filename;m=inspect(p);data=p.read_bytes();identities[filename]=m['sha256']
  for e in m['exports']:
   if e['class']!='Function' or not e['size']:continue
   parent=m['exports'][e['outer']-1]['name'] if e['outer']>0 else ''
   if filename=='Engine.u' and parent not in ('Canvas','Console','GUIController'):continue
   r,h=function_header(data[e['offset']:e['offset']+e['size']],m['names']);stub=native_stub(r,h)
   if not stub or not stub['native']:continue
   rows.append(dict(package=filename,owner=parent,name=e['name'],nativeIndex=stub['nativeIndex'],directRegistrationCandidate=(parent.casefold(),e['name'].casefold())in pairs))
 return dict(scope='Static direct-binding audit; inheritance, numeric dispatch, registration guards and handler semantics still need review. Unmatched declarations are candidates, not proven startup blockers.',packages=identities,declarations=rows,total=len(rows),unmatched=sum(not r['directRegistrationCandidate']for r in rows))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('system',type=pathlib.Path);p.add_argument('source',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args();r=audit(a.system,a.source);a.output.write_text(json.dumps(r,indent=2)+'\n');print('Menu native declarations:',r['total'],'unmatched direct bindings:',r['unmatched'])
