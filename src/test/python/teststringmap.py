import find_projection

import _pyrt
import unittest
import asyncio

class TestStringMap(unittest.TestCase):

    def test_stringmap(self):
        m = _pyrt.StringMap()
        m.Insert("hello", "world")

        self.assertTrue(m.HasKey("hello"))
        self.assertFalse(m.HasKey("world"))
        self.assertEqual(m.Size, 1)
        self.assertEqual(m.Lookup("hello"), "world")


    def test_stringmap_changed_event(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(None)

        async def event_test():
            future = loop.create_future()

            def onMapChanged(sender, args): 
                self.assertEqual(args.CollectionChange, 1)
                self.assertEqual(args.Key, "dr")
                future.set_result(None)

            m = _pyrt.StringMap()
            m.Insert("hello", "world")

            token = m.add_MapChanged(onMapChanged)
            m.Insert("dr", "who")
        
            await future
        
        loop.run_until_complete(event_test())
        loop.close()

if __name__ == '__main__':
    _pyrt.init_apartment()
    unittest.main()
    _pyrt.uninit_apartment()