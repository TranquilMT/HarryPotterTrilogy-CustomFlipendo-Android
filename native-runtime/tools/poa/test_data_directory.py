import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class DataDirectory(unittest.TestCase):
 def test_actual_directory_visitor(self):
  with tempfile.TemporaryDirectory() as temp:
   root=pathlib.Path(temp);exe=root/'visitor'
   subprocess.run(['g++','-std=c++17','-I'+str(ROOT/'src/engine/SurrealEngine'),str(pathlib.Path(__file__).with_suffix('.cpp')),'-o',str(exe)],check=True)
   subprocess.run([str(exe),str(root/'data')],check=True)
if __name__=='__main__':unittest.main()
