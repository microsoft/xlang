using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using WinRT;

namespace TestComp.Projected
{
    public delegate String StringProvider();

    internal class IClass
    {
        // IClass (IID copied from generated TestComp.0.h)
        [Guid("027A6AB6-74B0-5A15-94F1-95DDAC0DB9E5")]
        public struct Vftbl
        {
            public unsafe delegate int _GetString([In] IntPtr thisPtr);
            public unsafe delegate int _SetString([In] IntPtr thisPtr, IntPtr provider);

#pragma warning disable 0169
            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
#pragma warning enable 0169
            public WinRT.Interop._get_PropertyAsInt get_IntProperty;
            public WinRT.Interop._put_PropertyAsInt put_IntProperty;
            public WinRT.Interop._add_EventHandler add_IntPropertyChanged;
            public WinRT.Interop._remove_EventHandler remove_IntPropertyChanged;
            public WinRT.Interop._get_PropertyAsString get_StringProperty;
            public WinRT.Interop._put_PropertyAsString put_StringProperty;
            public WinRT.Interop._add_EventHandler add_StringPropertyChanged;
            public WinRT.Interop._remove_EventHandler remove_StringPropertyChanged;
            public WinRT.Interop._get_PropertyAsString get_StringProperty2;
            public WinRT.Interop._put_PropertyAsString put_StringProperty2;
            public WinRT.Interop._get_PropertyAsObject get_StringsProperty;
            public WinRT.Interop._put_PropertyAsObject put_StringsProperty;
            public _GetString GetString;
            public _SetString SetString;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;

        public static implicit operator IClass(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IClass(WinRT.ObjectReference<Vftbl> obj) => new IClass(obj);
        public IClass(WinRT.ObjectReference<Vftbl> obj)
        {
            _obj = obj;

            _IntPropertyChanged =
                new WinRT.EventSource<Class, UInt32>(_obj,
                _obj.Vftbl.add_IntPropertyChanged,
                _obj.Vftbl.remove_IntPropertyChanged,
                (IntPtr) => 42);    // todo: unbox 

            _StringPropertyChanged =
                new WinRT.EventSource<Class, WinRT.HString>(_obj,
                _obj.Vftbl.add_StringPropertyChanged,
                _obj.Vftbl.remove_StringPropertyChanged,
                // todo: some way to generically/default define conversion op?
                (IntPtr handle) => new WinRT.HString(handle));
        }

        public int IntProperty
        {
            get
            {
                int value = 0;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_IntProperty(_obj.ThisPtr, &value)); }
                return value;
            }
            set
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.put_IntProperty(_obj.ThisPtr, value));
            }
        }

        public event WinRT.TypedEventHandler<Class, UInt32> IntPropertyChanged
        {
            add { _IntPropertyChanged.Event += value; }
            remove { _IntPropertyChanged.Event -= value; }
        }

        public String StringProperty
        {
            get
            {
                unsafe
                {
                    String value;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_StringProperty(_obj.ThisPtr, out value));
                    return value;
                }
            }
            set
            {
                unsafe
                {
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.put_StringProperty(_obj.ThisPtr, value));
                }
            }
        }

        public event WinRT.TypedEventHandler<Class, WinRT.HString> StringPropertyChanged
        {
            add { _StringPropertyChanged.Event += value; }
            remove { _StringPropertyChanged.Event -= value; }
        }

        public String StringProperty2
        {
            get
            {
                unsafe
                {
                    String value;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_StringProperty2(_obj.ThisPtr, out value));
                    return value;
                }
            }
            set
            {
                unsafe
                {
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.put_StringProperty2(_obj.ThisPtr, value));
                }
            }
        }

        public IEnumerable<String> StringsProperty
        {
            get
            {
                return null;
            }
            set
            {
            }
        }

        public void GetString()
        {
            unsafe
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.GetString(_obj.ThisPtr));
            }
        }

        public void SetString(StringProvider provider)
        {
            unsafe
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.SetString(_obj.ThisPtr, GetSetStringDelegate(provider)));
            }
        }

        unsafe delegate int StringProvider_Invoke(IntPtr thisPtr, [Out] IntPtr* value);
        private WinRT.Delegate _SetStringDelegate;

        private unsafe IntPtr GetSetStringDelegate(StringProvider provider)
        {
            if (_SetStringDelegate == null)
            {
                StringProvider_Invoke native_invoke = (IntPtr thisPtr, IntPtr* value) =>
                    WinRT.Delegate.MarshalInvoke(thisPtr, (StringProvider invoke) =>
                    {
                        var str = invoke();
                        IntPtr handle;
                        Marshal.ThrowExceptionForHR(WinRT.Platform.WindowsCreateString(str, str.Length, &handle));
                        *value = handle;
                    });

                _SetStringDelegate = new WinRT.Delegate(Marshal.GetFunctionPointerForDelegate(native_invoke), provider);
            }
            return _SetStringDelegate.ThisPtr;
        }

        public WinRT.EventSource<Class, UInt32> _IntPropertyChanged;
        public WinRT.EventSource<Class, WinRT.HString> _StringPropertyChanged;
    }

    public class Class
    {
        IClass _class;

        public Class() : this(ActivationFactory<Class>.ActivateInstance<IClass.Vftbl>()) { }

        internal Class(IClass c)
        {
            _class = c;
            _class._IntPropertyChanged.Sender = this;
            _class._StringPropertyChanged.Sender = this;
        }

        public int IntProperty
        {
            get => _class.IntProperty;
            set => _class.IntProperty = value;
        }

        public String StringProperty
        {
            get => _class.StringProperty;
            set => _class.StringProperty = value;
        }

        public event WinRT.TypedEventHandler<Class, WinRT.HString> StringPropertyChanged
        {
            add { _class.StringPropertyChanged += value; }
            remove { _class.StringPropertyChanged -= value; }
        }

        public String StringProperty2
        {
            get => _class.StringProperty2;
            set => _class.StringProperty2 = value;
        }

        public IEnumerable<String> StringsProperty
        {
            get => _class.StringsProperty;
            set => _class.StringsProperty = value;
        }

        public void GetString()
        {
            _class.GetString();
        }

        public void SetString(StringProvider provider)
        {
            _class.SetString(provider);
        }
    }
}
