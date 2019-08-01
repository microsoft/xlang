using System;
using System.Runtime.InteropServices;

namespace TestComp
{
    namespace Interop
    {
        // IClass
        [Guid("89418FCA-D2E1-52EC-AB4C-AE611B4C823A")]
        public struct IClassVftbl
        {
            #pragma warning disable 0169
            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            #pragma warning enable 0169
            public WinRT.Interop._get_PropertyAsInt get_MyProperty;
            public WinRT.Interop._put_PropertyAsInt put_MyProperty;
            public WinRT.Interop._add_EventHandler add_MyPropertyChanged;
            public WinRT.Interop._remove_EventHandler remove_MyPropertyChanged;
            public WinRT.Interop._get_PropertyAsObject get_StringProperty;
            public WinRT.Interop._put_PropertyAsObject put_StringProperty;
            public WinRT.Interop._get_PropertyAsObject get_StringsProperty;
            public WinRT.Interop._put_PropertyAsObject put_StringsProperty;
        }
    }

    public class Class : WinRT.RuntimeClass<Class, Interop.IClassVftbl>
    {
        WinRT.EventSource<Class, UInt32> _myPropertyChanged;

        public Class() :
            base(_factory.Value.ActivateInstance<Interop.IClassVftbl>())
        {
            _myPropertyChanged = 
                new WinRT.EventSource<Class, UInt32>(this, 
                _obj.As<WinRT.Interop.IInspectableVftbl>(), 
                _obj.Vftbl.add_MyPropertyChanged, 
                _obj.Vftbl.remove_MyPropertyChanged, 
                (IntPtr) => 42);    // todo: unbox 
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

        public event WinRT.TypedEventHandler<Class, UInt32> MyPropertyChanged
        {
            add { _myPropertyChanged.Event += value; }
            remove { _myPropertyChanged.Event -= value; }
        }

        //internal struct HSTRING_HEADER
        //{
        //    public IntPtr unused;
        //};

        public unsafe string StringProperty
        {
            get 
            {
                IntPtr handle;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_StringProperty(_obj.ThisPtr, &handle)); }
                uint length;
                char* buffer = WinRT.Platform.WindowsGetStringRawBuffer(handle, &length);
                var value = new string(buffer, 0, (int)length);
                WinRT.Platform.WindowsDeleteString(handle);
                return value;
            }
            set
            {
                //ReadOnlySpan<char> span = value;
                IntPtr header;
                IntPtr handle;
                WinRT.Platform.WindowsCreateStringReference(value, value.Length, &header, &handle);
                Marshal.ThrowExceptionForHR(_obj.Vftbl.put_StringProperty(_obj.ThisPtr, handle));
            }
        }
    }
}
