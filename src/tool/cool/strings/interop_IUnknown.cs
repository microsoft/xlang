namespace __Interop__.Windows.Foundation
{
    using System;
    using System.Runtime.InteropServices;

    internal struct IUnknownVftbl
    {
        internal delegate int _QueryInterface([In] IntPtr pThis, [In] Guid iid, [Out] IntPtr vftbl);
        internal delegate uint _AddRef([In] IntPtr pThis);
        internal delegate uint _Release([In] IntPtr pThis);

        internal _QueryInterface QueryInterface;
        internal _AddRef AddRef;
        internal _Release Release;
    }
}