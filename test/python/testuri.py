from inspect import currentframe, getframeinfo
from pathlib import Path
import sys

filename = getframeinfo(currentframe()).filename
langworthy_root = Path(filename).resolve().parent.parent.parent.parent
python_build_root = langworthy_root / ("_build/Windows/x86/Debug/tool/python/output/build/lib.win-amd64-" + sys.version[0:3])

print(python_build_root)

sys.path.append(str(python_build_root))

import _xlang
import unittest

class TestXlangJson(unittest.TestCase):

    def test_activate_uri(self):
        u = _xlang.Uri("http://microsoft.com")
        self.assertEqual(u.Domain, "microsoft.com")
        self.assertEqual(u.AbsoluteCanonicalUri, "http://microsoft.com/")
        self.assertEqual(u.Port, 80)
        self.assertEqual(u.SchemeName, "http")
        self.assertEqual(u.Suspicious, False)
        self.assertEqual(u.Path, "/")
        self.assertEqual(u.Query, "")
        self.assertEqual(u.QueryParsed.Size, 0)


    def test_activate_uri2(self):
        u = _xlang.Uri("http://microsoft.com", "surface/studio")
        self.assertEqual(u.Domain, "microsoft.com")
        self.assertEqual(u.AbsoluteCanonicalUri, "http://microsoft.com/surface/studio")
        self.assertEqual(u.Port, 80)
        self.assertEqual(u.SchemeName, "http")
        self.assertEqual(u.Suspicious, False)
        self.assertEqual(u.Path, "/surface/studio")
        self.assertEqual(u.Query, "")
        self.assertEqual(u.QueryParsed.Size, 0)


    def test_combine_uri(self):
        u1 = _xlang.Uri("http://microsoft.com")
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
        u = _xlang.Uri("http://microsoft.com?projection=python&platform=windows")
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
    _xlang.init_apartment()
    unittest.main()
    _xlang.uninit_apartment()
