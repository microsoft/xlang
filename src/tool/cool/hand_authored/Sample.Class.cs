using System;
using System.Runtime.InteropServices;

namespace Sample
{
    namespace Interop
    {
        // IClass
        [Guid("C50B7217-25B5-5657-9DB8-A23874617520")]
        public struct IClassVftbl
        {
            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public WinRT.Interop._get_PropertyAsInt get_MyProperty;
            public WinRT.Interop._put_PropertyAsInt put_MyProperty;
            public WinRT.Interop._add_EventHandler add_MyPropertyChanged;
            public WinRT.Interop._remove_EventHandler remove_MyPropertyChanged;
        }
    }

    public class Class
    {
        static Lazy<WinRT.ActivationFactory<Class>> _factory = new Lazy<WinRT.ActivationFactory<Class>>();
        WinRT.ObjectReference<Interop.IClassVftbl> _IClass;
        WinRT.EventSource<Class, object> _myPropertyChanged;

        public Class()
        {
            _IClass = _factory.Value.ActivateInstance<Interop.IClassVftbl>();
            _myPropertyChanged = new WinRT.EventSource<Class, object>(this, _IClass.ThisPtr, _IClass.Vftbl.add_MyPropertyChanged, _IClass.Vftbl.remove_MyPropertyChanged, (IntPtr) => null);
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

        public event WinRT.TypedEventHandler<Class, object> MyPropertyChanged
        {
            add { _myPropertyChanged.Event += value; }
            remove { _myPropertyChanged.Event -= value; }
        }
    }
}
