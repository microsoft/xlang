using System;
using System.Runtime.InteropServices;

namespace WinRT
{
    public enum TrustLevel
    {
        BaseTrust = 0,
        PartialTrust = (BaseTrust + 1),
        FullTrust = (PartialTrust + 1)
    };

    namespace Interop
    {
        // IUnknown

        [Guid("00000000-0000-0000-C000-000000000046")]
        public struct IUnknownVftbl
        {
            public unsafe delegate int _QueryInterface([In] IntPtr pThis, [In] ref Guid iid, [Out] IntPtr* vftbl);
            public delegate uint _AddRef([In] IntPtr pThis);
            public delegate uint _Release([In] IntPtr pThis);

            public _QueryInterface QueryInterface;
            public _AddRef AddRef;
            public _Release Release;
        }

        // IInspectable
        [Guid("AF86E2E0-B12D-4c6a-9C5A-D7AA65101E90")]
        public struct IInspectableVftbl
        {
            public delegate int _GetIids([In] IntPtr pThis, [Out] uint iidCount, [Out] Guid[] iids);
            public delegate int _GetRuntimeClassName([In] IntPtr pThis, [Out] IntPtr className);
            public delegate int _GetTrustLevel([In] IntPtr pThis, [Out] TrustLevel trustLevel);

            public IUnknownVftbl IUnknownVftbl;
            public _GetIids GetIids;
            public _GetRuntimeClassName GetRuntimeClassName;
            public _GetTrustLevel GetTrustLevel;
        }

        // IActivationFactory
        [Guid("00000035-0000-0000-C000-000000000046")]
        public struct IActivationFactoryVftbl
        {
            public unsafe delegate int _ActivateInstance([In] IntPtr pThis, [Out] IntPtr* instance);

            public IInspectableVftbl IInspectableVftbl;
            public _ActivateInstance ActivateInstance;
        }

        // IEventHandler
        public struct IEventHandlerVftbl
        {
            public unsafe delegate int _Invoke([In] IntPtr thisPtr, [In] IntPtr sender, [In] IntPtr args);

            public IntPtr QueryInterface;
            public IntPtr AddRef;
            public IntPtr Release;
            public IntPtr GetIids;
            public IntPtr GetRuntimeClassName;
            public IntPtr GetTrustLevel;
            public IntPtr Invoke;
        }
    }

    internal class Platform
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibraryExW([MarshalAs(UnmanagedType.LPWStr)] string fileName, IntPtr fileHandle, uint flags);

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool FreeLibrary(IntPtr moduleHandle);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetProcAddress(IntPtr moduleHandle, [MarshalAs(UnmanagedType.LPStr)] string functionName);

        public static T GetProcAddress<T>(IntPtr moduleHandle)
        {
            IntPtr functionPtr = Platform.GetProcAddress(moduleHandle, typeof(T).Name);
            if (functionPtr == IntPtr.Zero)
            {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }
            return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
        }

        [DllImport("api-ms-win-core-com-l1-1-0.dll")]
        public static extern unsafe int CoIncrementMTAUsage([Out] IntPtr* cookie);

        [DllImport("api-ms-win-core-com-l1-1-0.dll")]
        public static extern int CoDecrementMTAUsage([In] IntPtr cookie);

