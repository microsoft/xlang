import ctypes as _ct
from guid import GUID as _GUID
from hstring import HSTRING as _HSTRING

_rtcore = _ct.oledll.LoadLibrary("api-ms-win-core-winrt-l1-1-0.dll")

_RoInitialize = _rtcore.RoInitialize
_RoInitialize.argtypes = [_ct.c_long]

def initialize():
    _RoInitialize(1)

_QueryInterface = _ct.WINFUNCTYPE(_ct.HRESULT, _ct.POINTER(_GUID), _ct.POINTER(_ct.c_void_p))(0, 'QueryInterface')
_AddRef = _ct.WINFUNCTYPE(_ct.c_ulong)(1, 'AddRef')
_Release = _ct.WINFUNCTYPE(_ct.c_ulong)(2, 'Release')

def query_interface(iface, iid):
    new_interface = _ct.c_void_p()
    _QueryInterface(iface, _ct.byref(iid), _ct.byref(new_interface))
    return new_interface

_RoGetActivationFactory = _rtcore.RoGetActivationFactory
_RoGetActivationFactory.argtypes = [_ct.c_void_p, _ct.POINTER(_GUID), _ct.POINTER(_ct.c_void_p)]

def get_activation_factory(typename, iid=_GUID(0x00000035, 0x0000, 0x0000, (0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46))):
    factory = _ct.c_void_p()
    _typename = _HSTRING(typename)
    _RoGetActivationFactory(_typename, _ct.byref(iid), _ct.byref(factory))
    return factory