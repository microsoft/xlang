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

    public class Class : WinRT.RuntimeClass<Class, Interop.IClassVftbl>
    {
        WinRT.EventSource<Class, object> _myPropertyChanged;

        public Class() :
            base(_factory.Value.ActivateInstance<Interop.IClassVftbl>())
        {
            _myPropertyChanged = new WinRT.EventSource<Class, object>(this, _obj.As<WinRT.Interop.IInspectableVftbl>(), _obj.Vftbl.add_MyPropertyChanged, _obj.Vftbl.remove_MyPropertyChanged, (IntPtr) => null);
        }

        public int MyProperty
        {
            get
            {
                int value = 0;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_MyProperty(_obj.ThisPtr, &value)); }
                return value;
            }
            set
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.put_MyProperty(_obj.ThisPtr, value));
            }
        }

        public event WinRT.TypedEventHandler<Class, object> MyPropertyChanged
        {
            add { _myPropertyChanged.Event += value; }
            remove { _myPropertyChanged.Event -= value; }
        }
    }
}
