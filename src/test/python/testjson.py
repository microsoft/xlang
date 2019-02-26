import unittest

import pyrt.windows.data.json as wdj

class TestJson(unittest.TestCase):

    def test_activate_JsonArray(self):
        a = wdj.JsonArray()
        self.assertEqual(a.Size, 0)
        self.assertEqual(a.ValueType, wdj.JsonValueType.Array)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")

    def test_JsonArray_str(self):
        a = wdj.JsonArray()
        self.assertEqual(str(a), "[]")

    def test_activate_JsonObject(self):
        o = wdj.JsonObject()
        self.assertEqual(o.Size, 0)
        self.assertEqual(o.ValueType, wdj.JsonValueType.Object)
        self.assertEqual(o.ToString(), "{}")
        self.assertEqual(o.Stringify(), "{}")

    def test_cant_activate_JsonError(self):
        with self.assertRaises(TypeError):
            e = wdj.JsonError()

    def test_cant_activate_JsonValue(self):
        with self.assertRaises(TypeError):
            v = wdj.JsonValue()

    def test_JsonArray_iter(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        count = 0
        for x in a:
            count = count + 1
        self.assertEqual(a.Size, count)

    def test_JsonArray_len(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.Size, len(a))

    def test_JsonArray_get_item(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertTrue(a[0].GetBoolean())
        self.assertFalse(a[1].GetBoolean())
        self.assertEqual(42, a[2].GetNumber())
        self.assertEqual(wdj.JsonValueType.Null, a[3].ValueType)
        self.assertEqual(wdj.JsonValueType.Array, a[4].ValueType)
        self.assertEqual(wdj.JsonValueType.Object, a[5].ValueType)
        self.assertEqual("plugh", a[6].GetString())

    def test_JsonArray_set_item(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')

        self.assertEqual(wdj.JsonValueType.Null, a[3].ValueType)
        a[3] = wdj.JsonValue.CreateNumberValue(3.14)
        self.assertEqual(wdj.JsonValueType.Number, a[3].ValueType)
        self.assertEqual(3.14, a[3].GetNumber())

    def test_JsonArray_parse(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.ValueType, wdj.JsonValueType.Array)
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
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
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

    def test_JsonObject_len(self):
        o = wdj.JsonObject.Parse('{ "king": "Arthur", "count": 3, "squire": "Patsy", "knights": ["Bedevere", "Lancelot", "Galahad", "Robin"]}')
        self.assertEqual(o.Size, len(o))

    def test_JsonObject_get_item(self):
        o = wdj.JsonObject.Parse('{ "king": "Arthur", "count": 3, "squire": "Patsy", "knights": ["Bedevere", "Lancelot", "Galahad", "Robin"]}')
        self.assertEqual(3, o["count"].GetNumber())
        self.assertEqual("Arthur", o["king"].GetString())
        self.assertEqual("Patsy", o["squire"].GetString())
        self.assertEqual(4, o["knights"].ValueType)

    def test_JsonObject_set_item(self):
        o = wdj.JsonObject.Parse('{ "king": "Arthur", "count": 3, "squire": "Patsy", "knights": ["Bedevere", "Lancelot", "Galahad", "Robin"]}')
        self.assertEqual(4, o.Size)
        o["camelot"] =  wdj.JsonValue.CreateStringValue("a silly place")
        self.assertEqual(5, o.Size)

    def test_JsonValue_boolean(self):
        t = wdj.JsonValue.CreateBooleanValue(True)
        self.assertEqual(t.ValueType, wdj.JsonValueType.Boolean)
        self.assertTrue(t.GetBoolean())

        f = wdj.JsonValue.CreateBooleanValue(False)
        self.assertEqual(f.ValueType, wdj.JsonValueType.Boolean)
        self.assertFalse(f.GetBoolean())

    def test_JsonValue_null(self):
        n = wdj.JsonValue.CreateNullValue()
        self.assertEqual(n.ValueType, wdj.JsonValueType.Null)

    def test_JsonValue_number(self):
        t = wdj.JsonValue.CreateNumberValue(42)
        self.assertEqual(t.ValueType, wdj.JsonValueType.Number)
        self.assertEqual(t.GetNumber(), 42)

    def test_JsonValue_string(self):
        t = wdj.JsonValue.CreateStringValue("Plugh")
        self.assertEqual(t.ValueType, wdj.JsonValueType.String)
        self.assertEqual(t.GetString(), "Plugh")

    def test_JsonValue_parse(self):
        b = wdj.JsonValue.Parse("true")
        self.assertEqual(b.ValueType, wdj.JsonValueType.Boolean)
        self.assertTrue(b.GetBoolean())

        n = wdj.JsonValue.Parse("16")
        self.assertEqual(n.ValueType, wdj.JsonValueType.Number)
        self.assertEqual(n.GetNumber(), 16)

        s = wdj.JsonValue.Parse("\"plugh\"")
        self.assertEqual(s.ValueType, wdj.JsonValueType.String)
        self.assertEqual(s.GetString(), "plugh")

    def test_invalid_param_count_instance(self):
        a = wdj.JsonArray()
        with self.assertRaises(TypeError):
            a.Append(10, 20)

    def test_invalid_param_count_static(self):
        with self.assertRaises(TypeError):
            wdj.JsonArray.Parse(10, 20)

    def test_JsonArray_GetMany(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        count, items = a.GetMany(1, 3)

        self.assertEqual(count, 3)
        self.assertEqual(len(items), 3)

        self.assertFalse(items[0].GetBoolean())
        self.assertEqual(items[1].GetNumber(), 42)
        self.assertEqual(items[2].ValueType, wdj.JsonValueType.Null)

        # TODO: remove clear call after resolving leak issue
        items.clear()

    def test_JsonArray_GetMany2(self):
        a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
        count, items = a.GetMany(6, 3)

        self.assertEqual(count, 1)
        self.assertEqual(len(items), 3)

        self.assertEqual(items[0].GetString(), "plugh")
        self.assertIsNone(items[1])
        self.assertIsNone(items[2])

        # TODO: remove clear call after resolving leak issue
        items.clear()


if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
