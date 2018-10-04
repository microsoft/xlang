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

    def test_GetGeopositionAsync(self):
        locator = _pyrt.Geolocator()
        
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def callback(operation, status):
            self.assertEqual(status, 1)
            result = operation.GetResults()
            loop.call_soon_threadsafe(asyncio.Future.set_result, future, result)

        op = locator.GetGeopositionAsync()
        op.Completed = callback

        loop = asyncio.get_event_loop()
        loop.run_until_complete(future)
        loop.close()

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()
