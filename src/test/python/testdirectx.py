import unittest

import pyrt.windows.graphics.directx as wgd
import pyrt.windows.graphics.directx.direct3d11 as wgdd

class TestDirectX(unittest.TestCase):

    def test_struct_containing_enum_pos(self):
        msd = wgdd.Direct3DMultisampleDescription(1, 2)
        sd = wgdd.Direct3DSurfaceDescription(4, 8, wgd.DirectXPixelFormat.R16G16B16A16Float, msd)

        self.assertEqual(sd.Width, 4)
        self.assertEqual(sd.Height, 8)
        self.assertEqual(sd.Format, wgd.DirectXPixelFormat.R16G16B16A16Float)

        msd2 = sd.MultisampleDescription
        self.assertEqual(msd.Count, 1)
        self.assertEqual(msd.Quality, 2)
    
    def test_struct_containing_enum_kwd(self):
        msd = wgdd.Direct3DMultisampleDescription(1, 2)
        sd = wgdd.Direct3DSurfaceDescription(Format=wgd.DirectXPixelFormat.R16G16B16A16Float, Width=4, MultisampleDescription=msd, Height=8)

        self.assertEqual(sd.Width, 4)
        self.assertEqual(sd.Height, 8)
        self.assertEqual(sd.Format, wgd.DirectXPixelFormat.R16G16B16A16Float)

        msd2 = sd.MultisampleDescription
        self.assertEqual(msd.Count, 1)
        self.assertEqual(msd.Quality, 2)

if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
