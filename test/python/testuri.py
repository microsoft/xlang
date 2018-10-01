import find_projection

import _pyrt
import unittest

class TestXlangJson(unittest.TestCase):

    def test_activate_uri(self):
        u = _pyrt.Uri("http://microsoft.com")
        self.assertEqual(u.Domain, "microsoft.com")
        self.assertEqual(u.AbsoluteCanonicalUri, "http://microsoft.com/")
        self.assertEqual(u.Port, 80)
        self.assertEqual(u.SchemeName, "http")
        self.assertEqual(u.Suspicious, False)
        self.assertEqual(u.Path, "/")
        self.assertEqual(u.Query, "")
        self.assertEqual(u.QueryParsed.Size, 0)


    def test_activate_uri2(self):
        u = _pyrt.Uri("http://microsoft.com", "surface/studio")
        self.assertEqual(u.Domain, "microsoft.com")
        self.assertEqual(u.AbsoluteCanonicalUri, "http://microsoft.com/surface/studio")
        self.assertEqual(u.Port, 80)
        self.assertEqual(u.SchemeName, "http")
        self.assertEqual(u.Suspicious, False)
        self.assertEqual(u.Path, "/surface/studio")
        self.assertEqual(u.Query, "")
        self.assertEqual(u.QueryParsed.Size, 0)


    def test_combine_uri(self):
        u1 = _pyrt.Uri("http://microsoft.com")
        u = u1.CombineUri("surface/studio")
        self.assertEqual(u.Domain, "microsoft.com")
        self.assertEqual(u.AbsoluteCanonicalUri, "http://microsoft.com/surface/studio")
        self.assertEqual(u.Port, 80)
        self.assertEqual(u.SchemeName, "http")
        self.assertEqual(u.Suspicious, False)
        self.assertEqual(u.Path, "/surface/studio")
        self.assertEqual(u.Query, "")
        self.assertEqual(u.QueryParsed.Size, 0)

    def test_activate_query_parsed(self):
        u = _pyrt.Uri("http://microsoft.com?projection=python&platform=windows")
        self.assertEqual(u.Query, "?projection=python&platform=windows")

        qp = u.QueryParsed
        self.assertEqual(qp.Size, 2)

        self.assertEqual(qp.GetFirstValueByName("projection"), "python")
        self.assertEqual(qp.GetFirstValueByName("platform"), "windows")

        e0 = qp.GetAt(0)
        self.assertEqual(e0.Name, "projection")
        self.assertEqual(e0.Value, "python")

        e1 = qp.GetAt(1)
        self.assertEqual(e1.Name, "platform")
        self.assertEqual(e1.Value, "windows")

        # t = qp.IndexOf(e0)
        # self.assertTrue(t[0])
        # self.assertEqual(t[1], 0)

    





if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
