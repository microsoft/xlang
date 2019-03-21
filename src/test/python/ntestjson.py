import loader_native, unittest, enum

# need to ensure WFC is initialized 
wfc = loader_native.import_ns("Windows.Foundation.Collections")
wdj = loader_native.import_ns("Windows.Data.Json")

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
        a = wdj.JsonArray._default_ctor()
        self.assertEqual(a.get_Size(), 0)
        self.assertEqual(a.get_ValueType(), JsonValueType.Array)
        self.assertEqual(a.ToString(), "[]")
        self.assertEqual(a.Stringify(), "[]")

    def test_JsonObject_ctor(self):
        o = wdj.JsonObject._default_ctor()
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
        s = "{\"bool\": true,\"null\": null,\"number\": 42,\"string\": \"plugh\",\"array\": [1,2,3,4],\"object\": {}}"
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

    def test_JsonObject_GetView(self):
        s = "{\"bool\": true,\"null\": null,\"number\": 42,\"string\": \"plugh\",\"array\": [1,2,3,4],\"object\": {}}"
        o = wdj.JsonObject.Parse(s)
        v = o.GetView()

        self.assertEqual(v.get_Size(), 6)
        
        self.assertTrue(v.Lookup("bool").GetBoolean())
        self.assertEqual(v.Lookup("null").get_ValueType(), JsonValueType.Null)
        self.assertEqual(v.Lookup("number").GetNumber(), 42)
        self.assertEqual(v.Lookup("string").GetString(), "plugh")
        a2 = v.Lookup("array").GetArray()
        self.assertEqual(a2.get_Size(), 4)
        self.assertEqual(a2.get_ValueType(), JsonValueType.Array)
        o2 = v.Lookup("object").GetObject()
        self.assertEqual(o2.get_Size(), 0)
        self.assertEqual(o2.get_ValueType(), JsonValueType.Object)

    def test_JsonValue_boolean(self):
        t = wdj.JsonValue.CreateBooleanValue(True)
        self.assertEqual(t.get_ValueType(), JsonValueType.Boolean)
        self.assertTrue(t.GetBoolean())

        f = wdj.JsonValue.CreateBooleanValue(False)
        self.assertEqual(f.get_ValueType(), JsonValueType.Boolean)
        self.assertFalse(f.GetBoolean())

    def test_JsonValue_null(self):
        n = wdj.JsonValue.CreateNullValue()
        self.assertEqual(n.get_ValueType(), JsonValueType.Null)

    def test_JsonValue_number(self):
        t = wdj.JsonValue.CreateNumberValue(42)
        self.assertEqual(t.get_ValueType(), JsonValueType.Number)
        self.assertEqual(t.GetNumber(), 42)

    def test_JsonValue_string(self):
        t = wdj.JsonValue.CreateStringValue("Plugh")
        self.assertEqual(t.get_ValueType(), JsonValueType.String)
        self.assertEqual(t.GetString(), "Plugh")

    def test_JsonValue_parse(self):
        b = wdj.JsonValue.Parse("true")
        self.assertEqual(b.get_ValueType(), JsonValueType.Boolean)
        self.assertTrue(b.GetBoolean())

        n = wdj.JsonValue.Parse("16")
        self.assertEqual(n.get_ValueType(), JsonValueType.Number)
        self.assertEqual(n.GetNumber(), 16)

        s = wdj.JsonValue.Parse("\"plugh\"")
        self.assertEqual(s.get_ValueType(), JsonValueType.String)
        self.assertEqual(s.GetString(), "plugh")

    def test_invalid_param_count_instance(self):
        a = wdj.JsonArray._default_ctor()
        with self.assertRaises(TypeError):
            a.Append(10, 20)

    def test_invalid_param_count_static(self):
        with self.assertRaises(TypeError):
            wdj.JsonArray.Parse(10, 20)

    # def test_JsonArray_GetMany(self):
    #     a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
    #     count, items = a.GetMany(1, 3)

    #     self.assertEqual(count, 3)
    #     self.assertEqual(len(items), 3)

    #     self.assertFalse(items[0].GetBoolean())
    #     self.assertEqual(items[1].GetNumber(), 42)
    #     self.assertEqual(items[2].get_ValueType(), JsonValueType.Null)

    # def test_JsonArray_GetMany2(self):
    #     a = wdj.JsonArray.Parse('[true, false, 42, null, [], {}, "plugh"]')
    #     count, items = a.GetMany(6, 3)

    #     self.assertEqual(count, 1)
    #     self.assertEqual(len(items), 3)

    #     self.assertEqual(items[0].GetString(), "plugh")
    #     self.assertIsNone(items[1])
    #     self.assertIsNone(items[2])

if __name__ == '__main__':
    unittest.main()
