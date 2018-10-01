import find_projection

import _pyrt
import unittest
import asyncio

class TestXlangGeolocation(unittest.TestCase):

    def test_pinterface_qi(self):
        locator = _pyrt.Geolocator()
        op = locator.GetGeopositionAsync()
        self.assertEqual(type(op), _pyrt.IAsyncOperation)
        info = _pyrt.IAsyncInfo(op)
        self.assertEqual(type(info), _pyrt.IAsyncInfo)
        info.Cancel()

    def test_geocircle(self):
        basic_pos = {"Latitude":47.1, "Longitude":-122.1, "Altitude": 0.0}
        geocircle = _pyrt.Geocircle(basic_pos, 10)
        self.assertEqual(10, geocircle.Radius)
        center = geocircle.Center

        for x in ["Latitude", "Longitude", "Altitude"]:
            self.assertEqual(basic_pos[x], center[x])


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
            op.Completed = callback

            pos = await future
            self.assertEqual(type(pos), _pyrt.Geoposition)

            coord = pos.Coordinate
            ts = coord.Timestamp
            self.assertEqual(type(ts), int)

            basic_pos = coord.Point.Position
            lat = basic_pos['Latitude']
            self.assertEqual(type(lat), float)
            print(basic_pos['Latitude'], basic_pos['Longitude'])

        loop.run_until_complete(async_test())

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
