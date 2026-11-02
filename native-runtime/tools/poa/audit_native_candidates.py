"""Find registration-name candidates only. This does not verify native ABIs."""
import pathlib,json,re,collections,argparse
def main():
 p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('bindings');p.add_argument('output');a=p.parse_args();root=pathlib.Path(a.source);candidates={}
 for path in list((root/'src/engine/SurrealEngine/Native').glob('*.cpp'))+list((root/'src/surreal-patches').glob('*.patch')):
  text=path.read_text(errors='replace')
  if path.suffix=='.patch':text='\n'.join(l[1:] for l in text.splitlines() if l.startswith('+') and not l.startswith('+++'))
  for cls,func,index in re.findall(r'RegisterVMNativeFunc_\d+\("([^"]+)",\s*"([^"]+)",[^\n]*,\s*(\d+)\)',text):candidates.setdefault((cls,func),set()).add(int(index))
 rows=[]
 for n in json.loads(pathlib.Path(a.bindings).read_text()):
  indices=candidates.get((n['owner'],n['object']),set());status='no-name-candidate' if not indices else ('name-and-index-candidate' if n['nativeIndex'] in indices else 'name-candidate-index-differs')
  rows.append({'package':n['package'],'owner':n['owner'],'function':n['object'],'hp3NativeIndex':n['nativeIndex'],'candidateIndices':sorted(indices),'status':status,'abiVerified':False})
 pathlib.Path(a.output).write_text(json.dumps(rows,indent=2));print(dict(collections.Counter(r['status'] for r in rows)))
if __name__=='__main__':main()

