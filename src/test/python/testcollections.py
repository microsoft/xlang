import find_projection
import unittest
import asyncio

import pyrt.windows.foundation.collections as wfc

class TestCollections(unittest.TestCase):

    def test_stringmap(self):
        m = wfc.StringMap()
        m.Insert("hello", "world")

        self.assertTrue(m.HasKey("hello"))
        self.assertFalse(m.HasKey("world"))
        self.assertEqual(m.get_Size(), 1)
        self.assertEqual(m.Lookup("hello"), "world")


    def test_stringmap_changed_event(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(None)
        
        async def async_test():
            future = loop.create_future()

            def onMapChanged(sender, args): 
                self.assertEqual(args.get_CollectionChange(), 1)
                self.assertEqual(args.get_Key(), "dr")

                self.assertEqual(sender.get_Size(), 2)
                self.assertTrue(sender.HasKey("dr"))
                self.assertTrue(sender.HasKey("hello"))
                
                loop.call_soon_threadsafe(asyncio.Future.set_result, future, True)

            m = wfc.StringMap()
            m.Insert("hello", "world")
            token = m.add_MapChanged(onMapChanged)
            m.Insert("dr", "who")
            m.remove_MapChanged(token)

            called = await future
            self.assertTrue(called)

        loop.run_until_complete(async_test())
        loop.close()


if __name__ == '__main__':
    unittest.main()
