"""Lock the original UE2 startup contract used to replace the UE1 login path."""
import os,pathlib,unittest
from audit_player_startup import audit
class PlayerStartupContract(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('POA_EU_GAME'),'Set POA_EU_GAME for original startup metadata')
 def test_original_login_return_and_possession_contract(self):
  result=audit(pathlib.Path(os.environ['POA_EU_GAME'])/'System')
  self.assertEqual(len(result['functions']),8)
  for owner in ('GameInfo','KWGame'):
   login=next(f for f in result['functions']if f['owner']==owner and f['name']=='Login')
   self.assertEqual([p['name']for p in login['parameters']],['Portal','Options','Error','ReturnValue'])
   self.assertTrue(login['parameters'][2]['out']);self.assertTrue(login['parameters'][3]['return']);self.assertEqual(login['parameters'][3]['objectClass'],'PlayerController')
  pre=next(f for f in result['functions']if f['owner']=='GameInfo'and f['name']=='PreLogin')
  self.assertEqual([p['name']for p in pre['parameters']],['Options','Address','Error','FailCode'])
  for owner in ('Controller','PlayerController','KWHeroController'):
   f=next(f for f in result['functions']if f['owner']==owner and f['name']=='Possess');self.assertEqual(len(f['parameters']),1);self.assertEqual(f['parameters'][0]['objectClass'],'Pawn')
  bases={c['name']:c['base']for c in result['classes']}
  self.assertEqual(bases,dict(Controller='Actor',PlayerController='Controller',KWHeroController='PlayerController',HPHeroController='KWHeroController',HarryController='HPHeroController'))
if __name__=='__main__':unittest.main()
