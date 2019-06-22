using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Numerics;

namespace WinRT
{
    public enum TrustLevel
    {
        BaseTrust = 0,
        PartialTrust = (BaseTrust + 1),
        FullTrust = (PartialTrust + 1)
    };

    public delegate void EventHandler<A>(object sender, A args);
    public delegate void TypedEventHandlerNoSender<A>(A args);
    public delegate void TypedEventHandler<S, A>(S sender, A args);

    namespace Interop
    {
        // IUnknown

        [Guid("00000000-0000-0000-C000-000000000046")]
        public struct IUnknownVftbl
        {
            public unsafe delegate int _QueryInterface([In] IntPtr pThis, [In] ref Guid iid, IntPtr* vftbl);
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

        // standard accessors/mutators
        public unsafe delegate int _get_PropertyAsInt([In] IntPtr thisPtr, [Out] int* value);
        public delegate int _put_PropertyAsInt([In] IntPtr thisPtr, [In] int value);
        public unsafe delegate int _get_PropertyAsUInt([In] IntPtr thisPtr, [Out] uint* value);
        public delegate int _put_PropertyAsUInt([In] IntPtr thisPtr, [In] uint value);
        public unsafe delegate int _get_PropertyAsLong([In] IntPtr thisPtr, [Out] long* value);
        public delegate int _put_PropertyAsLong([In] IntPtr thisPtr, [In] long value);
        public unsafe delegate int _get_PropertyAsBool([In] IntPtr thisPtr, [Out, MarshalAs(UnmanagedType.Bool)] bool* value);
        public delegate int _put_PropertyAsBool([In] IntPtr thisPtr, [In, MarshalAs(UnmanagedType.Bool)] bool value);
        public unsafe delegate int _get_PropertyAsFloat([In] IntPtr thisPtr, [Out] float* value);
        public delegate int _put_PropertyAsFloat([In] IntPtr thisPtr, [In] float value);
        public unsafe delegate int _get_PropertyAsByte([In] IntPtr thisPtr, [Out] byte* value);
        public delegate int _put_PropertyAsByte([In] IntPtr thisPtr, [In] byte value);
        public unsafe delegate int _get_PropertyAsObject([In] IntPtr thisPtr, [Out] IntPtr* value);
        public delegate int _put_PropertyAsObject([In] IntPtr thisPtr, [In] IntPtr value);
        public unsafe delegate int _get_PropertyAsGuid([In] IntPtr thisPtr, [Out] Guid* value);
        public delegate int _put_PropertyAsGuid([In] IntPtr thisPtr, [In] Guid value);
        public unsafe delegate int _get_PropertyAsVector3([In] IntPtr thisPtr, [Out] Vector3* value);
        public delegate int _put_PropertyAsVector3([In] IntPtr thisPtr, [In] Vector3 value);
        public unsafe delegate int _get_PropertyAsQuaternion([In] IntPtr thisPtr, [Out] Quaternion* value);
        public delegate int _put_PropertyAsQuaternion([In] IntPtr thisPtr, [In] Quaternion value);
        public unsafe delegate int _get_PropertyAsMatrix4x4([In] IntPtr thisPtr, [Out] Matrix4x4* value);
        public delegate int _put_PropertyAsMatrix4x4([In] IntPtr thisPtr, [In] Matrix4x4 value);
        public unsafe delegate int _add_EventHandler([In] IntPtr thisPtr, [In] IntPtr handler, [Out] WinRT.Interop.EventRegistrationToken* token);
        public delegate int _remove_EventHandler([In] IntPtr thisPtr, [In] WinRT.Interop.EventRegistrationToken token);

        // IReference
        [Guid("dacbffdc-68ef-5fd0-b657-782d0ac9807e")]
        public struct IReference_Matrix4x4
        {
            IInspectableVftbl IInspectableVftbl;
            public _get_PropertyAsMatrix4x4 get_Value;
        }

        // IDelegate
        public struct IDelegateVftbl
        {
            public IntPtr QueryInterface;
            public IntPtr AddRef;
            public IntPtr Release;
            public IntPtr Invoke;
        }

