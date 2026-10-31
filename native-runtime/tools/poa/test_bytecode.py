import unittest
from inspect_packages import Reader
from bytecode import Decoder,DecodeError
class BytecodeTests(unittest.TestCase):
 def decode(self,raw,size,exports=0):return Decoder(Reader(bytes(raw)),['None'],0,exports,size).decode()
 def test_compact_reference_expands_to_memory_width(self):
  d=Decoder(Reader(bytes([4,0,0x40,1])),['None'],0,64,6);result=d.decode();self.assertEqual(result['serializedBytes'],4);self.assertEqual(d.expanded,bytes([4,0,64,0,0,0]));self.assertEqual(result['tokens'],2)
 def test_jump_to_operand_rejected(self):
  with self.assertRaises(DecodeError):self.decode([6,1,0,4,0x0b],5)
 def test_jump_into_nested_expression_rejected(self):
  with self.assertRaises(DecodeError):self.decode([6,4,0,4,0x0b],5)
 def test_unknown_opcode_rejected(self):
  with self.assertRaises(DecodeError):self.decode([3],1)
 def test_unterminated_native_arguments_rejected(self):
  with self.assertRaises(ValueError):self.decode([0x70,0x0b],2)
 def test_reference_outside_package_rejected(self):
  with self.assertRaises(DecodeError):self.decode([0,1],5)
 def test_depth_bound(self):
  with self.assertRaises(DecodeError):self.decode([4]*130+[0x0b],131)
 def test_ue2_primitive_cast_is_distinct_from_ue1_conversion(self):
  result=self.decode([0x39,0x3a,0x2c,7],4);self.assertEqual(result['tokens'],2)
if __name__=='__main__':unittest.main()
