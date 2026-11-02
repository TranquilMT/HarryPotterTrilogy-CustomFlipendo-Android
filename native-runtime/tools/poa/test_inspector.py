import unittest,tempfile,pathlib,struct
from inspect_packages import Reader,inspect
class BoundsTests(unittest.TestCase):
 def test_truncated_compact(self):
  with self.assertRaises(ValueError):Reader(b'\x40').compact()
 def test_outside_read(self):
  with self.assertRaises(ValueError):Reader(b'123').read(4)
 def test_bad_signature(self):
  with tempfile.TemporaryDirectory() as d:
   p=pathlib.Path(d)/'bad.u';p.write_bytes(b'X'*64)
   with self.assertRaises(ValueError):inspect(p)
 def test_table_count_limit(self):
  b=bytearray(64);struct.pack_into('<IHH7I',b,0,0x9e2a83c1,129,0,0,1000001,36,0,36,0,36)
  with tempfile.TemporaryDirectory() as d:
   p=pathlib.Path(d)/'bad.u';p.write_bytes(b)
   with self.assertRaises(ValueError):inspect(p)
if __name__=='__main__':unittest.main()

