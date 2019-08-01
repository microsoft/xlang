private bool _disposed = false; 
void _Dispose()
{
    if (!_disposed)
    {
        // TODO: call IClosable.Close() if appropriate 
        Windows.Foundation.IUnknown.Release(_instance);
        _instance = System.IntPtr.Zero;

        _disposed = true;
    }
}
~%()
{
    _Dispose();
}
public void Dispose()
{
    _Dispose();
    System.GC.SuppressFinalize(this);
}
