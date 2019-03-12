import loader_native, unittest

wgd = loader_native.import_ns("Windows.Graphics.DirectX")
wgdd = loader_native.import_ns("Windows.Graphics.DirectX.Direct3D11")

class TestDirectX(unittest.TestCase):

    def test_struct_containing_enum_pos(self):
        msd = wgdd.Direct3DMultisampleDescription(1, 2)
        sd = wgdd.Direct3DSurfaceDescription(4, 8, 10, msd) # 10 = DirectXPixelFormat.R16G16B16A16Float

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
    unittest.main()
