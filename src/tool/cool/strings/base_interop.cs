namespace Windows
{
    using System;
    using System.Runtime.InteropServices;

    public sealed class HString : IDisposable
    {
        public static class Interop
        {
            [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
            static extern unsafe int WindowsCreateString([MarshalAs(UnmanagedType.LPWStr)] string source, int length, [Out] IntPtr* hstring);

            [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
            static extern int WindowsDeleteString(IntPtr hstring);

            [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
            static extern unsafe char* WindowsGetStringRawBuffer(IntPtr hstring, [Out] uint* length);

            public static IntPtr Create(string source)
            {
                if (string.IsNullOrEmpty(source))
                {
                    return IntPtr.Zero;
                }

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

        private IntPtr _handle;
        private bool _disposed = false;

        public HString(IntPtr handle)
        {
            _handle = handle;
        }

        public HString() : this(null)
        {
        }

        public HString(string value)
        {
            _handle = Interop.Create(value);
        }

        public IntPtr Handle => _disposed ? throw new ObjectDisposedException("Windows.HString") : _handle;

        public override string ToString()
        {
            if (_disposed) throw new ObjectDisposedException("Windows.HString");
            return Interop.ToString(_handle);
        }

        void _Dispose()
        {
            if (!_disposed)
            {
                Interop.Delete(_handle);
                _handle = IntPtr.Zero;
                _disposed = true;
            }
        }

        ~HString()
        {
            _Dispose();
        }

        public void Dispose()
        {
            _Dispose();
            GC.SuppressFinalize(this);
        }
    }

    public sealed class ComPtr : IDisposable
    {
        private IntPtr _value;
        private bool _disposed = false;

        public ComPtr(IntPtr value)
        {
            _value = value;
        }

        public IntPtr Value => _disposed ? throw new ObjectDisposedException("Windows.ComPtr") : _value;

        public ComPtr As(Guid iid)
        {
            if (_disposed) throw new ObjectDisposedException("Windows.ComPtr");
            return Windows.Foundation.IUnknown.As(_value, iid);
        }

        void _Dispose()
        {
            if (!_disposed)
            {
                Windows.Foundation.IUnknown.Release(_value);
                _value = IntPtr.Zero;

                _disposed = true;
            }
        }

        ~ComPtr()
        {
            _Dispose();
        }

        public void Dispose()
        {
            _Dispose();
            System.GC.SuppressFinalize(this);
        }
    }

    namespace Foundation
    {
        //#pragma warning disable 0649
        public static class IUnknown
        {
            public static readonly Guid IID = new Guid("00000000-0000-0000-C000-000000000046");

            unsafe delegate int delegateQueryInterface(IntPtr @this, [In] ref Guid iid, IntPtr* @object);

            unsafe public static int invokeQueryInterface(IntPtr @this, ref Guid iid, IntPtr* @object)
            {
                void* __slot = (*(void***)@this.ToPointer())[0];
                var __delegate = Marshal.GetDelegateForFunctionPointer<delegateQueryInterface>(new IntPtr(__slot));
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

            public static ComPtr As(IntPtr @this, Guid iid)
            {
                return new ComPtr(QueryInterface(@this, iid));
            }

            delegate uint delegateAddRef(IntPtr @this);

            unsafe public static uint invokeAddRef(IntPtr @this)
            {
                void* __slot = (*(void***)@this.ToPointer())[1];
                var __delegate = Marshal.GetDelegateForFunctionPointer<delegateAddRef>(new IntPtr(__slot));
                return __delegate(@this);
            }

            public static uint AddRef(IntPtr @this)
            {
                return invokeAddRef(@this);
            }

            delegate uint delegateRelease(IntPtr @this);

            unsafe public static uint invokeRelease(IntPtr @this)
            {
                void* __slot = (*(void***)@this.ToPointer())[2];
                var __delegate = Marshal.GetDelegateForFunctionPointer<delegateRelease>(new IntPtr(__slot));
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
                void* __slot = (*(void***)@this.ToPointer())[6];
                var __delegate = Marshal.GetDelegateForFunctionPointer<delegateActivateInstance>(new IntPtr(__slot));
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

            [DllImport("api-ms-win-core-winrt-l1-1-0.dll", PreserveSig = true)]
            static extern unsafe int RoGetActivationFactory([In] IntPtr activatableClassId, [In] ref Guid iid, [Out] IntPtr* factory);

            public static IntPtr Get(string className)
            {
                return Get(className, IActivationFactory.IID);
            }

            public static IntPtr Get(string className, Guid iid)
            {
                using (var classNameHstring = new HString(className))
                {
                    IntPtr factory = IntPtr.Zero;
                    unsafe
                    {
                        Marshal.ThrowExceptionForHR(RoGetActivationFactory(classNameHstring.Handle, ref iid, &factory));
                    }
                    return factory;
                }
            }
        }
    }
//#pragma warning restore 0649
}