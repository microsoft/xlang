namespace __Interop__.Windows.Foundation
{
    using System;
    using System.Runtime.InteropServices;

    internal enum TrustLevel
    {
        BaseTrust = 0,
        PartialTrust = (BaseTrust + 1),
        FullTrust = (PartialTrust + 1)
    };

    internal struct IInspectableVftbl
    {
        internal delegate int _GetIids([In] IntPtr pThis, [Out] uint iidCount, [Out] Guid[] iids);
        internal delegate int _GetRuntimeClassName([In] IntPtr pThis, [Out] IntPtr className);
        internal delegate int _GetTrustLevel([In] IntPtr pThis, [Out] TrustLevel trustLevel);

        internal IUnknownVftbl IUnknownVftbl;
        internal _GetIids GetIids;
        internal _GetRuntimeClassName GetRuntimeClassName;
        internal _GetTrustLevel GetTrustLevel;
    }
}