string Windows.Foundation.IStringable.ToString()
{
    %
    System.IntPtr _IStringablePtr = System.IntPtr.Zero;
    System.IntPtr _returnValue = System.IntPtr.Zero;

    try
    {
        _IStringablePtr = Windows.Foundation.IUnknown.QueryInterface(_instance, Windows.Foundation._IStringableInterop.IID);
        _returnValue = Windows.Foundation._IStringableInterop.ToString6(_IStringablePtr);
        return Windows.HString.Interop.ToString(_returnValue);
    }
    finally
    {
        Windows.HString.Interop.Delete(_returnValue);
        Windows.Foundation.IUnknown.Release(_IStringablePtr);
    }
}
