import find_projection

import _pyrt
import unittest
import asyncio

class TestGeolocation(unittest.TestCase):

    def test_pinterface_qi(self):
        locator = _pyrt.Geolocator()
        op = locator.GetGeopositionAsync()
        self.assertEqual(type(op), _pyrt.IAsyncOperation)
        op.Cancel()

    def test_struct_ctor(self):
        basic_pos = _pyrt.BasicGeoposition(Latitude = 47.1, Longitude = -122.1, Altitude = 0.0)
        self.assertEqual(basic_pos.Latitude, 47.1)
        self.assertEqual(basic_pos.Longitude, -122.1)
        self.assertEqual(basic_pos.Altitude, 0.0)

        geocircle = _pyrt.Geocircle(basic_pos, 10)
        center = geocircle.get_Center()

        self.assertEqual(10, geocircle.get_Radius())
        for x in ["Latitude", "Longitude", "Altitude"]:
            self.assertEqual(getattr(basic_pos, x), getattr(center, x))

    def test_struct_from_dict(self):
        basic_pos = {"Latitude": 47.1, "Longitude": -122.1, "Altitude": 0.0}

        geocircle = _pyrt.Geocircle(basic_pos, 10)
        center = geocircle.get_Center()

        self.assertEqual(10, geocircle.get_Radius())
        for x in ["Latitude", "Longitude", "Altitude"]:
            self.assertEqual(basic_pos[x], getattr(center, x))

    def test_GetGeopositionAsync(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(None)

        async def async_test():
            future = loop.create_future()
            
            def callback(operation, status):
                if status == 1:
                    result = operation.GetResults()
                    loop.call_soon_threadsafe(asyncio.Future.set_result, future, result)
                elif status == 2:
                    loop.call_soon_threadsafe(asyncio.Future.set_exception, future, asyncio.CancelledError())
                elif status == 3:
                    loop.call_soon_threadsafe(asyncio.Future.set_exception, future, RuntimeError("AsyncOp failed"))
                else:
                    loop.call_soon_threadsafe(asyncio.Future.set_exception, future, RuntimeError("Unexpected AsyncStatus"))
            
            locator = _pyrt.Geolocator()
            op = locator.GetGeopositionAsync()
            op.put_Completed(callback)

            pos = await future
            self.assertEqual(type(pos), _pyrt.Geoposition)

            coord = pos.get_Coordinate()
            self.assertEqual(type(coord.get_Timestamp().UniversalTime), int)

            basic_pos = coord.get_Point().get_Position()
            lat = basic_pos.Latitude
            self.assertEqual(type(lat), float)

        loop.run_until_complete(async_test())
        loop.close()

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
