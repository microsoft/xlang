import ctypes as _ct
import os as _os

class GUID(_ct.Structure):
    _fields_ = [("Data1", _ct.c_ulong),
                ("Data2", _ct.c_ushort),
                ("Data3", _ct.c_ushort),
                ("Data4", _ct.c_ubyte * 8)]

if _os.name == "nt":

    _CoCreateGuid = _ct.oledll.ole32.CoCreateGuid
    _StringFromCLSID = _ct.oledll.ole32.StringFromCLSID
    _CoTaskMemFree = _ct.windll.ole32.CoTaskMemFree
    _CLSIDFromString = _ct.oledll.ole32.CLSIDFromString

    def co_create_guid():
        g = GUID()
        _CoCreateGuid(_ct.byref(g))
        return g

    def string_from_clsid(g):
        p = _ct.c_wchar_p()
        try:
            _StringFromCLSID(_ct.byref(g), _ct.byref(p))
            return p.value
        finally:
            _CoTaskMemFree(p)

    def clsid_from_string(name):
        g = GUID()
        _CLSIDFromString(_ct.c_wchar_p(name), _ct.byref(g))
        return g
