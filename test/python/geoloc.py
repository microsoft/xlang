import find_xlang
import _xlang
import asyncio

async def wrap_async_op(op):

    loop = asyncio.get_event_loop()
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

    op.Completed = callback

    return await future

async def runGeoloc():
    loc = _xlang.Geolocator()
    pos = await wrap_async_op(loc.GetGeopositionAsync())
    coord = pos.Coordinate
    print("Accuracy: {0}".format(coord.Accuracy))
    print("Latitude: {0}".format(coord.Latitude))
    print("Longitude: {0}".format(coord.Longitude))
    print("PositionSource: {0}".format(coord.PositionSource))
    #print("Timestamp: {0}".format(coord.Timestamp))

_xlang.init_apartment()
loop = asyncio.get_event_loop()
loop.run_until_complete(runGeoloc())
loop.close()