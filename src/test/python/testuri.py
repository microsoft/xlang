import find_projection
import unittest

import pyrt.windows.foundation as wf

class TestUri(unittest.TestCase):

    def test_activate_uri(self):
        u = wf.Uri("http://microsoft.com")
        self.assertEqual(u.get_Domain(), "microsoft.com")
        self.assertEqual(u.get_AbsoluteCanonicalUri(), "http://microsoft.com/")
        self.assertEqual(u.get_Port(), 80)
        self.assertEqual(u.get_SchemeName(), "http")
        self.assertEqual(u.get_Suspicious(), False)
        self.assertEqual(u.get_Path(), "/")
        self.assertEqual(u.get_Query(), "")
        self.assertEqual(u.get_QueryParsed().get_Size(), 0)


    def test_activate_uri2(self):
        u = wf.Uri("http://microsoft.com", "surface/studio")
        self.assertEqual(u.get_Domain(), "microsoft.com")
        self.assertEqual(u.get_AbsoluteCanonicalUri(), "http://microsoft.com/surface/studio")
        self.assertEqual(u.get_Port(), 80)
        self.assertEqual(u.get_SchemeName(), "http")
        self.assertEqual(u.get_Suspicious(), False)
        self.assertEqual(u.get_Path(), "/surface/studio")
        self.assertEqual(u.get_Query(), "")
        self.assertEqual(u.get_QueryParsed().get_Size(), 0)


    def test_combine_uri(self):
        u1 = wf.Uri("http://microsoft.com")
        u = u1.CombineUri("surface/studio")
        self.assertEqual(u.get_Domain(), "microsoft.com")
        self.assertEqual(u.get_AbsoluteCanonicalUri(), "http://microsoft.com/surface/studio")
        self.assertEqual(u.get_Port(), 80)
        self.assertEqual(u.get_SchemeName(), "http")
        self.assertEqual(u.get_Suspicious(), False)
        self.assertEqual(u.get_Path(), "/surface/studio")
        self.assertEqual(u.get_Query(), "")
        self.assertEqual(u.get_QueryParsed().get_Size(), 0)

    def test_activate_query_parsed(self):
        u = wf.Uri("http://microsoft.com?projection=python&platform=windows")
        self.assertEqual(u.get_Query(), "?projection=python&platform=windows")

        qp = u.get_QueryParsed()
        self.assertEqual(qp.get_Size(), 2)

        self.assertEqual(qp.GetFirstValueByName("projection"), "python")
        self.assertEqual(qp.GetFirstValueByName("platform"), "windows")

        e0 = qp.GetAt(0)
        self.assertEqual(e0.get_Name(), "projection")
        self.assertEqual(e0.get_Value(), "python")

        e1 = qp.GetAt(1)
        self.assertEqual(e1.get_Name(), "platform")
        self.assertEqual(e1.get_Value(), "windows")

        # t = qp.IndexOf(e0)
        # self.assertTrue(t[0])
        # self.assertEqual(t[1], 0)

    

if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
