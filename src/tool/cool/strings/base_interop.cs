namespace __Interop__
{
    using System;
    using System.Runtime.InteropServices;

    static class Helper
    {
        public unsafe static T GetDelegate<T>(IntPtr @this, int offset)
        {
            void* __slot = (*(void***)@this.ToPointer())[offset];
            return Marshal.GetDelegateForFunctionPointer<T>(new IntPtr(__slot));
        }
    }
}

namespace __Interop__.Windows.Foundation
{
#pragma warning disable 0649
    using System;
    using System.Runtime.InteropServices;

    public static class HString
    {
        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        static extern unsafe int WindowsCreateString([MarshalAs(UnmanagedType.LPWStr)] string source, int length, [Out] IntPtr* hstring);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        static extern int WindowsDeleteString(IntPtr hstring);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        static extern unsafe char* WindowsGetStringRawBuffer(IntPtr hstring, [Out] uint* length);

        public static IntPtr Create(string source)
        {
            IntPtr hstring;
            unsafe
            {
                Marshal.ThrowExceptionForHR(WindowsCreateString(source, source.Length, &hstring));
            }
            return hstring;
        }

        public static void Delete(IntPtr hstring)
        {
            Marshal.ThrowExceptionForHR(WindowsDeleteString(hstring));
        }

        public static string ToString(IntPtr hstring)
        {
            if (hstring == IntPtr.Zero)
            {
                return string.Empty;
            }

            unsafe
            {
                uint length;
                var buffer = WindowsGetStringRawBuffer(hstring, &length);
                return new string(buffer, 0, checked((int)length));
            }
        }
    }

    public static class Platform
    {
        [DllImport("api-ms-win-core-winrt-l1-1-0.dll", PreserveSig = true)]
        static extern unsafe int RoGetActivationFactory([In] IntPtr activatableClassId, [In] ref Guid iid, [Out] IntPtr* factory);

        public static IntPtr GetActivationFactory(string className, Guid iid)
        {
            var classNameHstring = HString.Create(className);
            try
            {
                IntPtr factory = IntPtr.Zero;
                unsafe
                {
                    Marshal.ThrowExceptionForHR(RoGetActivationFactory(classNameHstring, ref iid, &factory));
                }
                return factory;
            }
            finally
            {
                HString.Delete(classNameHstring);
            }
        }
    }

    public static class IUnknown
    {
        public static readonly Guid IID = new Guid("00000000-0000-0000-C000-000000000046");

        unsafe delegate int delegateQueryInterface(IntPtr @this, [In] ref Guid iid, IntPtr* @object);

        unsafe public static int invokeQueryInterface(IntPtr @this, ref Guid iid, IntPtr* @object)
        {
            var __delegate = Helper.GetDelegate<delegateQueryInterface>(@this, 0);
            return __delegate(@this, ref iid, @object);
        }

        public static IntPtr QueryInterface(IntPtr @this, Guid iid)
        {
            IntPtr instance = IntPtr.Zero;
            unsafe
            {
                Marshal.GetExceptionForHR(invokeQueryInterface(@this, ref iid, &instance));
            }
            return instance;
        }

        delegate uint delegateAddRef(IntPtr @this);

        public static uint invokeAddRef(IntPtr @this)
        {
            var __delegate = Helper.GetDelegate<delegateAddRef>(@this, 1);
            return __delegate(@this);
        }

        public static uint AddRef(IntPtr @this)
        {
            return invokeAddRef(@this);
        }

        delegate uint delegateRelease(IntPtr @this);

        public static uint invokeRelease(IntPtr @this)
        {
            var __delegate = Helper.GetDelegate<delegateRelease>(@this, 2);
            return __delegate(@this);
        }

        public static uint Release(IntPtr @this)
        {
            return invokeRelease(@this);
        }
    }

    public static class IActivationFactory
    {
        public static readonly Guid IID = new Guid("00000035-0000-0000-C000-000000000046");

        unsafe delegate int delegateActivateInstance([In] IntPtr @this, [Out] IntPtr* instance);

        unsafe public static int invokeActivateInstance(IntPtr @this, IntPtr* instance)
        {
            var __delegate = Helper.GetDelegate<delegateActivateInstance>(@this, 6);
            return __delegate(@this, instance);
        }

        public static IntPtr ActivateInstance(IntPtr @this)
        {
            IntPtr instance = IntPtr.Zero;
            unsafe
            {
                Marshal.GetExceptionForHR(invokeActivateInstance(@this, &instance));
            }
            return instance;
        }
    }
#pragma warning restore 0649
}