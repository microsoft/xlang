using System;
using System.Runtime.InteropServices;
using WinRT;

namespace Sample
{
    internal class IClass
    {
        // IClass
        [Guid("CB801447-236A-54A1-8103-F91996A225ED")]
        public struct Vftbl
        {
            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public WinRT.Interop._get_PropertyAsInt get_MyProperty;
            public WinRT.Interop._put_PropertyAsInt put_MyProperty;
            public WinRT.Interop._add_EventHandler add_MyPropertyChanged;
            public WinRT.Interop._remove_EventHandler remove_MyPropertyChanged;
            public WinRT.Interop._get_PropertyAsObject get_Derived1;
            public WinRT.Interop._get_PropertyAsObject get_Derived2;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;

        public static implicit operator IClass(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IClass(WinRT.ObjectReference<Vftbl> obj) => new IClass(obj);
        public IClass(WinRT.ObjectReference<Vftbl> obj)
        {
            _obj = obj;
            MyPropertyChanged = new WinRT.EventSource<Class, object>(_obj, _obj.Vftbl.add_MyPropertyChanged, _obj.Vftbl.remove_MyPropertyChanged, (IntPtr) => null);
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

        public readonly WinRT.EventSource<Class, object> MyPropertyChanged;

        public Base Derived1
        {
            get
            {
                IntPtr instancePtr;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Derived1(_obj.ThisPtr, &instancePtr)); }
                return new Base(WinRT.ObjectReference<IBase.Vftbl>.Attach(_obj.Module, ref instancePtr));
            }
        }

        public Derived2 Derived2
        {
            get
            {
                IntPtr instancePtr;
                unsafe { Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Derived2(_obj.ThisPtr, &instancePtr)); }
                return new Derived2(new IDerived2(WinRT.ObjectReference<IDerived2.Vftbl>.Attach(_obj.Module, ref instancePtr)));
            }
        }
    }

    public class Class
    {
        IClass _class;

        public Class() : this(ActivationFactory<Class>._ActivateInstance<IClass.Vftbl>()) { }

        internal Class(IClass c)
        {
            _class = c;
            _class.MyPropertyChanged.Sender = this;
        }

        public event WinRT.TypedEventHandler<Class, object> MyPropertyChanged
        {
            add => _class.MyPropertyChanged.Event += value;
            remove => _class.MyPropertyChanged.Event -= value;
        }

        public int MyProperty
        {
            get => _class.MyProperty;
            set => _class.MyProperty = value;
        }

        public Base Derived1 => _class.Derived1;
        public Derived2 Derived2 => _class.Derived2;
    }


    internal class IBase
    {
        [Guid("7ED23C25-EECA-548D-A216-156D1F4784F0")]
        public struct Vftbl
        {
            public delegate int _Hello(IntPtr thisPtr);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _Hello Hello;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;
        public static implicit operator IBase(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IBase(WinRT.ObjectReference<Vftbl> obj) => new IBase(obj);
        public IBase(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

        public void Hello()
        {
            Marshal.ThrowExceptionForHR(_obj.Vftbl.Hello(_obj.ThisPtr));
        }
    }

    internal class IDerived1
    {
        [Guid("B952CE78-D50F-5238-92F4-935F12769D1F")]
        public struct Vftbl
        {
            public unsafe delegate int _Planet(IntPtr thisPtr, int* value);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _Planet Planet;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;
        public static implicit operator IDerived1(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IDerived1(WinRT.ObjectReference<Vftbl> obj) => new IDerived1(obj);
        public IDerived1(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

        public unsafe int Planet()
        {
            int value;
            Marshal.ThrowExceptionForHR(_obj.Vftbl.Planet(_obj.ThisPtr, &value));
            return value;
        }
    }

    internal class IDerived2
    {
        [Guid("8E24A1AE-1AFB-570A-ABEF-0BAB89BC1223")]
        public struct Vftbl
        {
            public delegate int _World(IntPtr thisPtr, int value);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _World World;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;
        public static implicit operator IDerived2(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IDerived2(WinRT.ObjectReference<Vftbl> obj) => new IDerived2(obj);
        public IDerived2(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

        public void World(int value)
        {
            Marshal.ThrowExceptionForHR(_obj.Vftbl.World(_obj.ThisPtr, value));
        }
    }

    internal class IGrandchild
    {
        [Guid("91F9943D-15FF-54DF-B705-C0514C5A820F")]
        public struct Vftbl
        {
            public delegate int _Galaxy(IntPtr thisPtr, int repeat);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _Galaxy Galaxy;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;
        public static implicit operator IGrandchild(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator IGrandchild(WinRT.ObjectReference<Vftbl> obj) => new IGrandchild(obj);
        public IGrandchild(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

        public void Galaxy(int repeat)
        {
            Marshal.ThrowExceptionForHR(_obj.Vftbl.Galaxy(_obj.ThisPtr, repeat));
        }
    }

    public class Base
    {
        internal readonly IBase _base;

        internal Base(IBase b)
        {
            _base = b;
        }

        public void Hello() => _base.Hello();
    }

    public class Derived1
    {
        public static implicit operator Base(Derived1 d) => new Base(d._base);
        public static explicit operator Derived1(Base b) => new Derived1(b._base);

        internal readonly IBase _base;
        internal readonly IDerived1 _derived1;

        internal Derived1(IBase b) : this(b, b._obj) { }
        internal Derived1(IDerived1 d) : this(d._obj, d) { }
        internal Derived1(IBase b, IDerived1 d)
        {
            _base = b;
            _derived1 = d;
        }

        public void Hello() => _base.Hello();
        public int Planet() => _derived1.Planet();
    }

    public class Derived2
    {
        public static implicit operator Base(Derived2 d) => new Base(d._base);
        public static explicit operator Derived2(Base b) => new Derived2(b._base);

        internal readonly IBase _base;
        internal IDerived2 _derived2;

        internal Derived2(IBase b) : this(b, b._obj) { }
        internal Derived2(IDerived2 d) : this(d._obj, d) { }
        internal Derived2(IBase b, IDerived2 d)
        {
            _base = b;
            _derived2 = d;
        }

        public void Hello() => _base.Hello();
        public void World(int value) => _derived2.World(value);
    }

    public class Grandchild
    {
        public static implicit operator Base(Grandchild g) => new Base(g._base);
        public static implicit operator Derived2(Grandchild g) => new Derived2(g._base, g._derived2);
        public static explicit operator Grandchild(Base b) => new Grandchild(b._base);
        public static explicit operator Grandchild(Derived2 d) => new Grandchild(d._base, d._derived2);

        internal readonly IBase _base;
        internal readonly IDerived2 _derived2;
        internal readonly IGrandchild _grandchild;

        internal Grandchild(IBase b) : this(b, b._obj, b._obj) { }
        internal Grandchild(IBase b, IDerived2 d) : this(b, d, b._obj) { }
        internal Grandchild(IGrandchild g) : this(g._obj, g._obj, g) { }
        internal Grandchild(IBase b, IDerived2 d, IGrandchild g)
        {
            _base = b;
            _derived2 = d;
            _grandchild = g;
        }

        public void Hello() => _base.Hello();
        public void World(int value) => _derived2.World(value);
        public void Galaxy(int repeat) => _grandchild.Galaxy(repeat);
    }
}
