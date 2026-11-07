import io,tempfile,unittest,zipfile
from pathlib import Path
from release_checks import hp3_inventory,verify_embedded_game
class PoARelease(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
  names=['System/'+n for n in ['hppoa.exe','Core.u','Engine.u','HGame.u','KWGame.u','GUI.u','UWindow.u','GamePlay.u','Default.ini','DefUser.ini']]+['Maps/Start.unr','Textures/Start.utx','Animations/Start.ukx','StaticMeshes/Start.usx']
  for n in names:
   p=self.root/n;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(bytes.fromhex('c1832a9e81000000'))
 def tearDown(self):self.tmp.cleanup()
 def test_complete_script_set(self):self.assertEqual(len(hp3_inventory(self.root)),14)
 def test_kwgame_dll_is_not_script_replacement(self):
  (self.root/'System/KWGame.u').unlink()
  (self.root/'System/KWGame.dll').write_bytes(b'MZ Windows DLL')
  with self.assertRaisesRegex(ValueError,'kwgame.u'):hp3_inventory(self.root)
 def test_missing_animation_rejected(self):
  (self.root/'Animations/Start.ukx').unlink()
  with self.assertRaises(ValueError):hp3_inventory(self.root)
 def test_wrong_script_version_rejected(self):
  (self.root/'System/HGame.u').write_bytes(bytes.fromhex('c1832a9e4c000000'))
  with self.assertRaises(ValueError):hp3_inventory(self.root)
 def test_case_collision_rejected(self):
  (self.root/'System/core.u').write_bytes(b'x')
  with self.assertRaises(ValueError):hp3_inventory(self.root)
 def test_symlink_rejected(self):
  (self.root/'System/link.u').symlink_to('Core.u')
  with self.assertRaises(ValueError):hp3_inventory(self.root)
if __name__=='__main__':unittest.main()
