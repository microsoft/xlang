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

# need to ensure WFC is initialized 
wfc = _import_ns("Windows.Foundation.Collections")
wdj = _import_ns("Windows.Data.Json")

class JsonValueType(enum.IntEnum):
    Null = 0
    Boolean = 1
    Number = 2
    String = 3
    Array = 4
    Object = 5

class TestJson(unittest.TestCase):

    def test_cant_new_JsonArray(self):
        with self.assertRaises(TypeError):
            e = wdj.JsonArray()

    def test_JsonArray_ctor(self):
        a = wdj.JsonArray._ctor()
        self.assertEqual(a.get_Size(), 0)
        self.assertEqual(a.get_ValueType(), JsonValueType.Array)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")

    def test_JsonObject_ctor(self):
        o = wdj.JsonObject._ctor()
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
        self.assertEqual(a.GetAt(3).get_ValueType(), JsonValueType.Null)
        a2 = a.GetArrayAt(4)
        self.assertEqual(a2.get_Size(), 0)
        self.assertEqual(a2.get_ValueType(), JsonValueType.Array)
        o = a.GetObjectAt(5)
        self.assertEqual(o.get_Size(), 0)
        self.assertEqual(o.get_ValueType(), JsonValueType.Object)
        self.assertEqual(a.GetStringAt(6), "plugh")

    def test_JsonArray_GetView(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        view = a.GetView()

        self.assertEqual(view.get_Size(), 7)
        v0 = view.GetAt(0)
        self.assertTrue(v0.GetBoolean())
        v1 = view.GetAt(1)
        self.assertFalse(v1.GetBoolean())
        v2 = view.GetAt(2)
        self.assertEqual(v2.GetNumber(), 42)
        v6 = view.GetAt(6)
        self.assertEqual(v6.GetString(), "plugh")

        v4 = view.GetAt(4)
        a4 = v4.GetArray()
        self.assertEqual(a4.get_Size(), 0)

        v5 = view.GetAt(5)
        o5 = v5.GetObject()
        self.assertEqual(o5.get_Size(), 0)

    def test_JsonObject_Parse(self):
        s = """
{
    "bool": true,
    "null": null,
    "number": 42,
    "string": "plugh",
    "array": [1,2,3,4],
    "object": {}
}"""
        o = wdj.JsonObject.Parse(s)
        self.assertEqual(o.get_ValueType(), JsonValueType.Object)
        self.assertEqual(o.get_Size(), 6)
        self.assertTrue(o.GetNamedBoolean("bool"))
        self.assertEqual(o.GetNamedValue("null").get_ValueType(), JsonValueType.Null)
        self.assertEqual(o.GetNamedNumber("number"), 42)
        self.assertEqual(o.GetNamedString("string"), "plugh")
        a2 = o.GetNamedArray("array")
        self.assertEqual(a2.get_Size(), 4)
        self.assertEqual(a2.get_ValueType(), JsonValueType.Array)
        o2 = o.GetNamedObject("object")
        self.assertEqual(o2.get_Size(), 0)
        self.assertEqual(o2.get_ValueType(), JsonValueType.Object)

if __name__ == '__main__':
    unittest.main()
