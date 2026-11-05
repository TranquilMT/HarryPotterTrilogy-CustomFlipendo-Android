import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[2]
class IniLayout(unittest.TestCase):
 def test_production_policy_and_call_sites(self):
  with tempfile.TemporaryDirectory() as temp:
   exe=pathlib.Path(temp)/'ini-layout'
   subprocess.run(['g++','-std=c++17','-I'+str(ROOT/'src/engine/SurrealEngine'),str(pathlib.Path(__file__).with_suffix('.cpp')),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
  source=(ROOT/'src/engine/SurrealEngine/Package/PackageManager.cpp').read_text()
  self.assertIn('if (UsesIndexedPackagePaths(launchInfo))',source)
  self.assertEqual(source.count('if (UsesCombinedUserIni(launchInfo))'),3)
  self.assertIn('if (LoadsSeparateUserIni(launchInfo))',source)
if __name__=='__main__':unittest.main()
