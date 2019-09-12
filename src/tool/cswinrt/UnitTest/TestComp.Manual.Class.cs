using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using WinRT;
using TestComp;

namespace TestComp.Manual
{
    public delegate WinRT.HString ProvideString();

    public static class ProvideStringExtensions
    {
        private unsafe delegate int NativeInvoke(IntPtr thisPtr, [Out] IntPtr* value);

        public static unsafe IntPtr ToNative(this ProvideString provideString)
        {
            NativeInvoke native_invoke = (IntPtr thisPtr, IntPtr* value) =>
                WinRT.Delegate.MarshalInvoke(thisPtr, (ProvideString invoke) =>
                {
                    *value = invoke().Handle;
                });

            return new WinRT.Delegate(Marshal.GetFunctionPointerForDelegate(native_invoke), provideString).ThisPtr;
        }
    };

    internal class IClass
    {
        // IClass (IID copied from generated TestComp.0.h)
        [Guid("95F07136-663C-522A-B979-4CD58DF432F6")]
        public struct Vftbl
        {
#pragma warning disable 0169 // warning CS0169: The field '...' is never used
            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
#pragma warning enable 0169
#pragma warning disable 0649 // warning CS0169: Field '...' is never assigned to
            public WinRT.Interop._add_EventHandler add_Event0;
            public WinRT.Interop._remove_EventHandler remove_Event0;
            public delegate int _InvokeEvent0([In] System.IntPtr @this);
            public _InvokeEvent0 InvokeEvent0;
            public WinRT.Interop._add_EventHandler add_Event1;
            public WinRT.Interop._remove_EventHandler remove_Event1;
            public delegate int _InvokeEvent1([In] System.IntPtr @this, System.IntPtr sender);
            public _InvokeEvent1 InvokeEvent1;
            public WinRT.Interop._add_EventHandler add_Event2;
            public WinRT.Interop._remove_EventHandler remove_Event2;
            public delegate int _InvokeEvent2([In] System.IntPtr @this, System.IntPtr sender, int arg0);
            public _InvokeEvent2 InvokeEvent2;
            public WinRT.Interop._add_EventHandler add_Event3;
            public WinRT.Interop._remove_EventHandler remove_Event3;
            public delegate int _InvokeEvent3([In] System.IntPtr @this, System.IntPtr sender, int arg0, System.IntPtr arg1);
            public _InvokeEvent3 InvokeEvent3;

            public WinRT.Interop._add_EventHandler add_EventCollection;
            public WinRT.Interop._remove_EventHandler remove_EventCollection;
            public delegate int _InvokeEventCollection([In] System.IntPtr @this, System.IntPtr sender, int arg0, System.IntPtr arg1);
            public _InvokeEvent3 InvokeEventCollection;

            public WinRT.Interop._add_EventHandler add_NestedEvent;
            public WinRT.Interop._remove_EventHandler remove_NestedEvent;
            public delegate int _InvokeNestedEvent([In] System.IntPtr @this, System.IntPtr sender, int arg0, System.IntPtr arg1);
            public _InvokeEvent3 InvokeNestedEvent;

            public WinRT.Interop._add_EventHandler add_NestedTypedEvent;
            public WinRT.Interop._remove_EventHandler remove_NestedTypedEvent;
            public delegate int _InvokeNestedTypedEvent([In] System.IntPtr @this, System.IntPtr sender, int arg0, System.IntPtr arg1);
            public _InvokeEvent3 InvokeNestedTypedEvent;

            public WinRT.Interop._get_PropertyAsInt32 get_IntProperty;
            public WinRT.Interop._put_PropertyAsInt32 put_IntProperty;
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

            public delegate int _GetString([In] System.IntPtr @this);
            public _GetString GetString;
            public delegate int _SetString([In] System.IntPtr @this, System.IntPtr provideString);
            public _SetString SetString;
#pragma warning enable 0649
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;

