import find_projection

import _pyrt
import unittest

class TestXlangNumerics(unittest.TestCase):
    def test_struct_ctor_pos(self):
        r = _pyrt.Rational(2, 4)

        self.assertEqual(r.Numerator, 2)
        self.assertEqual(r.Denominator, 4)

    def test_struct_ctor_kwd(self):
        r = _pyrt.Rational(Denominator=2, Numerator=4)

        self.assertEqual(r.Numerator, 4)
        self.assertEqual(r.Denominator, 2)

    def test_struct_ctor_mix(self):
        r = _pyrt.Rational(3, Denominator=6)

        self.assertEqual(r.Numerator, 3)
        self.assertEqual(r.Denominator, 6)

    def test_vec3(self):
        v = _pyrt.Vector3(1.0, 2.0, 3.0)

        self.assertEqual(v.X, 1.0)
        self.assertEqual(v.Y, 2.0)
        self.assertEqual(v.Z, 3.0)

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()