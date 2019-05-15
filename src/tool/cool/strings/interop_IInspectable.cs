namespace __Interop__.Windows.Foundation
{
    internal enum TrustLevel
    {
        BaseTrust = 0,
        PartialTrust = (BaseTrust + 1),
        FullTrust = (PartialTrust + 1)
    };

    internal struct IInspectableVftbl
    {
        internal delegate int _GetIids([System.Runtime.InteropServices.In] System.IntPtr pThis, [System.Runtime.InteropServices.Out] uint iidCount, [System.Runtime.InteropServices.Out] System.Guid[] iids);
        internal delegate int _GetRuntimeClassName([System.Runtime.InteropServices.In] System.IntPtr pThis, [System.Runtime.InteropServices.Out] System.IntPtr className);
        internal delegate int _GetTrustLevel([System.Runtime.InteropServices.In] System.IntPtr pThis, [System.Runtime.InteropServices.Out] TrustLevel trustLevel);

        internal IUnknownVftbl IUnknownVftbl;
        internal _GetIids GetIids;
        internal _GetRuntimeClassName GetRuntimeClassName;
        internal _GetTrustLevel GetTrustLevel;
    }
}