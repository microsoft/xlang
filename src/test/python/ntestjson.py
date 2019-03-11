import sys
sys.path.append("./generated")

import _pyrt
_pyrt.init_apartment()

def _import_ns(ns):
    import importlib.machinery
    import importlib.util
    try:
        module_name = "_pyrt_" + ns.replace('.', '_')
        loader = importlib.machinery.ExtensionFileLoader(module_name, _pyrt.__file__)
        spec = importlib.util.spec_from_loader(module_name, loader)
        module = importlib.util.module_from_spec(spec)
        loader.exec_module(module)
        return module
    except:
        return None

import unittest, enum

wdj = _import_ns("Windows.Data.Json")

a = wdj.JsonArray()

class JsonValueType(enum.IntEnum):
    Null = 0
    Boolean = 1
    Number = 2
    String = 3
    Array = 4
    Object = 5

class TestJson(unittest.TestCase):

    def test_activate_JsonArray(self):
        a = wdj.JsonArray._default_constructor()
        self.assertEqual(a.get_Size(), 0)
        self.assertEqual(a.get_ValueType(), JsonValueType.Array)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")

    def test_activate_JsonObject(self):
        o = wdj.JsonObject._default_constructor()
        self.assertEqual(o.get_Size(), 0)
        self.assertEqual(o.get_ValueType(), JsonValueType.Object)
        self.assertEqual(o.ToString(), "{}")
        self.assertEqual(o.Stringify(), "{}")

    def test_JsonArray_parse(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.get_ValueType(), JsonValueType.Array)
        self.assertEqual(a.get_Size(), 7)
        self.assertTrue(a.GetBooleanAt(0))
        self.assertFalse(a.GetBooleanAt(1))
        self.assertEqual(a.GetNumberAt(2), 42)
        self.assertEqual(a.GetStringAt(6), "plugh")

        a2 = a.GetArrayAt(4)
        self.assertEqual(a2.get_Size(), 0)
        self.assertEqual(a2.get_ValueType(), JsonValueType.Array)
        o = a.GetObjectAt(5)
        self.assertEqual(o.get_Size(), 0)
        self.assertEqual(o.get_ValueType(), JsonValueType.Object)

if __name__ == '__main__':
    unittest.main()
