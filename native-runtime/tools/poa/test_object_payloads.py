import unittest,struct
from inspect_packages import Reader
from object_payloads import tags,function_header,native_stub
class ObjectTests(unittest.TestCase):
 def test_boolean_value_is_in_header(self):
  r=Reader(bytes([1,0x83,0]));t=tags(r,['None','Enabled']);self.assertTrue(t[0]['boolValue']);self.assertEqual(r.p,3);self.assertEqual(t[0]['size'],0)
 def test_four_byte_array_index(self):
  r=Reader(bytes([1,0x81,0xc0,0,0x40,0,7,0]));t=tags(r,['None','Entry']);self.assertEqual(t[0]['arrayIndex'],16384);self.assertEqual(t[0]['size'],1)
 def test_declared_size_does_not_authorize_overread(self):
  with self.assertRaises(ValueError):tags(Reader(bytes([1,0x72,255,255,255,127])),['None','Bad'])
 def test_native_parameter_uses_memory_index_width(self):
  header=bytes([0,0,0,0,0,1,0])+struct.pack('<III',1,2,6)
  raw=header+bytes([0x29,1,0x0b])+struct.pack('<HBI',191,0,0x400)
  r,h=function_header(raw,['None','Exp']);n=native_stub(r,h);self.assertEqual(n['nativeIndex'],191);self.assertEqual(n['parameters'],[1]);self.assertEqual(r.p,len(raw))
 def test_unknown_extension_is_rejected(self):
  raw=bytes([0,0,0,0,0,1,1])+bytes(12)
  with self.assertRaises(ValueError):function_header(raw,['None','Function'])
if __name__=='__main__':unittest.main()
