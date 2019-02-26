import unittest
import asyncio

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

import pyrt.windows.devices.geolocation as wdg
import pyrt.windows.foundation as wf

class TestGeolocation(unittest.TestCase):

    def test_pinterface_qi(self):
        locator = wdg.Geolocator()
        op = locator.GetGeopositionAsync()
        self.assertEqual(type(op), wf.IAsyncOperation)
        op.Cancel()

    def test_struct_ctor(self):
        basic_pos = wdg.BasicGeoposition(Latitude = 47.1, Longitude = -122.1, Altitude = 0.0)
        self.assertEqual(basic_pos.Latitude, 47.1)
        self.assertEqual(basic_pos.Longitude, -122.1)
        self.assertEqual(basic_pos.Altitude, 0.0)

        geocircle = wdg.Geocircle(basic_pos, 10)
        center = geocircle.Center

        self.assertEqual(10, geocircle.Radius)
        for x in ["Latitude", "Longitude", "Altitude"]:
            self.assertEqual(getattr(basic_pos, x), getattr(center, x))

    def test_struct_from_dict(self):
        basic_pos = {"Latitude": 47.1, "Longitude": -122.1, "Altitude": 0.0}

        geocircle = wdg.Geocircle(basic_pos, 10)
        center = geocircle.Center

        self.assertEqual(10, geocircle.Radius)
        for x in ["Latitude", "Longitude", "Altitude"]:
            self.assertEqual(basic_pos[x], getattr(center, x))

    def test_iiterable_wraping(self):
        basic_pos1 = wdg.BasicGeoposition(47.1, -122.1, 0.0)
        basic_pos2 = wdg.BasicGeoposition(47.2, -122.2, 0.0)

        box = wdg.GeoboundingBox.TryCompute([basic_pos1, basic_pos2])
        nw = box.NorthwestCorner
        se = box.SoutheastCorner

        self.assertAlmostEqual(nw.Latitude, basic_pos2.Latitude)
        self.assertAlmostEqual(nw.Longitude, basic_pos2.Longitude)
        self.assertAlmostEqual(se.Latitude, basic_pos1.Latitude)
        self.assertAlmostEqual(se.Longitude, basic_pos1.Longitude)

    def test_GetGeopositionAsync(self):
        """test async method using IAsyncOperation Completed callback"""
        import threading

        complete_event = threading.Event()

        def callback(operation, status):
            self.assertEqual(status, 1)
            pos = operation.GetResults()

            self.assertEqual(type(pos), wdg.Geoposition)

            coord = pos.Coordinate
            self.assertEqual(type(coord.Timestamp.UniversalTime), int)

            basic_pos = coord.Point.Position
            lat = basic_pos.Latitude
            self.assertEqual(type(lat), float)

            complete_event.set()

        locator = wdg.Geolocator()
        op = locator.GetGeopositionAsync()
        op.Completed = callback

        self.assertTrue(complete_event.wait(5))

    @async_test
    async def test_GetGeopositionAsync_await(self):
        """test async method by directly awaiting IAsyncOperation"""

        locator = wdg.Geolocator()
        pos = await locator.GetGeopositionAsync()
        self.assertEqual(type(pos), wdg.Geoposition)

        coord = pos.Coordinate
        self.assertEqual(type(coord.Timestamp.UniversalTime), int)

        basic_pos = coord.Point.Position
        lat = basic_pos.Latitude
        self.assertEqual(type(lat), float)


if __name__ == '__main__':
    import _pyrt
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
