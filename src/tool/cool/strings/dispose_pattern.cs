private bool _disposedValue = false; 
protected virtual void _Dispose()
{
    if (!_disposedValue)
    {
        // TODO: call IClosable.Close() if appropriate 
        Windows.Foundation.IUnknown.Release(_instance);
        _instance = System.IntPtr.Zero;

        _disposedValue = true;
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
