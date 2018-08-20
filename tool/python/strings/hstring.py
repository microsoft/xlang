import ctypes as _ct

_core_winrt_str_library = _ct.windll.LoadLibrary("api-ms-win-core-winrt-string-l1-1-0.dll")

_WindowsCreateString = _core_winrt_str_library.WindowsCreateString
_WindowsCreateString.restype = _ct.HRESULT
_WindowsCreateString.argtypes = [_ct.c_wchar_p, _ct.c_long, _ct.POINTER(_ct.c_void_p)]

_WindowsGetStringRawBuffer = _core_winrt_str_library.WindowsGetStringRawBuffer
_WindowsGetStringRawBuffer.restype = _ct.c_wchar_p
_WindowsGetStringRawBuffer.argtypes = [_ct.c_void_p, _ct.POINTER(_ct.c_ulong)]

_WindowsDeleteString = _core_winrt_str_library.WindowsDeleteString
_WindowsDeleteString.restype = _ct.HRESULT
_WindowsDeleteString.argtypes = [_ct.c_void_p]

class HSTRING(_ct.c_void_p):
    def __init__(self, text = None):
        if text is not None:
            hr = _WindowsCreateString(_ct.c_wchar_p(text), len(text), _ct.byref(self))
            _ct._check_HRESULT(hr)

    def __del__(self):
        hr = _core_winrt_str_library.WindowsDeleteString(self)
        _ct._check_HRESULT(hr)

    def __str__(self):
        strlen = _ct.c_ulong()
        buf = _core_winrt_str_library.WindowsGetStringRawBuffer(self, _ct.byref(strlen))
        return buf[:strlen.value]
