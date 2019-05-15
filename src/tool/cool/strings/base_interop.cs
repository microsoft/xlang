namespace __Interop__
{
    static class Helper
    {
        public unsafe static T GetDelegate<T>(void* @this, int offset)
        {
            void* __slot = (*(void***)@this)[offset];
            return System.Runtime.InteropServices.Marshal.GetDelegateForFunctionPointer<T>(new System.IntPtr(__slot));
        }
    }
}

namespace __Interop__.Windows.Foundation
{
#pragma warning disable 0649
    using System;
    using System.Runtime.InteropServices;

    internal unsafe static class IUnknown
    {
        delegate int delegateQueryInterface(void* @this, IntPtr iid, void** @object);
        public static int invokeQueryInterface(void* @this, IntPtr iid, void** @object)
        {
            var __delegate = __Interop__.Helper.GetDelegate<delegateQueryInterface>(@this, 0);
            return __delegate(@this, iid, @object);
        }

        delegate uint delegateAddRefRelease(void* @this);
        public static uint invokeAddRef(void* @this, IntPtr iid, void** vftbl)
        {
            var __delegate = __Interop__.Helper.GetDelegate<delegateAddRefRelease>(@this, 1);
            return __delegate(@this);
        }
        public static uint invokeRelease(void* @this)
        {
            var __delegate = __Interop__.Helper.GetDelegate<delegateAddRefRelease>(@this, 2);
            return __delegate(@this);
        }
    }

    internal unsafe static class IActivationFactory
    {
        delegate int delegateActivateInstance(void* @this, void** instance);
        public static int invokeActivateInstance(void* @this, void** instance)
        {
            var __delegate = __Interop__.Helper.GetDelegate<delegateActivateInstance>(@this, 6);
            return __delegate(@this, instance);
        }
    }
#pragma warning restore 0649
}