        public delegate int IEventHandlerNoSender_Invoke([In] IntPtr thisPtr, [In] IntPtr args);
        public delegate int IEventHandler_Invoke([In] IntPtr thisPtr, [In] IntPtr sender, [In] IntPtr args);

        public struct EventRegistrationToken
        {
            public long Value;
        }

        // IIterator
        public struct IIteratorOfObject
        {
            public unsafe delegate int _MoveNext([In] IntPtr thisPtr, [Out, MarshalAs(UnmanagedType.Bool)] bool* hasCurrent);
            public unsafe delegate int _GetMany([In] IntPtr thisPtr, [In] uint capacity, [In] ref IntPtr[] values, [Out] uint* actual);

            public IInspectableVftbl IInspectableVftbl;
            public _get_PropertyAsObject get_Current;
            public _get_PropertyAsBool get_HasCurrent;
            public _MoveNext MoveNext;
            public _GetMany GetMany;
        }

        public struct IIterableOfObject
        {
            public IInspectableVftbl IInspectableVftbl;
            public _get_PropertyAsObject get_First;
        }

        public struct IVectorViewOfObject
        {
            public unsafe delegate int _GetAt([In] IntPtr thisPtr, [In] uint index, [Out] IntPtr* result);
            public unsafe delegate int _IndexOf([In] IntPtr thisPtr, [In] IntPtr value, [Out] uint* index, [Out, MarshalAs(UnmanagedType.Bool)] bool* found);
            public unsafe delegate int _GetMany([In] IntPtr thisPtr, [In] uint startingIndex, [In] uint capacity, [In] ref IntPtr[] values, [Out] uint* actual);

            public IInspectableVftbl IInspectableVftbl;
            public _GetAt GetAt;
            public _get_PropertyAsUInt get_Size;
            public _IndexOf IndexOf;
            public _GetMany GetMany;
        }

        public struct IIteratorOfByte
        {
            public unsafe delegate int _MoveNext([In] IntPtr thisPtr, [Out, MarshalAs(UnmanagedType.Bool)] bool* hasCurrent);
            public unsafe delegate int _GetMany([In] IntPtr thisPtr, [In] uint capacity, [In] ref byte[] values, [Out] uint* actual);

            public IInspectableVftbl IInspectableVftbl;
            public _get_PropertyAsByte get_Current;
            public _get_PropertyAsBool get_HasCurrent;
            public _MoveNext MoveNext;
            public _GetMany GetMany;
        }

        public struct IIterableOfByte
        {
            public IInspectableVftbl IInspectableVftbl;
            public _get_PropertyAsObject get_First;
        }

        public struct IVectorViewOfByte
        {
            public unsafe delegate int _GetAt([In] IntPtr thisPtr, [In] uint index, [Out] byte* result);
            public unsafe delegate int _IndexOf([In] IntPtr thisPtr, [In] byte value, [Out] uint* index, [Out, MarshalAs(UnmanagedType.Bool)] bool* found);
            public unsafe delegate int _GetMany([In] IntPtr thisPtr, [In] uint startingIndex, [In] uint capacity, [In] ref byte[] values, [Out] uint* actual);

            public IInspectableVftbl IInspectableVftbl;
            public _GetAt GetAt;
            public _get_PropertyAsUInt get_Size;
            public _IndexOf IndexOf;
            public _GetMany GetMany;
        }
    }

