import find_projection
import unittest

import pyrt.windows.graphics.directx.direct3d11 as wgdd

class TestDirectX(unittest.TestCase):

    def test_struct_containing_enum_pos(self):
        msd = wgdd.Direct3DMultisampleDescription(1, 2)
        sd = wgdd.Direct3DSurfaceDescription(4, 8, 10, msd)

        self.assertEqual(sd.Width, 4)
        self.assertEqual(sd.Height, 8)
        self.assertEqual(sd.Format, 10)

        msd2 = sd.MultisampleDescription
        self.assertEqual(msd.Count, 1)
        self.assertEqual(msd.Quality, 2)
    
    def test_struct_containing_enum_kwd(self):
        msd = wgdd.Direct3DMultisampleDescription(1, 2)
        sd = wgdd.Direct3DSurfaceDescription(Format=10, Width=4, MultisampleDescription=msd, Height=8)

        self.assertEqual(sd.Width, 4)
        self.assertEqual(sd.Height, 8)
        self.assertEqual(sd.Format, 10)

        msd2 = sd.MultisampleDescription
        self.assertEqual(msd.Count, 1)
        self.assertEqual(msd.Quality, 2)

if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
