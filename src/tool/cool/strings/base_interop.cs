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

    internal struct IActivationFactoryVftbl
    {
        internal delegate int _ActivateInstance([In] IntPtr pThis, [Out] IntPtr instance);

        internal IInspectableVftbl IInspectableVftbl;
        internal _ActivateInstance ActivateInstance;
    }
}