        [DllImport("api-ms-win-core-winrt-l1-1-0.dll")]
        public static extern unsafe int RoGetActivationFactory(IntPtr runtimeClassId, ref Guid iid, IntPtr* factory);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe int WindowsCreateString([MarshalAs(UnmanagedType.LPWStr)] string sourceString,
                                                  int length,
                                                  [Out] IntPtr* hstring);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe int WindowsDuplicateString([In] IntPtr sourceString,
                                                  [Out] IntPtr* hstring);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int WindowsDeleteString(IntPtr hstring);
    }

    internal class HString : ICloneable, IDisposable
    {
        public readonly IntPtr Handle;

        public HString(string value)
        {
            IntPtr handle;
            unsafe { Marshal.ThrowExceptionForHR(Platform.WindowsCreateString(value, value.Length, &handle)); }
            Handle = handle;
        }

        HString(IntPtr handle)
        {
            Handle = handle;
        }

        public object Clone()
        {
            IntPtr handle;
            unsafe { Marshal.ThrowExceptionForHR(Platform.WindowsDuplicateString(Handle, &handle)); }
            return new HString(handle);
        }

        public void Dispose()
        {
            Marshal.ThrowExceptionForHR(Platform.WindowsDeleteString(Handle));
        }
    }

    internal struct VftblPtr
    {
        public IntPtr Vftbl;
    }

    internal class ObjectReference<T> : IDisposable, ICloneable
    {
        Interop.IUnknownVftbl _vftblIUnknown;
        public readonly T Vftbl;
        public readonly IntPtr ThisPtr;
        public readonly ModuleReference Module;

        public ObjectReference(ModuleReference module, IntPtr thisPtr)
        {
            Module = (ModuleReference)module?.Clone();
            ThisPtr = thisPtr;
            var vftblPtr = Marshal.PtrToStructure<VftblPtr>(ThisPtr);
            _vftblIUnknown = Marshal.PtrToStructure<Interop.IUnknownVftbl>(vftblPtr.Vftbl);
            Vftbl = Marshal.PtrToStructure<T>(vftblPtr.Vftbl);
        }

        public ObjectReference<T> As<T>()
        {
            IntPtr thatPtr;
            var iid = typeof(T).GUID;
            unsafe { Marshal.ThrowExceptionForHR(_vftblIUnknown.QueryInterface(ThisPtr, ref iid, &thatPtr)); }
            return new ObjectReference<T>(Module, thatPtr);
        }

        public void Dispose()
        {
            _vftblIUnknown.Release(ThisPtr);
            Module?.Dispose();
        }

        public object Clone()
        {
            _vftblIUnknown.AddRef(ThisPtr);
            return new ObjectReference<T>(Module, ThisPtr);
        }
    }

    internal abstract class IModule : IDisposable
    {
        int _refs = 0;
        protected static object _cacheLock = new object();
        public void AddRef() { System.Threading.Interlocked.Increment(ref _refs); }
        public void Release()
        {
            lock (_cacheLock)
            {
                if (_refs == 0)
                {
                    throw new IndexOutOfRangeException();
                }

                System.Threading.Interlocked.Decrement(ref _refs);
                if (_refs == 0)
                {
                    Dispose();
                }
            }
        }
        public abstract ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId);
        public abstract void Dispose();
    }

    internal class ModuleReference : ICloneable, IDisposable
    {
        IModule _module;

        public ModuleReference(IModule module)
        {
            _module = module;
            _module.AddRef();
        }

        public ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId) { return _module.GetActivationFactory(runtimeClassId); }

        public object Clone()
        {
            return new ModuleReference(_module);
        }

        public void Dispose()
        {
            _module.Release();
        }
    }

    internal class DllModule : IModule
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public unsafe delegate int DllGetActivationFactory(
            [In] IntPtr activatableClassId,
            [Out] IntPtr* activationFactory);

        readonly string _fileName;
        readonly IntPtr _moduleHandle;
        readonly DllGetActivationFactory _GetActivationFactory;

        static readonly string _currentModuleDirectory = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);

        static System.Collections.Generic.Dictionary<string, IModule> _cache = new System.Collections.Generic.Dictionary<string, IModule>();

        public static ModuleReference Load(string fileName)
        {
            lock (_cacheLock)
            {
                if (!_cache.ContainsKey(fileName))
                {
                    _cache[fileName] = new DllModule(fileName);
                }
                return new ModuleReference(_cache[fileName]);
            }
        }

        DllModule(string fileName)
        {
            _fileName = fileName;

            // Explicitly look for module in the same directory as this one, and
            // use altered search path to ensure any dependencies in the same directory are found.
            _moduleHandle = Platform.LoadLibraryExW(System.IO.Path.Combine(_currentModuleDirectory, fileName), IntPtr.Zero, /* LOAD_WITH_ALTERED_SEARCH_PATH */ 8);
            if (_moduleHandle == IntPtr.Zero)
            {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }

            _GetActivationFactory = Platform.GetProcAddress<DllGetActivationFactory>(_moduleHandle);
        }

        public override ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(_GetActivationFactory(runtimeClassId.Handle, &instancePtr)); }
            using (var module = new ModuleReference(this))
            {
                return new ObjectReference<Interop.IActivationFactoryVftbl>(module, instancePtr);
            }
        }

        public override void Dispose()
        {
            lock (_cacheLock)
            {
                _cache.Remove(_fileName);
            }

            if (!Platform.FreeLibrary(_moduleHandle))
            {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }
        }
    }

    internal class WinrtModule : IModule
    {
        readonly IntPtr _mtaCookie;

        public static ModuleReference Initialize()
        {
            return new ModuleReference(new WinrtModule());
        }

        WinrtModule()
        {
            IntPtr mtaCookie;
            unsafe { Marshal.ThrowExceptionForHR(Platform.CoIncrementMTAUsage(&mtaCookie)); }
            _mtaCookie = mtaCookie;
        }

        public override ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            Guid iid = typeof(Interop.IActivationFactoryVftbl).GUID;
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(Platform.RoGetActivationFactory(runtimeClassId.Handle, ref iid, &instancePtr)); }
            using (var module = new ModuleReference(this))
            {
                return new ObjectReference<Interop.IActivationFactoryVftbl>(module, instancePtr);
            }
        }

        public override void Dispose()
        {
            Marshal.ThrowExceptionForHR(Platform.CoDecrementMTAUsage(_mtaCookie));
        }
    }


    internal class ActivationFactory<T> : IDisposable
    {
        ObjectReference<Interop.IActivationFactoryVftbl> _IActivationFactory;

        public ActivationFactory()
        {
            string moduleName = typeof(T).Namespace;

            using (HString runtimeClassId = new HString(typeof(T).FullName))
            {
                do
                {
                    try
                    {
                        using (var module = DllModule.Load(moduleName + ".dll"))
                        {
                            _IActivationFactory = module.GetActivationFactory(runtimeClassId);
                        }
                    }
                    catch (Exception)
                    {
                        // If activation factory wasn't found, trim sub-namespace off of module name and try again.
                        int lastSegment = moduleName.LastIndexOf(".");
                        if (lastSegment < 0)
                        {
                            lastSegment = 0;
                        }
                        moduleName = moduleName.Remove(lastSegment);
                    }
                } while (_IActivationFactory == null && moduleName.Length > 0);

                // If activation factory wasn't found by namespace, fall back to WinRT activation.
                if (_IActivationFactory == null) using (var module = WinrtModule.Initialize())
                {
                    _IActivationFactory = module.GetActivationFactory(runtimeClassId);
                }
            }
        }

        public ObjectReference<T> ActivateInstance<T>()
        {
            IntPtr instancePtr = IntPtr.Zero;
            unsafe { Marshal.ThrowExceptionForHR(_IActivationFactory.Vftbl.ActivateInstance(_IActivationFactory.ThisPtr, &instancePtr)); }
            using (var instance = new ObjectReference<Interop.IInspectableVftbl>(_IActivationFactory.Module, instancePtr))
            {
                return instance.As<T>();
            }
        }

        public ObjectReference<T> As<T>()
        {
            return _IActivationFactory.As<T>();
        }

        public void Dispose()
        {
            _IActivationFactory.Dispose();
        }
    }

    public abstract class RuntimeClass<C, I> : ICloneable, IDisposable
    {
        private protected static readonly Lazy<WinRT.ActivationFactory<C>> _factory = new Lazy<WinRT.ActivationFactory<C>>();
        private protected readonly ObjectReference<I> _obj;

        private protected RuntimeClass(ObjectReference<I> obj)
        {
            _obj = (ObjectReference<I>)obj.Clone();
        }

        public abstract object Clone();

        public virtual void Dispose()
        {
            _obj.Dispose();
        }
    }

    public delegate void EventHandlerDelegate<S, A>(S sender, A args);

    public abstract class EventHandlerObject<S, A> : IDisposable
        where S : IDisposable
        where A : IDisposable
    {
        EventHandlerDelegate<S, A> _delegate;
        int _refs = 0;
        readonly IntPtr _thisPtr;

        static System.Collections.Generic.Dictionary<IntPtr, EventHandlerObject<S, A>> _objects = new System.Collections.Generic.Dictionary<IntPtr, EventHandlerObject<S, A>>();
        static EventHandlerObject<S, A> FindObject(IntPtr thisPtr) { lock (_objects) { return _objects[thisPtr]; } }

        // IUnknown
        static unsafe readonly Interop.IUnknownVftbl._QueryInterface _QueryInterface = new Interop.IUnknownVftbl._QueryInterface(QueryInterface);
        static readonly Interop.IUnknownVftbl._AddRef _AddRef = new Interop.IUnknownVftbl._AddRef(AddRef);
        static readonly Interop.IUnknownVftbl._Release _Release = new Interop.IUnknownVftbl._Release(Release);

        static unsafe int QueryInterface([In] IntPtr thisPtr, [In] ref Guid iid, [Out] IntPtr* obj)
        {
            // TODO: verify iid
            *obj = thisPtr;
            return 0; // S_OK;
        }

        static uint AddRef([In] IntPtr thisPtr)
        {
            return FindObject(thisPtr).AddRef();
        }

        static uint Release([In] IntPtr thisPtr)
        {
            return FindObject(thisPtr).Release();
        }

        // IInspectable
        static readonly Interop.IInspectableVftbl._GetIids _GetIids = new Interop.IInspectableVftbl._GetIids(GetIids);
        static readonly Interop.IInspectableVftbl._GetRuntimeClassName _GetRuntimeClassName = new Interop.IInspectableVftbl._GetRuntimeClassName(GetRuntimeClassName);
        static readonly Interop.IInspectableVftbl._GetTrustLevel _GetTrustLevel = new Interop.IInspectableVftbl._GetTrustLevel(GetTrustLevel);

        static int GetIids([In] IntPtr pThis, [Out] uint iidCount, [Out] Guid[] iids) { return Marshal.GetHRForException(new NotImplementedException()); }
        static int GetRuntimeClassName([In] IntPtr pThis, [Out] IntPtr className) { return Marshal.GetHRForException(new NotImplementedException()); }
        static int GetTrustLevel([In] IntPtr pThis, [Out] TrustLevel trustLevel) { return Marshal.GetHRForException(new NotImplementedException()); }

        // ITypedEventHandler
        static readonly Interop.IEventHandlerVftbl._Invoke _Invoke = new Interop.IEventHandlerVftbl._Invoke(Invoke);
        static int Invoke(IntPtr thisPtr, IntPtr sender, IntPtr args)
        {
            return FindObject(thisPtr).Invoke(sender, args);
        }

        static VftblPtr _vftblPtr;


        // IUnknown
        uint AddRef()
        {
            System.Threading.Interlocked.Increment(ref _refs);
            return (uint)_refs;
        }

        uint Release()
        {
            System.Threading.Interlocked.Decrement(ref _refs);
            if (_refs == 0)
            {
                Dispose();
                lock (_objects) { _objects.Remove(_thisPtr); }
            }
            return (uint)_refs;
        }

        // ITypedEventHandler
        int Invoke(IntPtr senderPtr, IntPtr argsPtr)
        {
            try
            {
                using (var sender = GetSender(senderPtr))
                using (var args = GetArgs(argsPtr))
                {
                    _delegate(sender, args);
                }
                return 0;
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        static EventHandlerObject()
        {
            // lay out the vftable
            Interop.IEventHandlerVftbl vftbl;
            vftbl.QueryInterface = Marshal.GetFunctionPointerForDelegate(_QueryInterface);
            vftbl.AddRef = Marshal.GetFunctionPointerForDelegate(_AddRef);
            vftbl.Release = Marshal.GetFunctionPointerForDelegate(_Release);
            vftbl.GetIids = Marshal.GetFunctionPointerForDelegate(_GetIids);
            vftbl.GetRuntimeClassName = Marshal.GetFunctionPointerForDelegate(_GetRuntimeClassName);
            vftbl.GetTrustLevel = Marshal.GetFunctionPointerForDelegate(_GetTrustLevel);
            vftbl.Invoke = Marshal.GetFunctionPointerForDelegate(_Invoke);

            _vftblPtr.Vftbl = Marshal.AllocCoTaskMem(Marshal.SizeOf(vftbl));
            Marshal.StructureToPtr(vftbl, _vftblPtr.Vftbl, false);
        }

        public EventHandlerObject(EventHandlerDelegate<S, A> @delegate)
        {
            _thisPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(_vftblPtr));
            Marshal.StructureToPtr(_vftblPtr, _thisPtr, false);
            lock (_objects) { _objects.Add(_thisPtr, this); }
            _delegate = @delegate;
        }

        ~EventHandlerObject()
        {
            Marshal.FreeCoTaskMem(_thisPtr);
        }

        private protected abstract S GetSender(IntPtr senderPtr);
        private protected abstract A GetArgs(IntPtr argsPtr);
        public abstract void Dispose();
    }
}
