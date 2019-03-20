import sys
sys.path.append("./generated")

import unittest

import winrt.windows.data.json as wdj

class TestJson(unittest.TestCase):

    def test_activate_JsonArray(self):
        a = wdj.JsonArray()
        self.assertEqual(a.size, 0)
        self.assertEqual(a.value_type, wdj.JsonValueType.ARRAY)
        self.assertEqual(a.to_string(), "[]")
        self.assertEqual(a.stringify(), "[]")

    def test_JsonArray_dunder_str(self):
        a = wdj.JsonArray()
        self.assertEqual(str(a), "[]")

    def test_JsonArray_parse(self):
        a = wdj.JsonArray.parse("[1,2,3,4,5]")
        self.assertEqual(a.size, 5)
        for x in range(0,4):
            self.assertEqual(a.get_number_at(x), x+1)

    def test_JsonArray_dunder_len(self):
        a = wdj.JsonArray.parse("[1,2,3,4,5]")
        self.assertEqual(len(a), 5)

    def test_JsonArray_remove_at(self):
        a = wdj.JsonArray.parse("[1,2,3,4,5]")
        self.assertEqual(a.size, 5)
        a.remove_at(0)
        self.assertEqual(a.size, 4)
        for x in range(0,3):
            self.assertEqual(a.get_number_at(x), x+2)

    def test_JsonArray_remove_at_end(self):
        a = wdj.JsonArray.parse("[1,2,3,4,5]")
        self.assertEqual(a.size, 5)
        a.remove_at_end()
        self.assertEqual(a.size, 4)
        for x in range(0,3):
            self.assertEqual(a.get_number_at(x), x+1)

    def test_JsonArray_try_parse(self):
        succeeded, a = wdj.JsonArray.try_parse("[1,2,3,4,5]")
        self.assertTrue(succeeded)
        self.assertEqual(a.size, 5)
        for x in range(0,4):
            self.assertEqual(a.get_number_at(x), x+1)

    def test_JsonArray_try_parse_fail(self):
        succeeded, a = wdj.JsonArray.try_parse("z[1,2,3,4,5]")
        self.assertFalse(succeeded)

    def test_JsonArray_get_array_at(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        v1 = a.get_array_at(1)
        self.assertEqual(v1.size, 0)
        self.assertEqual(v1.value_type, wdj.JsonValueType.ARRAY)

    def test_JsonArray_clear(self):
        a = wdj.JsonArray.parse("[1,2,3,4,5]")
        self.assertEqual(a.size, 5)
        a.clear()
        self.assertEqual(a.size, 0)

    def test_JsonArray_get_array(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        v1 = a.get_array()
        self.assertEqual(v1.size, 3)
        self.assertEqual(v1.value_type, wdj.JsonValueType.ARRAY)

    def test_JsonArray_get_boolean(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        with self.assertRaises(RuntimeError):
            v1 = a.get_boolean()

    def test_JsonArray_get_number(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        with self.assertRaises(RuntimeError):
            v1 = a.get_number()

    def test_JsonArray_get_string(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        with self.assertRaises(RuntimeError):
            v1 = a.get_string()

    def test_JsonArray_get_array_at(self):
        a = wdj.JsonArray.parse("[true, [], false]")
        v1 = a.get_array_at(1)
        self.assertEqual(v1.size, 0)
        self.assertEqual(v1.value_type, wdj.JsonValueType.ARRAY)

    def test_JsonArray_get_object(self):
        a = wdj.JsonArray.parse("[true, {}, false]")
        with self.assertRaises(RuntimeError):
            v1 = a.get_object()

    def test_JsonArray_get_object_at(self):
        a = wdj.JsonArray.parse("[true, {}, false]")
        v1 = a.get_object_at(1)
        self.assertEqual(v1.size, 0)
        self.assertEqual(v1.value_type, wdj.JsonValueType.OBJECT)

    def test_JsonArray_get_string_at(self):
        a = wdj.JsonArray.parse("[true, \"spam\", false]")
        v1 = a.get_string_at(1)
        self.assertEqual(v1, "spam")

    def test_JsonArray_get_boolean_at(self):
        a = wdj.JsonArray.parse("[true, false]")
        v1 = a.get_boolean_at(0)
        v2 = a.get_boolean_at(1)
        self.assertTrue(v1)
        self.assertFalse(v2)

    def test_JsonArray_get_at(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = a.get_at(0)
        self.assertEqual(v.value_type, wdj.JsonValueType.NULL)
        v = a.get_at(1)
        self.assertEqual(v.value_type, wdj.JsonValueType.BOOLEAN)
        v = a.get_at(2)
        self.assertEqual(v.value_type, wdj.JsonValueType.NUMBER)
        v = a.get_at(3)
        self.assertEqual(v.value_type, wdj.JsonValueType.STRING)
        v = a.get_at(4)
        self.assertEqual(v.value_type, wdj.JsonValueType.ARRAY)
        v = a.get_at(5)
        self.assertEqual(v.value_type, wdj.JsonValueType.OBJECT)

    def test_JsonArray_index_of(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = a.get_at(3)
        found, index = a.index_of(v)
        self.assertTrue(found)
        self.assertEqual(index, 3)

    def test_JsonArray_append(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = wdj.JsonValue.create_string_value("the larch")
        self.assertEqual(a.size, 6)
        a.append(v)
        self.assertEqual(a.size, 7)
        found, index = a.index_of(v)
        self.assertTrue(found)
        self.assertEqual(index, 6)

    def test_IJsonvalue_get_boolean(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = a.get_at(1)
        self.assertTrue(v.get_boolean())

    def test_IJsonvalue_get_number(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = a.get_at(2)
        self.assertEqual(v.get_number(), 42)

    def test_IJsonvalue_get_string(self):
        a = wdj.JsonArray.parse("[null, true, 42, \"spam\", [1,2,3], {\"scene\":24}]")
        v = a.get_at(3)
        self.assertEqual(v.get_string(), "spam")

    def test_JsonValue_create_number_value(self):
        v = wdj.JsonValue.create_number_value(42)
        self.assertEqual(v.value_type, wdj.JsonValueType.NUMBER)
        self.assertEqual(v.get_number(), 42)

    def test_JsonValue_create_boolean_value(self):
        v = wdj.JsonValue.create_boolean_value(True)
        self.assertEqual(v.value_type, wdj.JsonValueType.BOOLEAN)
        self.assertEqual(v.get_boolean(), True)

    def test_JsonValue_create_string_value(self):
        v = wdj.JsonValue.create_string_value("spam")
        self.assertEqual(v.value_type, wdj.JsonValueType.STRING)
        self.assertEqual(v.get_string(), "spam")

    def test_JsonValue_create_null_value(self):
        v = wdj.JsonValue.create_null_value()
        self.assertEqual(v.value_type, wdj.JsonValueType.NULL)
        
    def test_JsonObject_get_named_boolean(self):
        o = wdj.JsonObject.parse("{ \"spam\": true }")
        v = o.get_named_boolean("spam")
        self.assertTrue(v)

    def test_JsonObject_str(self):
        a = wdj.JsonObject()
        self.assertEqual(str(a), "{}")

    def test_JsonObject_get_named_boolean_default(self):
        o = wdj.JsonObject.parse("{ \"spam\": true }")
        v = o.get_named_boolean("more-spam", True)
        self.assertTrue(v)

    def test_JsonObject_get_named_number(self):
        o = wdj.JsonObject.parse("{ \"spam\": 42 }")
        v = o.get_named_number("spam")
        self.assertEqual(v, 42)

    def test_JsonObject_get_named_number_default(self):
        o = wdj.JsonObject.parse("{ \"spam\": true }")
        v = o.get_named_number("more-spam", 16)
        self.assertEqual(v, 16)

# todo: GetMany, iterator, sequence

if __name__ == '__main__':
    unittest.main()
