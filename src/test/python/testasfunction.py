import find_projection
import _pyrt
import unittest

class TestQueryInterface(unittest.TestCase):
    def test_as_function(self):
        propset = _pyrt.PropertySet()
        propset.Insert("strmap", _pyrt.StringMap())
        self.assertTrue(propset.HasKey("strmap"))
        o = propset.Lookup("strmap")
        strmap = _pyrt.StringMap._as(o)
        self.assertEqual(type(strmap), _pyrt.StringMap)

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()