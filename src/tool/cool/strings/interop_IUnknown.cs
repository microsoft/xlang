namespace __Interop__.Windows.Foundation
{
    internal struct IUnknownVftbl
    {
        internal delegate int _QueryInterface([System.Runtime.InteropServices.In] System.IntPtr pThis, [System.Runtime.InteropServices.In] System.Guid iid, [System.Runtime.InteropServices.Out] System.IntPtr vftbl);
        internal delegate uint _AddRef([System.Runtime.InteropServices.In] System.IntPtr pThis);
        internal delegate uint _Release([System.Runtime.InteropServices.In] System.IntPtr pThis);

        internal _QueryInterface QueryInterface;
        internal _AddRef AddRef;
        internal _Release Release;
    }
}