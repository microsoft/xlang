from winrt import initialize
import Windows.Foundation as wf
import unittest

initialize()

class TestUri(unittest.TestCase):
    def test_EscapeComponent(self):
        expected = "this%20is%20a%20test%3F"
        actual = wf.Uri.EscapeComponent("this is a test?")
        self.assertEqual(expected, actual)
 
    def test_UnescapeComponent(self):
        expected = "this is a test?"
        actual = wf.Uri.UnescapeComponent("this%20is%20a%20test%3F")
        self.assertEqual(expected, actual)

    def test_EscapeUnescapeComponent(self):
        component = "this is a test?"
        escaped = wf.Uri.EscapeComponent(component)
        unescaped = wf.Uri.UnescapeComponent(escaped)
        self.assertEqual(component, unescaped)

    def test_CreateUri(self):
        uri = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        self.assertEqual(uri.DisplayUri, "http://microsoft.com/surface/studio")

    def test_CreateWithRelativeUri(self):
        uri = wf.Uri.CreateWithRelativeUri("http://microsoft.com", "surface/studio")
        self.assertEqual(uri.DisplayUri, "http://microsoft.com/surface/studio")

    def test_Domain(self):
        uri = wf.Uri.CreateWithRelativeUri("http://microsoft.com", "surface/studio")
        self.assertEqual(uri.Domain, "microsoft.com")

    # def test_QueryParsed(self):

    def test_Port(self):
        uri = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        self.assertEqual(uri.Port, 80)

    def test_Suspicious(self):
        uri = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        self.assertFalse(uri.Suspicious)

    def test_Equals(self):
        uri1 = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        uri2 = wf.Uri.CreateWithRelativeUri("http://microsoft.com", "surface/studio")
        self.assertTrue(uri1.Equals(uri2))

    def test_CombineUri(self):
        uri1 = wf.Uri.CreateUri("http://microsoft.com")
        uri2 = uri1.CombineUri("surface/studio")
        self.assertEqual(uri2.DisplayUri, "http://microsoft.com/surface/studio")

    def test_AbsoluteCanonicalUri(self):
        uri = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        self.assertEqual(uri.AbsoluteCanonicalUri, "http://microsoft.com/surface/studio")

    def test_ToString(self):
        uri = wf.Uri.CreateUri("http://microsoft.com/surface/studio")
        self.assertEqual(uri.ToString(), "http://microsoft.com/surface/studio")

unittest.main()