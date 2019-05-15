namespace __Interop__.Windows.Foundation
{
    using System;
    using System.Runtime.InteropServices;

    internal struct IActivationFactoryVftbl
    {
        internal delegate int _ActivateInstance([In] IntPtr pThis, [Out] IntPtr instance);

        internal IInspectableVftbl IInspectableVftbl;
        internal _ActivateInstance ActivateInstance;
    }
}