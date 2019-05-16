namespace __Interop__
{
    using System;
    using System.Runtime.InteropServices;

    static class Helper
    {
        public unsafe static T GetDelegate<T>(void* @this, int offset)
        {
            void* __slot = (*(void***)@this)[offset];
            return System.Runtime.InteropServices.Marshal.GetDelegateForFunctionPointer<T>(new System.IntPtr(__slot));
        }
    }

    [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
    internal static extern unsafe int WindowsCreateString(
        [MarshalAs(UnmanagedType.LPWStr)] string sourceString, int length, [Out] IntPtr* hstring);

    [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
    internal static extern int WindowsDeleteString(IntPtr hstring);

    [DllImport("api-ms-win-core-winrt-l1-1-0.dll", PreserveSig = true)]
    internal static extern unsafe int RoGetActivationFactory(IntPtr activatableClassId, [In] ref Guid iid,
        [Out] out IntPtr* factory);
}

namespace __Interop__.Windows.Foundation
{
#pragma warning disable 0649
    using System;
    using System.Runtime.InteropServices;

    internal static class HString
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

    internal static class IUnknown
    {
        unsafe delegate int delegateQueryInterface(IntPtr @this, [In] ref Guid iid, IntPtr* @object);
        public static IntPtr QueryInterface(IntPtr @this, Guid iid)
        {
            var __delegate = GetDelegate<delegateQueryInterface>(@this, 0);

            IntPtr instance = IntPtr.Zero;
            unsafe
            {
                Marshal.GetExceptionForHR(__delegate(@this, ref iid, &instance));
            }
            return instance;
        }

        delegate uint delegateAddRefRelease(IntPtr @this);
        public static uint AddRef(IntPtr @this)
        {
            var __delegate = GetDelegate<delegateAddRefRelease>(@this, 1);
            return __delegate(@this);
        }
        public static uint Release(IntPtr @this)
        {
            var __delegate = GetDelegate<delegateAddRefRelease>(@this, 2);
            return __delegate(@this);
        }
    }

    internal unsafe static class IActivationFactory
    {
        public static readonly Guid IID = new Guid("00000035-0000-0000-C000-000000000046");

        unsafe delegate int delegateActivateInstance([In] IntPtr @this, [Out] IntPtr* instance);
        public static IntPtr ActivateInstance(IntPtr @this)
        {
            var __delegate = GetDelegate<delegateActivateInstance>(@this, 6);

            IntPtr instance = IntPtr.Zero;
            unsafe
            {
                Marshal.GetExceptionForHR(__delegate(@this, &instance));
            }
            return instance;
        }
    }
#pragma warning restore 0649
}