        public static implicit operator IClass(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IClass(WinRT.ObjectReference<Vftbl> obj) => new IClass(obj);
        public IClass(WinRT.ObjectReference<Vftbl> obj)
        {
            _obj = obj;

            _IntPropertyChanged =
                new WinRT.EventSource<Object, Int32>(_obj,
                _obj.Vftbl.add_IntPropertyChanged,
                _obj.Vftbl.remove_IntPropertyChanged,
                // add unit tests for:
                // demonstrating 'casting' an object to an interface (Class -> IClass),
                //       both exclusive and polymorphic (e.g. IStringable)
                // demonstrating 'casting' an interface back to its RC (constructing from IUnknown -> exclusive)
                // object equivalence via IUknown, etc
                (IntPtr value) => (obj.ThisPtr == value) ? Owner : ObjectReference<Vftbl>.FromNativePtr(value),
                (IntPtr value) => new MarshaledValue<Int32>(value).UnmarshalFromNative());    

            _StringPropertyChanged =
                new EventSource<Class, HString>(_obj,
                _obj.Vftbl.add_StringPropertyChanged,
                _obj.Vftbl.remove_StringPropertyChanged,
                (IntPtr value) => (obj.ThisPtr == value) ? (Class)Owner : new Class(ObjectReference<Vftbl>.FromNativePtr(value)),
                (IntPtr value) => new MarshaledValue<HString>(value).UnmarshalFromNative());    
        }
        
        public object Owner { get; set; }

        public int IntProperty
        {
            get
            {
                int value = 0;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_IntProperty(_obj.ThisPtr, out value)); }
                return value;
            }
            set
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.put_IntProperty(_obj.ThisPtr, value));
            }
        }

        public event WinRT.EventHandler<Object, Int32> IntPropertyChanged
        {
            add { _IntPropertyChanged.Event += value; }
            remove { _IntPropertyChanged.Event -= value; }
        }

        public HString StringProperty
        {
            get
            {
                unsafe
                {
                    IntPtr value;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_StringProperty(_obj.ThisPtr, out value));
                    return new HString(value);
                }
            }
            set
            {
                unsafe
                {
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.put_StringProperty(_obj.ThisPtr, value.Handle));
                }
            }
        }

        public event WinRT.EventHandler<Class, WinRT.HString> StringPropertyChanged
        {
            add { _StringPropertyChanged.Event += value; }
            remove { _StringPropertyChanged.Event -= value; }
        }

        public HString StringProperty2
        {
            get
            {
                unsafe
                {
                    IntPtr value;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_StringProperty2(_obj.ThisPtr, out value));
                    return new HString(value);
                }
            }
            set
            {
                unsafe
                {
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.put_StringProperty2(_obj.ThisPtr, value.Handle));
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

        public void SetString(ProvideString provideString)
        {
            unsafe
            {
                Marshal.ThrowExceptionForHR(_obj.Vftbl.SetString(_obj.ThisPtr, provideString.ToNative()));
            }
        }

        public WinRT.EventSource<Object, Int32> _IntPropertyChanged;
        public WinRT.EventSource<Class, WinRT.HString> _StringPropertyChanged;
    }

    public class Class
    {
        IClass _default;

//        public static explicit operator Class(Object obj) => new Class((WinRT.ObjectReference<IClass.Vftbl>)obj);

        public Class() : this(ActivationFactory<Class>.ActivateInstance<IClass.Vftbl>()) { }

        internal Class(IClass ifc)
        {
            _default = ifc;
            _default.Owner = this;
        }

        public void GetString()
        {
            _default.GetString();
        }

        public void SetString(ProvideString provideString)
        {
            _default.SetString(provideString);
        }

        public int IntProperty
        {
            get => _default.IntProperty;
            set => _default.IntProperty = value;
        }

        public String StringProperty
        {
            get => _default.StringProperty;
            set => _default.StringProperty = value;
        }

        public String StringProperty2
        {
            get => _default.StringProperty2;
            set => _default.StringProperty2 = value;
        }

        public IEnumerable<String> StringsProperty
        {
            get => _default.StringsProperty;
            set => _default.StringsProperty = value;
        }

        public event WinRT.EventHandler<Object, Int32> IntPropertyChanged
        {
            add { _default.IntPropertyChanged += value; }
            remove { _default.IntPropertyChanged -= value; }
        }

        public event WinRT.EventHandler<Class, WinRT.HString> StringPropertyChanged
        {
            add { _default.StringPropertyChanged += value; }
            remove { _default.StringPropertyChanged -= value; }
        }
    }
}
