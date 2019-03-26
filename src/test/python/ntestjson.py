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

    def test_JsonArray_ctor(self):
        a = wdj.JsonArray()
        self.assertEqual(a.size, 0)
        self.assertEqual(a.value_type, JsonValueType.Array)
        self.assertEqual(a.to_string(), "[]")
        self.assertEqual(a.stringify(), "[]")

    def test_JsonObject_ctor(self):
        o = wdj.JsonObject()
        self.assertEqual(o.size, 0)
        self.assertEqual(o.value_type, JsonValueType.Object)
        self.assertEqual(o.to_string(), "{}")
        self.assertEqual(o.stringify(), "{}")

    def test_JsonArray_parse(self):
        a = wdj.JsonArray.parse('[true, false, 42, null, [], {}, "plugh"]')
        self.assertEqual(a.value_type, JsonValueType.Array)
        self.assertEqual(a.size, 7)
        self.assertTrue(a.get_boolean_at(0))
        self.assertFalse(a.get_boolean_at(1))
        self.assertEqual(a.get_number_at(2), 42)
        self.assertEqual(a.get_at(3).value_type, JsonValueType.Null)
        a2 = a.get_array_at(4)
        self.assertEqual(a2.size, 0)
        self.assertEqual(a2.value_type, JsonValueType.Array)
        o = a.get_object_at(5)
        self.assertEqual(o.size, 0)
        self.assertEqual(o.value_type, JsonValueType.Object)
        self.assertEqual(a.get_string_at(6), "plugh")

    def test_JsonArray_GetView(self):
        a = wdj.JsonArray.parse('[true, false, 42, null, [], {}, "plugh"]')
        view = a.get_view()

        self.assertEqual(view.size, 7)
        v0 = view.get_at(0)
        self.assertTrue(v0.get_boolean())
        v1 = view.get_at(1)
        self.assertFalse(v1.get_boolean())
        v2 = view.get_at(2)
        self.assertEqual(v2.get_number(), 42)
        v6 = view.get_at(6)
        self.assertEqual(v6.get_string(), "plugh")

        v4 = view.get_at(4)
        a4 = v4.get_array()
        self.assertEqual(a4.size, 0)

        v5 = view.get_at(5)
        o5 = v5.get_object()
        self.assertEqual(o5.size, 0)

    def test_JsonObject_parse(self):
        s = "{\"bool\": true,\"null\": null,\"number\": 42,\"string\": \"plugh\",\"array\": [1,2,3,4],\"object\": {}}"
        o = wdj.JsonObject.parse(s)
        self.assertEqual(o.value_type, JsonValueType.Object)
        self.assertEqual(o.size, 6)
        self.assertTrue(o.get_named_boolean("bool"))
        self.assertEqual(o.get_named_value("null").value_type, JsonValueType.Null)
        self.assertEqual(o.get_named_number("number"), 42)
        self.assertEqual(o.get_named_string("string"), "plugh")
        a2 = o.get_named_array("array")
        self.assertEqual(a2.size, 4)
        self.assertEqual(a2.value_type, JsonValueType.Array)
        o2 = o.get_named_object("object")
        self.assertEqual(o2.size, 0)
        self.assertEqual(o2.value_type, JsonValueType.Object)

    def test_JsonObject_GetView(self):
        s = "{\"bool\": true,\"null\": null,\"number\": 42,\"string\": \"plugh\",\"array\": [1,2,3,4],\"object\": {}}"
        o = wdj.JsonObject.parse(s)
        v = o.get_view()

        self.assertEqual(v.size, 6)
        
        self.assertTrue(v.lookup("bool").get_boolean())
        self.assertEqual(v.lookup("null").value_type, JsonValueType.Null)
        self.assertEqual(v.lookup("number").get_number(), 42)
        self.assertEqual(v.lookup("string").get_string(), "plugh")
        a2 = v.lookup("array").get_array()
        self.assertEqual(a2.size, 4)
        self.assertEqual(a2.value_type, JsonValueType.Array)
        o2 = v.lookup("object").get_object()
        self.assertEqual(o2.size, 0)
        self.assertEqual(o2.value_type, JsonValueType.Object)

    def test_JsonValue_boolean(self):
        t = wdj.JsonValue.create_boolean_value(True)
        self.assertEqual(t.value_type, JsonValueType.Boolean)
        self.assertTrue(t.get_boolean())

        f = wdj.JsonValue.create_boolean_value(False)
        self.assertEqual(f.value_type, JsonValueType.Boolean)
        self.assertFalse(f.get_boolean())

    def test_JsonValue_null(self):
        n = wdj.JsonValue.create_null_value()
        self.assertEqual(n.value_type, JsonValueType.Null)

    def test_JsonValue_number(self):
        t = wdj.JsonValue.create_number_value(42)
        self.assertEqual(t.value_type, JsonValueType.Number)
        self.assertEqual(t.get_number(), 42)

    def test_JsonValue_string(self):
        t = wdj.JsonValue.create_string_value("Plugh")
        self.assertEqual(t.value_type, JsonValueType.String)
        self.assertEqual(t.get_string(), "Plugh")

    def test_JsonValue_parse(self):
        b = wdj.JsonValue.parse("true")
        self.assertEqual(b.value_type, JsonValueType.Boolean)
        self.assertTrue(b.get_boolean())

        n = wdj.JsonValue.parse("16")
        self.assertEqual(n.value_type, JsonValueType.Number)
        self.assertEqual(n.get_number(), 16)

        s = wdj.JsonValue.parse("\"plugh\"")
        self.assertEqual(s.value_type, JsonValueType.String)
        self.assertEqual(s.get_string(), "plugh")

    def test_invalid_param_count_instance(self):
        a = wdj.JsonArray()
        with self.assertRaises(TypeError):
            a.append(10, 20)

    def test_invalid_param_count_static(self):
        with self.assertRaises(TypeError):
            wdj.JsonArray.parse(10, 20)

    # def test_JsonArray_GetMany(self):
    #     a = wdj.JsonArray.parse('[true, false, 42, null, [], {}, "plugh"]')
    #     count, items = a.GetMany(1, 3)

    #     self.assertEqual(count, 3)
    #     self.assertEqual(len(items), 3)

    #     self.assertFalse(items[0].get_boolean())
    #     self.assertEqual(items[1].get_number(), 42)
    #     self.assertEqual(items[2].value_type, JsonValueType.Null)

    # def test_JsonArray_GetMany2(self):
    #     a = wdj.JsonArray.parse('[true, false, 42, null, [], {}, "plugh"]')
    #     count, items = a.GetMany(6, 3)

    #     self.assertEqual(count, 1)
    #     self.assertEqual(len(items), 3)

    #     self.assertEqual(items[0].get_string(), "plugh")
    #     self.assertIsNone(items[1])
    #     self.assertIsNone(items[2])

if __name__ == '__main__':
    unittest.main()
