namespace __Interop__.Windows.Foundation
{
    internal struct IActivationFactoryVftbl
    {
        internal delegate int _ActivateInstance([System.Runtime.InteropServices.In] System.IntPtr pThis, [System.Runtime.InteropServices.Out] System.IntPtr instance);

        internal IInspectableVftbl IInspectableVftbl;
        internal _ActivateInstance ActivateInstance;
    }
}