using System;
using System.Runtime.InteropServices;

namespace Sample
{
    namespace Interop
    {
        // IClass
        [Guid("F1702855-48DE-543A-BDB8-E179B86F3070")]
        public struct IClassVftbl
        {
            public unsafe delegate int _get_MyProperty([In] IntPtr thisPtr, [Out] int* value);
            public delegate int _put_MyProperty([In] IntPtr thisPtr, [In] int value);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _get_MyProperty get_MyProperty;
            public _put_MyProperty put_MyProperty;
        }
    }

    public class Class : ICloneable, IDisposable
    {
        static Lazy<WinRT.ActivationFactory<Class>> _factory = new Lazy<WinRT.ActivationFactory<Class>>();
        WinRT.ObjectReference<Interop.IClassVftbl> _IClass;

        Class(WinRT.ObjectReference<Interop.IClassVftbl> that)
        {
            _IClass = (WinRT.ObjectReference<Interop.IClassVftbl>)that.Clone();
        }

        public Class()
        {
            _IClass = _factory.Value.ActivateInstance<Interop.IClassVftbl>();
        }

        public int MyProperty
        {
            get
            {
                int value = 0;
                unsafe { Marshal.ThrowExceptionForHR(_IClass.Vftbl.get_MyProperty(_IClass.ThisPtr, &value)); }
                return value;
            }
            set
            {
                Marshal.ThrowExceptionForHR(_IClass.Vftbl.put_MyProperty(_IClass.ThisPtr, value));
            }
        }

        public object Clone()
        {
            return new Class(_IClass);
        }

        public void Dispose()
        {
            _IClass.Dispose();
        }
    }
}
