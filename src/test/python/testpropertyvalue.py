import sys
sys.path.append("./generated")

import unittest

import pyrt.windows.foundation as wf

class TestPropertyValue(unittest.TestCase):

    # TODO: CreateEmpty seems to be failing @ the C++/WinRT layer. 

    def test_create_uint8(self):
        o = wf.PropertyValue.CreateUInt8(250)
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.UInt8)
        self.assertTrue(ipv.GetUInt8(), 250)

    def test_create_int16(self):
       o = wf.PropertyValue.CreateInt16(-32000)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.Int16)
       self.assertTrue(ipv.GetInt16(), -32000)

    def test_create_uint16(self):
       o = wf.PropertyValue.CreateUInt16(65000)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.UInt16)
       self.assertTrue(ipv.GetUInt16(), 65000)

    def test_create_int32(self):
       o = wf.PropertyValue.CreateInt32(-2147483640)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.Int32)
       self.assertTrue(ipv.GetInt32(), -2147483640)

    def test_create_uint32(self):
       o = wf.PropertyValue.CreateUInt32(4294967290)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.UInt32)
       self.assertTrue(ipv.GetUInt32(), 4294967290)

    def test_create_int64(self):
       o = wf.PropertyValue.CreateInt64(-9223372036854775800)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.Int64)
       self.assertTrue(ipv.GetInt64(), -9223372036854775800)

    def test_create_uint64(self):
       o = wf.PropertyValue.CreateUInt64(18446744073709551610)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.UInt64)
       self.assertTrue(ipv.GetUInt64(), 18446744073709551610)

    def test_create_single(self):
        o = wf.PropertyValue.CreateSingle(3.14)
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.Single)
        self.assertAlmostEqual(ipv.GetSingle(), 3.14, 5)

    def test_create_double(self):
        o = wf.PropertyValue.CreateDouble(3.14)
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.Double)
        self.assertEqual(ipv.GetDouble(), 3.14)

    # # TODO: CreateChar16

    def test_create_boolean(self):
       o = wf.PropertyValue.CreateBoolean(True)
       ipv = wf.IPropertyValue._from(o)
       self.assertEqual(ipv.Type, wf.PropertyType.Boolean)
       self.assertTrue(ipv.GetBoolean())

    def test_create_string(self):
        o = wf.PropertyValue.CreateString("Ni!")
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.String)
        self.assertEqual(ipv.GetString(), "Ni!")

    # # TODO: CreateInspectable

    def test_create_datetime(self):
        o = wf.PropertyValue.CreateDateTime(wf.DateTime(0))
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.DateTime)
        self.assertEqual(ipv.GetDateTime().UniversalTime, 0)

    def test_create_TimeSpan(self):
        o = wf.PropertyValue.CreateTimeSpan(wf.TimeSpan(0))
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.TimeSpan)
        self.assertEqual(ipv.GetTimeSpan().Duration, 0)        

    # # TODO: CreateGuid

    def test_create_Point(self):
        o = wf.PropertyValue.CreatePoint(wf.Point(2, 4))
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.Point)
        s = ipv.GetPoint()
        self.assertEqual(s.X, 2)
        self.assertEqual(s.Y, 4)

    def test_create_Size(self):
        o = wf.PropertyValue.CreateSize(wf.Size(2, 4))
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.Size)
        s = ipv.GetSize()
        self.assertEqual(s.Width, 2)
        self.assertEqual(s.Height, 4)

    def test_create_Rect(self):
        o = wf.PropertyValue.CreateRect(wf.Rect(2, 4, 6, 8))
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.Rect)
        s = ipv.GetRect()
        self.assertEqual(s.X, 2)
        self.assertEqual(s.Y, 4)
        self.assertEqual(s.Width, 6)
        self.assertEqual(s.Height, 8)

    def test_create_uint8_array(self):
        o = wf.PropertyValue.CreateUInt8Array([1,2,3,4,5])
        ipv = wf.IPropertyValue._from(o)
        self.assertEqual(ipv.Type, wf.PropertyType.UInt8Array)
        a = ipv.GetUInt8Array()
        self.assertEqual(len(a), 5)
        for x in range(0,5):
            self.assertEqual(a[x], x+1)

if __name__ == '__main__':
    unittest.main()
