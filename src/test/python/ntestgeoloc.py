import loader_native, unittest, asyncio

wf = loader_native.import_ns("Windows.Foundation")
wdg = loader_native.import_ns("Windows.Devices.Geolocation")

# async_test inspired by https://stackoverflow.com/a/23036785
def async_test(test):
    def wrapper(*args, **kwargs):
        original_loop = asyncio.get_event_loop()
        test_loop = asyncio.new_event_loop()
        asyncio.set_event_loop(test_loop)
        try:
            test_loop.run_until_complete(test(*args, **kwargs))
        finally:
            test_loop.close()
            asyncio.set_event_loop(original_loop)
    return wrapper


class TestGeolocation(unittest.TestCase):

    def test_pinterface_qi(self):
        locator = wdg.Geolocator._default_ctor()
        op = locator.GetGeopositionAsync()
        self.assertEqual(type(op), wf.IAsyncOperation)
        op.Cancel()

    def test_struct_ctor(self):
        basic_pos = wdg.BasicGeoposition(latitude = 47.1, longitude = -122.1, altitude = 0.0)
        self.assertEqual(basic_pos.latitude, 47.1)
        self.assertEqual(basic_pos.longitude, -122.1)
        self.assertEqual(basic_pos.altitude, 0.0)

        geocircle = wdg.Geocircle.Create(basic_pos, 10)
        center = geocircle.get_Center()

        self.assertEqual(10, geocircle.get_Radius())
        for x in ["latitude", "longitude", "altitude"]:
            self.assertEqual(getattr(basic_pos, x), getattr(center, x))

    def test_struct_from_dict(self):
        basic_pos = {"latitude": 47.1, "longitude": -122.1, "altitude": 0.0}

        geocircle = wdg.Geocircle.Create(basic_pos, 10)
        center = geocircle.get_Center()

        self.assertEqual(10, geocircle.get_Radius())
        for x in ["latitude", "longitude", "altitude"]:
            self.assertEqual(basic_pos[x], getattr(center, x))

    def test_iiterable_wraping(self):
        basic_pos1 = wdg.BasicGeoposition(47.1, -122.1, 0.0)
        basic_pos2 = wdg.BasicGeoposition(47.2, -122.2, 0.0)

        box = wdg.GeoboundingBox.TryCompute([basic_pos1, basic_pos2])
        nw = box.get_NorthwestCorner()
        se = box.get_SoutheastCorner()

        self.assertAlmostEqual(nw.latitude, basic_pos2.latitude)
        self.assertAlmostEqual(nw.longitude, basic_pos2.longitude)
        self.assertAlmostEqual(se.latitude, basic_pos1.latitude)
        self.assertAlmostEqual(se.longitude, basic_pos1.longitude)

    def test_GetGeopositionAsync(self):
        """test async method using IAsyncOperation Completed callback"""
        import threading

        complete_event = threading.Event()

        def callback(operation, status):
            self.assertEqual(status, 1)
            pos = operation.GetResults()

            self.assertEqual(type(pos), wdg.Geoposition)

            coord = pos.get_Coordinate()
            self.assertEqual(type(coord.get_Timestamp().universal_time), int)

            basic_pos = coord.get_Point().get_Position()
            lat = basic_pos.latitude
            self.assertEqual(type(lat), float)

            complete_event.set()

        locator = wdg.Geolocator._default_ctor()
        op = locator.GetGeopositionAsync()
        op.put_Completed(callback)

        self.assertTrue(complete_event.wait(5))

if __name__ == '__main__':
    unittest.main()