    internal class Platform
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr LoadLibraryExW([MarshalAs(UnmanagedType.LPWStr)] string fileName, IntPtr fileHandle, uint flags);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool FreeLibrary(IntPtr moduleHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
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
        public static extern unsafe char* WindowsGetStringRawBuffer(IntPtr hstring, [Out] uint* length);

        [DllImport("api-ms-win-core-winrt-string-l1-1-0.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int WindowsDeleteString(IntPtr hstring);
    }

    internal class Mono
    {
        static Lazy<bool> _usingMono = new Lazy<bool>(() =>
        {
            var modulePtr = Platform.LoadLibraryExW("mono-2.0-bdwgc.dll", IntPtr.Zero, 0);
            if (modulePtr == IntPtr.Zero) return false;

            if (!Platform.FreeLibrary(modulePtr))
            {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }
            return true;
        });

        [DllImport("mono-2.0-bdwgc.dll")]
        static extern IntPtr mono_thread_current();

        [DllImport("mono-2.0-bdwgc.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool mono_thread_is_foreign(IntPtr threadPtr);

        [DllImport("mono-2.0-bdwgc.dll")]
        static extern void mono_unity_thread_fast_attach(IntPtr domainPtr);

        [DllImport("mono-2.0-bdwgc.dll")]
        static extern void mono_unity_thread_fast_detach();

        [DllImport("mono-2.0-bdwgc.dll")]
        static extern void mono_thread_pop_appdomain_ref();

        [DllImport("mono-2.0-bdwgc.dll")]
        static extern IntPtr mono_domain_get();

        struct MonoObject
        {
            IntPtr vtable;
            IntPtr synchronisation; // sic
        }

        unsafe struct MonoThread
        {
            MonoObject obj;
            public MonoInternalThread_x64* internal_thread;
            IntPtr start_obj;
            IntPtr pending_exception;
        }

        [Flags]
        enum MonoThreadFlag : int
        {
            MONO_THREAD_FLAG_DONT_MANAGE = 1,
            MONO_THREAD_FLAG_NAME_SET = 2,
            MONO_THREAD_FLAG_APPDOMAIN_ABORT = 4,
        }

        [StructLayout(LayoutKind.Explicit)]
        struct MonoInternalThread_x64
        {
            [FieldOffset(0xd0)]
            public MonoThreadFlag flags;
        }

        public class ThreadContext : IDisposable
        {
            static Lazy<HashSet<IntPtr>> _foreignThreads = new Lazy<HashSet<IntPtr>>();

            readonly IntPtr _threadPtr = IntPtr.Zero;

            public ThreadContext()
            {
                if (_usingMono.Value)
                {
                    // nothing to do for Mono-native threads
                    var threadPtr = mono_thread_current();
                    if (mono_thread_is_foreign(threadPtr))
                    {
                        // initialize this thread the first time it runs managed code, and remember it for future reference
                        if (_foreignThreads.Value.Add(threadPtr))
                        {
                            // clear initial appdomain ref for new foreign threads to avoid deadlock on domain unload
                            mono_thread_pop_appdomain_ref();

                            unsafe
                            {
                                // tell Mono to ignore the thread on process shutdown since there's nothing to synchronize with
                                ((MonoThread*)threadPtr)->internal_thread->flags |= MonoThreadFlag.MONO_THREAD_FLAG_DONT_MANAGE;
                            }
                        }

                        unsafe
                        {
                            // attach as Unity does to set up the proper domain for the call
                            mono_unity_thread_fast_attach(mono_domain_get());
                            _threadPtr = threadPtr;
                        }
                    }
                }
            }

            public void Dispose()
            {
                if (_threadPtr != IntPtr.Zero)
                {
                    // detach as Unity does to properly reset the domain context
                    mono_unity_thread_fast_detach();
                }
            }
        }
    }

    internal class HString : ICloneable, IDisposable
    {
        public readonly IntPtr Handle;

        public unsafe HString(string value)
        {
            IntPtr handle;
            Marshal.ThrowExceptionForHR(Platform.WindowsCreateString(value, value.Length, &handle));
            Handle = handle;
        }

        public HString(IntPtr handle)
        {
            Handle = handle;
        }

        public unsafe override string ToString()
        {
            uint length;
            char* buffer = Platform.WindowsGetStringRawBuffer(Handle, &length);
            return new string(buffer, 0, (int)length);
        }

        public unsafe object Clone()
        {
            IntPtr handle;
            Marshal.ThrowExceptionForHR(Platform.WindowsDuplicateString(Handle, &handle));
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

    internal class IObjectReference
    {
        public readonly IntPtr ThisPtr;
        public readonly object Module;
        readonly GCHandle _moduleHandle;

        protected IObjectReference(object module, IntPtr thisPtr)
        {
            Module = module;
            ThisPtr = thisPtr;
            if (Module != null)
            {
                _moduleHandle = GCHandle.Alloc(module);
            }
        }

        ~IObjectReference()
        {
            if (_moduleHandle.IsAllocated)
            {
                _moduleHandle.Free();
            }
        }
    }

    internal class ObjectReference<T> : IObjectReference
    {
        Interop.IUnknownVftbl _vftblIUnknown;
        public readonly T Vftbl;
        readonly bool _owned;

        public static ObjectReference<T> Attach(object module, ref IntPtr thisPtr)
        {
            var obj = new ObjectReference<T>(module, thisPtr, true);
            thisPtr = IntPtr.Zero;
            return obj;
        }

        public static ObjectReference<T> FromNativePtr(object module, IntPtr thisPtr)
        {
            var obj = new ObjectReference<T>(module, thisPtr, true);
            obj._vftblIUnknown.AddRef(obj.ThisPtr);
            return obj;
        }

        public static ObjectReference<T> FromNativePtrNoRef(IntPtr thisPtr)
        {
            return new ObjectReference<T>(null, thisPtr, false);
        }


        ObjectReference(object module, IntPtr thisPtr, bool owned) :
            base(module, thisPtr)
        {
            _owned = owned;
            var vftblPtr = Marshal.PtrToStructure<VftblPtr>(ThisPtr);
            _vftblIUnknown = Marshal.PtrToStructure<Interop.IUnknownVftbl>(vftblPtr.Vftbl);
            Vftbl = Marshal.PtrToStructure<T>(vftblPtr.Vftbl);
        }

        public ObjectReference<T> As<T>() => As<T>(typeof(T).GUID);
        public ObjectReference<T> As<T>(Guid iid)
        {
            IntPtr thatPtr;
            unsafe { Marshal.ThrowExceptionForHR(_vftblIUnknown.QueryInterface(ThisPtr, ref iid, &thatPtr)); }
            return ObjectReference<T>.Attach(Module, ref thatPtr);
        }

        ~ObjectReference()
        {
            if (_owned)
            {
                _vftblIUnknown.Release(ThisPtr);
            }
        }
    }

    internal class DllModule
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public unsafe delegate int DllGetActivationFactory(
            [In] IntPtr activatableClassId,
            [Out] IntPtr* activationFactory);

        readonly string _fileName;
        readonly IntPtr _moduleHandle;
        readonly DllGetActivationFactory _GetActivationFactory;

        static readonly string _currentModuleDirectory = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);

        static Dictionary<string, WeakReference<DllModule>> _cache = new System.Collections.Generic.Dictionary<string, WeakReference<DllModule>>();

        public static DllModule Load(string fileName)
        {
            lock (_cache)
            {
                WeakReference<DllModule> weakModule;
                DllModule module;
                if (!(_cache.TryGetValue(fileName, out weakModule) && weakModule.TryGetTarget(out module)))
                {
                    module = new DllModule(fileName);
                    _cache[fileName] = new WeakReference<DllModule>(module);
                }
                return module;
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

        public ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(_GetActivationFactory(runtimeClassId.Handle, &instancePtr)); }
            return ObjectReference<Interop.IActivationFactoryVftbl>.Attach(this, ref instancePtr);
        }

        ~DllModule()
        {
            lock (_cache)
            {
                _cache.Remove(_fileName);
            }

            if (!Platform.FreeLibrary(_moduleHandle))
            {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }
        }
    }

    internal class WeakLazy<T> where T : class, new()
    {
        WeakReference<T> _instance = new WeakReference<T>(null);
        public T Value
        {
            get
            {
                lock (_instance)
                {
                    T value;
                    if (!_instance.TryGetTarget(out value))
                    {
                        value = new T();
                        _instance.SetTarget(value);
                    }
                    return value;
                }
            }
        }
    }

    internal class WinrtModule
    {
        readonly IntPtr _mtaCookie;
        static WeakLazy<WinrtModule> _instance = new WeakLazy<WinrtModule>();
        public static WinrtModule Instance => _instance.Value;

        public WinrtModule()
        {
            IntPtr mtaCookie;
            unsafe { Marshal.ThrowExceptionForHR(Platform.CoIncrementMTAUsage(&mtaCookie)); }
            _mtaCookie = mtaCookie;
        }

        public static ObjectReference<Interop.IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            var module = Instance;
            Guid iid = typeof(Interop.IActivationFactoryVftbl).GUID;
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(Platform.RoGetActivationFactory(runtimeClassId.Handle, ref iid, &instancePtr)); }
            return ObjectReference<Interop.IActivationFactoryVftbl>.Attach(module, ref instancePtr);
        }

        ~WinrtModule()
        {
            Marshal.ThrowExceptionForHR(Platform.CoDecrementMTAUsage(_mtaCookie));
        }
    }


    internal class ActivationFactory<T>
    {
        ObjectReference<Interop.IActivationFactoryVftbl> _IActivationFactory;

        public ActivationFactory()
        {
            string moduleName = typeof(T).Namespace;

            using (HString runtimeClassId = new HString(typeof(T).FullName.Replace("WinRT", "Windows")))
            {
                do
                {
                    try
                    {
                        var module = DllModule.Load(moduleName + ".dll");
                        _IActivationFactory = module.GetActivationFactory(runtimeClassId);
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
                if (_IActivationFactory == null)
                {
                    _IActivationFactory = WinrtModule.GetActivationFactory(runtimeClassId);
                }
            }
        }

        public ObjectReference<T> ActivateInstance<T>()
        {
            IntPtr instancePtr = IntPtr.Zero;
            unsafe { Marshal.ThrowExceptionForHR(_IActivationFactory.Vftbl.ActivateInstance(_IActivationFactory.ThisPtr, &instancePtr)); }
            return ObjectReference<Interop.IInspectableVftbl>.Attach(_IActivationFactory.Module, ref instancePtr).As<T>();
        }

        public ObjectReference<T> As<T>()
        {
            return _IActivationFactory.As<T>();
        }
    }

    public abstract class RuntimeClass<C, I>
    {
        private protected static WeakLazy<WinRT.ActivationFactory<C>> _factory = new WeakLazy<ActivationFactory<C>>();
        private protected readonly ObjectReference<I> _obj;

        private protected RuntimeClass(ObjectReference<I> obj)
        {
            _obj = obj;
        }
    }

    internal class Delegate
    {
        int _refs = 0;
        public readonly IntPtr ThisPtr;

        protected static Delegate FindObject(IntPtr thisPtr)
        {
            UnmanagedObject unmanagedObject = Marshal.PtrToStructure<UnmanagedObject>(thisPtr);
            GCHandle thisHandle = GCHandle.FromIntPtr(unmanagedObject._gchandlePtr);
            return (Delegate)thisHandle.Target;
        }

        // IUnknown
        static unsafe readonly Interop.IUnknownVftbl._QueryInterface _QueryInterface = new Interop.IUnknownVftbl._QueryInterface(QueryInterface);
        static readonly Interop.IUnknownVftbl._AddRef _AddRef = new Interop.IUnknownVftbl._AddRef(AddRef);
        static readonly Interop.IUnknownVftbl._Release _Release = new Interop.IUnknownVftbl._Release(Release);

        static unsafe int QueryInterface([In] IntPtr thisPtr, [In] ref Guid iid, IntPtr* obj)
        {
            // TODO: verify iid
            AddRef(thisPtr);
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

        // IUnknown
        uint AddRef()
        {
            System.Threading.Interlocked.Increment(ref _refs);
            return (uint)_refs;
        }

        uint Release()
        {
            if (_refs == 0)
            {
                throw new InvalidOperationException("WinRT.Delegate has been over-released!");
            }

            System.Threading.Interlocked.Decrement(ref _refs);
            if (_refs == 0)
            {
                _Dispose();
            }
            return (uint)_refs;
        }

        public static int MarshalInvoke<T>(IntPtr thisPtr, Action<T> invoke)
        {
            try
            {
                using (new Mono.ThreadContext())
                {
                    var invoker = (T)FindObject(thisPtr)._weakInvoker.Target;
                    if (invoker != null)
                    {
                        invoke(invoker);
                    }
                    return 0; // S_OK;
                }
            }
            catch (Exception e)
            {
                return Marshal.GetHRForException(e);
            }
        }

        static Interop.IDelegateVftbl _vftblTemplate;
        static Delegate()
        {
            // lay out the vftable
            _vftblTemplate.QueryInterface = Marshal.GetFunctionPointerForDelegate(_QueryInterface);
            _vftblTemplate.AddRef = Marshal.GetFunctionPointerForDelegate(_AddRef);
            _vftblTemplate.Release = Marshal.GetFunctionPointerForDelegate(_Release);
            _vftblTemplate.Invoke = IntPtr.Zero;
        }

        struct UnmanagedObject
        {
            public IntPtr _vftblPtr;
            public IntPtr _gchandlePtr;
        }

        readonly WinrtModule _module = WinrtModule.Instance;
        readonly GCHandle _moduleHandle;
        readonly GCHandle _thisHandle;
        readonly WeakReference _weakInvoker = new WeakReference(null);
        readonly UnmanagedObject _unmanagedObj;

        public class InitialReference : IDisposable
        {
            Delegate _delegate;
            public IntPtr DelegatePtr => _delegate.ThisPtr;
            public InitialReference(IntPtr invoke, object invoker)
            {
                _delegate = new Delegate(invoke, invoker);
                _delegate.AddRef();
            }

            ~InitialReference()
            {
                Dispose();
            }

            public void Dispose()
            {
                if (_delegate != null)
                {
                    _delegate.Release();
                    _delegate = null;
                }
                GC.SuppressFinalize(this);
            }
        }

        Delegate(IntPtr invokePtr, object invoker)
        {
            _moduleHandle = GCHandle.Alloc(_module);

            var vftbl = _vftblTemplate;
            vftbl.Invoke = invokePtr;

            _unmanagedObj._vftblPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(_vftblTemplate));
            Marshal.StructureToPtr(vftbl, _unmanagedObj._vftblPtr, false);

            _weakInvoker.Target = invoker;
            _thisHandle = GCHandle.Alloc(this);
            _unmanagedObj._gchandlePtr = GCHandle.ToIntPtr(_thisHandle);

            ThisPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(_unmanagedObj));
            Marshal.StructureToPtr(_unmanagedObj, ThisPtr, false);
        }

        ~Delegate()
        {
            _Dispose();
        }

        public void _Dispose()
        {
            if (_refs != 0)
            {
                throw new InvalidOperationException("WinRT.Delegate has been leaked!");
            }

            Marshal.FreeCoTaskMem(ThisPtr);
            _thisHandle.Free();
            _moduleHandle.Free();

            GC.SuppressFinalize(this);
        }
    }

    internal class EventSourceNoSender<A>
    {
        delegate void _Invoke(IntPtr argsPtr);
        static Interop.IEventHandlerNoSender_Invoke _invoke = (IntPtr thisPtr, IntPtr argsPtr) =>
            Delegate.MarshalInvoke(thisPtr, (_Invoke invoke) => invoke(argsPtr));

        internal delegate A UnmarshalArgs(IntPtr argsPtr);
        internal delegate void EventHandler(A args);

        readonly IObjectReference _obj;
        readonly Interop._add_EventHandler _addHandler;
        readonly Interop._remove_EventHandler _removeHandler;
        readonly UnmarshalArgs _unmarshalArgs;

        Interop.EventRegistrationToken _token;
        public event WinRT.TypedEventHandlerNoSender<A> _event;
        public event WinRT.TypedEventHandlerNoSender<A> Event
        {
            add
            {
                lock (this)
                {
                    if (_event == null)
                        using (var reference = new Delegate.InitialReference(Marshal.GetFunctionPointerForDelegate(_invoke), new _Invoke(Invoke)))
                    {
                            Interop.EventRegistrationToken token;
                        unsafe { Marshal.ThrowExceptionForHR(_addHandler(_obj.ThisPtr, reference.DelegatePtr, &token)); }
                        _token = token;
                    }
                    _event += value;
                }
            }
            remove
            {
                _event -= value;
                if (_event == null)
                {
                    _Unsubscribe();
                }
            }
        }

        internal EventSourceNoSender(IObjectReference obj, Interop._add_EventHandler addHandler, Interop._remove_EventHandler removeHandler, UnmarshalArgs unmarshalArgs)
        {
            _obj = obj;
            _addHandler = addHandler;
            _removeHandler = removeHandler;
            _unmarshalArgs = unmarshalArgs;
        }

        ~EventSourceNoSender()
        {
            if (_event != null)
            {
                _Unsubscribe();
            }
        }

        void Invoke(IntPtr argsPtr)
        {
            _event?.Invoke(_unmarshalArgs(argsPtr));
        }

        void _Unsubscribe()
        {
            Marshal.ThrowExceptionForHR(_removeHandler(_obj.ThisPtr, _token));
            _token.Value = 0;
        }
    }

    internal class EventSource<S, A>
    {
        delegate void _Invoke(IntPtr senderPtr, IntPtr argsPtr);
        static Interop.IEventHandler_Invoke _invoke = (IntPtr thisPtr, IntPtr senderPtr, IntPtr argsPtr) =>
            Delegate.MarshalInvoke(thisPtr, (_Invoke invoker) => invoker(senderPtr, argsPtr));

        internal delegate A UnmarshalArgs(IntPtr argsPtr);

        readonly S _sender;
        readonly IObjectReference _obj;
        readonly Interop._add_EventHandler _addHandler;
        readonly Interop._remove_EventHandler _removeHandler;
        readonly UnmarshalArgs _unmarshalArgs;

        Interop.EventRegistrationToken _token;
        public event WinRT.TypedEventHandler<S, A> _event;
        public event WinRT.TypedEventHandler<S, A> Event
        {
            add
            {
                lock (this)
                {
                    if (_event == null)
                        using (var reference = new Delegate.InitialReference(Marshal.GetFunctionPointerForDelegate(_invoke), new _Invoke(Invoke)))
                    {
                            Interop.EventRegistrationToken token;
                        unsafe { Marshal.ThrowExceptionForHR(_addHandler(_obj.ThisPtr, reference.DelegatePtr, &token)); }
                        _token = token;
                    }
                    _event += value;
                }
            }
            remove
            {
                _event -= value;
                if (_event == null)
                {
                    _Unsubscribe();
                }
            }
        }

        internal EventSource(S sender, IObjectReference obj, Interop._add_EventHandler addHandler, Interop._remove_EventHandler removeHandler, UnmarshalArgs unmarshalArgs)
        {
            _sender = sender;
            _obj = obj;
            _addHandler = addHandler;
            _removeHandler = removeHandler;
            _unmarshalArgs = unmarshalArgs;
        }

        ~EventSource()
        {
            _Unsubscribe();
        }

        void Invoke(IntPtr senderPtr, IntPtr argsPtr)
        {
            if (senderPtr != _obj.ThisPtr)
            {
                throw new ArgumentException("Mis-matched sender.");
            }

            _event?.Invoke(_sender, _unmarshalArgs(argsPtr));
        }

        void _Unsubscribe()
        {
            Marshal.ThrowExceptionForHR(_removeHandler(_obj.ThisPtr, _token));
            _token.Value = 0;
        }
    }

    public class VectorViewOfObject<T> : IReadOnlyList<T>
    {
        ObjectReference<Interop.IVectorViewOfObject> _obj;
        Guid _iidIterable;
        internal delegate T CreateT(ObjectReference<Interop.IInspectableVftbl> obj);
        CreateT _createT;
        T _CreateT(ref IntPtr instancePtr) => _createT(ObjectReference<Interop.IInspectableVftbl>.Attach(_obj.Module, ref instancePtr));

        internal VectorViewOfObject(ObjectReference<Interop.IVectorViewOfObject> obj, Guid iidIterable, CreateT createT)
        {
            _obj = obj;
            _iidIterable = iidIterable;
            _createT = createT;
        }

        public class Iterator : IEnumerator<T>
        {
            ObjectReference<Interop.IIteratorOfObject> _obj;
            VectorViewOfObject<T> _parent;
            internal Iterator(VectorViewOfObject<T> parent)
            {
                _parent = parent;
                Reset();
            }

            public unsafe T Current
            {
                get
                {
                    IntPtr instancePtr;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Current(_obj.ThisPtr, &instancePtr));
                    return _parent._CreateT(ref instancePtr);
                }
            }

            object IEnumerator.Current => Current;

            public void Dispose()
            {
                _obj = null;
            }

            public unsafe bool MoveNext()
            {
                bool hasCurrent;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.MoveNext(_obj.ThisPtr, &hasCurrent));
                return hasCurrent;
            }

            public unsafe void Reset()
            {
                var iterable = _parent._obj.As<Interop.IIterableOfObject>(_parent._iidIterable);
                IntPtr iteratorPtr;
                Marshal.ThrowExceptionForHR(iterable.Vftbl.get_First(iterable.ThisPtr, &iteratorPtr));
                _obj = ObjectReference<Interop.IIteratorOfObject>.Attach(_parent._obj.Module, ref iteratorPtr);
            }
        }

        public unsafe int Count
        {
            get
            {
                uint value;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Size(_obj.ThisPtr, &value));
                return (int)value;
            }
        }

        public unsafe T this[int index]
        {
            get
            {
                IntPtr instancePtr;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.GetAt(_obj.ThisPtr, (uint)index, &instancePtr));
                return _CreateT(ref instancePtr);
            }
        }

