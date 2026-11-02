import unittest
from audit_dispatch_calls import classify_call


class CallAuditTests(unittest.TestCase):
    def setUp(self):
        self.scopes = {
            'p.base': dict(kind='Class', parent=None, functions={'run': 'base.run'}),
            'p.child': dict(kind='Class', parent='p.base', functions={}),
            'p.child.idle': dict(kind='State', parent=None, owner='p.child', functions={'run': 'idle.run'})}

    def token(self, op=0x1b, name='Run'):
        return dict(opcode=op, offset=5, end=12, functionName=name)

    def test_virtual_state(self):
        t = self.token()
        self.assertEqual(classify_call(self.scopes, 'p.child.idle', t, [t]),
                         ('lexicalTargetFound', 'idle.run'))

    def test_global_bypass_state(self):
        t = self.token(0x38)
        self.assertEqual(classify_call(self.scopes, 'p.child.idle', t, [t]),
                         ('lexicalTargetFound', 'base.run'))

    def test_class_inheritance(self):
        t = self.token()
        self.assertEqual(classify_call(self.scopes, 'p.child', t, [t])[1], 'base.run')

    def test_context_is_deferred(self):
        for op in (0x12, 0x19, 0x43):
            t = self.token()
            context = dict(opcode=op, offset=0, end=13)
            self.assertEqual(classify_call(self.scopes, 'p.child', t, [context, t]),
                             ('deferredReceiverContext', None))

    def test_finished_context_does_not_affect_next_call(self):
        t = self.token()
        context = dict(opcode=0x19, offset=0, end=5)
        self.assertEqual(classify_call(self.scopes, 'p.child', t, [context, t])[1], 'base.run')

    def test_unknown_is_not_success(self):
        t = self.token(name='Unknown')
        self.assertEqual(classify_call(self.scopes, 'p.child', t, [t]),
                         ('requiresDynamicReceiverOrMissing', None))


if __name__ == '__main__':
    unittest.main()
