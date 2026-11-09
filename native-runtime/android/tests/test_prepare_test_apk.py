import pathlib,tempfile,unittest,zipfile
from prepare_test_apk import prepare,LIBRARIES
class TestAPKPreparation(unittest.TestCase):
 def test_retains_three_payloads_and_replaces_code_without_old_signature(self):
  with tempfile.TemporaryDirectory()as tmp:
   p=pathlib.Path(tmp);base=p/'base.apk';resources=p/'resources.apk';out=p/'unsigned.apk'
   with zipfile.ZipFile(base,'w')as z:
    for game in ('hp1data','hp2data','hp3data'):z.writestr('assets/'+game+'/System/Game.u',game.encode())
    for n in ('classes.dex','META-INF/OLD.RSA','AndroidManifest.xml','resources.arsc','res/old.png'):z.writestr(n,b'old')
   with zipfile.ZipFile(resources,'w')as z:
    for n in ('AndroidManifest.xml','resources.arsc','res/new.png'):z.writestr(n,b'new')
   elf=b'\x7fELF\x02\x01'+b'\0'*12+b'\xb7\0'
   for name in LIBRARIES:(p/name).write_bytes(elf)
   prepare(base,p,resources,out,'a'*40)
   with zipfile.ZipFile(out)as z:
    self.assertEqual(z.read('classes.dex'),b'old');self.assertEqual(z.read('AndroidManifest.xml'),b'new');self.assertNotIn('META-INF/OLD.RSA',z.namelist());self.assertNotIn('res/old.png',z.namelist())
    for game in ('hp1data','hp2data','hp3data'):self.assertEqual(z.read('assets/'+game+'/System/Game.u'),game.encode())
    for name in LIBRARIES:self.assertEqual(z.read('lib/arm64-v8a/'+name),elf)
   (p/LIBRARIES[0]).write_bytes(b'bad')
   with self.assertRaises(ValueError):prepare(base,p,resources,out,'a'*40)
if __name__=='__main__':unittest.main()