        public unsafe IEnumerator<T> GetEnumerator()
        {
            return new Iterator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }

    public class VectorViewOfByte : IReadOnlyList<byte>
    {
        ObjectReference<Interop.IVectorViewOfByte> _obj;
        Guid _iidIterable;

        internal VectorViewOfByte(ObjectReference<Interop.IVectorViewOfByte> obj, Guid iidIterable)
        {
            _obj = obj;
            _iidIterable = iidIterable;
        }

        public class Iterator : IEnumerator<byte>
        {
            ObjectReference<Interop.IIteratorOfByte> _obj;
            VectorViewOfByte _parent;
            internal Iterator(VectorViewOfByte parent)
            {
                _parent = parent;
                Reset();
            }

            public unsafe byte Current
            {
                get
                {
                    byte value;
                    Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Current(_obj.ThisPtr, &value));
                    return value;
                }
            }

            object IEnumerator.Current => Current;

            public void Dispose()
            {
                _obj = null;
            }

            public unsafe bool MoveNext()
            {
                bool hasCurrent;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.MoveNext(_obj.ThisPtr, &hasCurrent));
                return hasCurrent;
            }

            public unsafe void Reset()
            {
                var iterable = _parent._obj.As<Interop.IIterableOfByte>(_parent._iidIterable);
                IntPtr iteratorPtr;
                Marshal.ThrowExceptionForHR(iterable.Vftbl.get_First(iterable.ThisPtr, &iteratorPtr));
                _obj = ObjectReference<Interop.IIteratorOfByte>.Attach(_parent._obj.Module, ref iteratorPtr);
            }
        }

        public unsafe int Count
        {
            get
            {
                uint value;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.get_Size(_obj.ThisPtr, &value));
                return (int)value;
            }
        }

        public unsafe byte this[int index]
        {
            get
            {
                byte value;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.GetAt(_obj.ThisPtr, (uint)index, &value));
                return value;
            }
        }

        public unsafe IEnumerator<byte> GetEnumerator()
        {
            return new Iterator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
