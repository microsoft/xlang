import find_projection
import unittest

import pyrt.windows.foundation.collections as wfc

class TestQueryInterface(unittest.TestCase):
    def test_as_function(self):
        propset = wfc.PropertySet()
        propset.Insert("strmap", wfc.StringMap())
        self.assertTrue(propset.HasKey("strmap"))
        o = propset.Lookup("strmap")
        strmap = wfc.StringMap._from(o)
        self.assertEqual(type(strmap), wfc.StringMap)

if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
