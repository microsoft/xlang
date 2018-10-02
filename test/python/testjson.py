import find_projection

import _pyrt
import unittest

class TestXlangJson(unittest.TestCase):

    def test_activate_JsonArray(self):
        a = _pyrt.JsonArray()
        self.assertEqual(a.Size, 0)
        self.assertEqual(a.ValueType, 4)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")


    def test_activate_JsonObject(self):
        o = _pyrt.JsonObject()
        self.assertEqual(o.Size, 0)
        self.assertEqual(o.ValueType, 5)
        self.assertEqual(o.ToString(), "{}")
        self.assertEqual(o.Stringify(), "{}")

    def test_cant_activate_JsonError(self):
        with self.assertRaises(TypeError):
            e = _pyrt.JsonError()

    def test_cant_activate_JsonValue(self):
        with self.assertRaises(TypeError):
            v = _pyrt.JsonValue()

    def test_JsonArray_parse(self):
        a = _pyrt.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.ValueType, 4)
        self.assertEqual(a.Size, 7)
        self.assertTrue(a.GetBooleanAt(0))
        self.assertFalse(a.GetBooleanAt(1))
        self.assertEqual(a.GetNumberAt(2), 42)
        self.assertEqual(a.GetStringAt(6), "plugh")

        a2 = a.GetArrayAt(4)
        self.assertEqual(a2.Size, 0)
        o = a.GetObjectAt(5)
        self.assertEqual(o.Size, 0)

    def test_JsonArray_GetView(self):
        a = _pyrt.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        view = a.GetView()

        self.assertEqual(view.Size, 7)
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
        self.assertEqual(a4.Size, 0)

        v5 = view.GetAt(5)
        o5 = v5.GetObject()
        self.assertEqual(o5.Size, 0)


    def test_JsonValue_boolean(self):
        t = _pyrt.JsonValue.CreateBooleanValue(True)
        self.assertEqual(t.ValueType, 1)
        self.assertTrue(t.GetBoolean())

        f = _pyrt.JsonValue.CreateBooleanValue(False)
        self.assertEqual(f.ValueType, 1)
        self.assertFalse(f.GetBoolean())

    def test_JsonValue_null(self):
        n = _pyrt.JsonValue.CreateNullValue()
        self.assertEqual(n.ValueType, 0)

    def test_JsonValue_number(self):
        t = _pyrt.JsonValue.CreateNumberValue(42)
        self.assertEqual(t.ValueType, 2)
        self.assertEqual(t.GetNumber(), 42)

    # def test_JsonArray(self):
    #     a = _pyrt.JsonArray()
    #     v = _pyrt.JsonValue.CreateNumberValue(3)
    #     a.InsertAt(0, _pyrt.JsonValue.CreateNumberValue(3))
    #     a.InsertAt(0, _pyrt.JsonValue.CreateNumberValue(2))
    #     a.InsertAt(0, _pyrt.JsonValue.CreateNumberValue(1))



    def test_JsonValue_string(self):
        t = _pyrt.JsonValue.CreateStringValue("Plugh")
        self.assertEqual(t.ValueType, 3)
        self.assertEqual(t.GetString(), "Plugh")

    def test_JsonValue_parse(self):
        b = _pyrt.JsonValue.Parse("true")
        self.assertEqual(b.ValueType, 1)
        self.assertTrue(b.GetBoolean())

        n = _pyrt.JsonValue.Parse("16")
        self.assertEqual(n.ValueType, 2)
        self.assertEqual(n.GetNumber(), 16)

        s = _pyrt.JsonValue.Parse("\"plugh\"")
        self.assertEqual(s.ValueType, 3)
        self.assertEqual(s.GetString(), "plugh")

    def test_invalid_param_count_instance(self):
        a = _pyrt.JsonArray()
        with self.assertRaises(RuntimeError):
            a.Append(10, 20)

    def test_invalid_param_count_static(self):
        with self.assertRaises(RuntimeError):
            _pyrt.JsonArray.Parse(10, 20)


if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
