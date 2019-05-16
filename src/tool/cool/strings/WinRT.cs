using System;
using System.Runtime.InteropServices;

namespace WinRT
{
    // IUnknown
    unsafe delegate int QueryInterface([In] IntPtr pThis, [In] ref Guid iid, [Out] IntPtr* vftbl);
    delegate uint AddRef([In] IntPtr pThis);
    delegate uint Release([In] IntPtr pThis);

    [Guid("00000000-0000-0000-C000-000000000046")]
    internal struct IUnknownVftbl
    {
        public QueryInterface QueryInterface;
        public AddRef AddRef;
        public Release Release;
    }

    // IInspectable
    enum TrustLevel
    {
        BaseTrust = 0,
        PartialTrust = (BaseTrust + 1),
        FullTrust = (PartialTrust + 1)
    };

    delegate int GetIids([In] IntPtr pThis, [Out] uint iidCount, [Out] Guid[] iids);
    delegate int GetRuntimeClassName([In] IntPtr pThis, [Out] IntPtr className);
    delegate int GetTrustLevel([In] IntPtr pThis, [Out] TrustLevel trustLevel);

    [Guid("AF86E2E0-B12D-4c6a-9C5A-D7AA65101E90")]
    internal struct IInspectableVftbl
    {
        public IUnknownVftbl IUnknownVftbl;
        public GetIids GetIids;
        public GetRuntimeClassName GetRuntimeClassName;
        public GetTrustLevel GetTrustLevel;
    }

    // IActivationFactory
    unsafe delegate int ActivateInstance([In] IntPtr pThis, [Out] IntPtr* instance);
    [Guid("00000035-0000-0000-C000-000000000046")]
    internal struct IActivationFactoryVftbl
    {
        public IInspectableVftbl IInspectableVftbl;
        public ActivateInstance ActivateInstance;
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
        IUnknownVftbl _vftblIUnknown;
        public readonly T Vftbl;
        public readonly IntPtr ThisPtr;
        public readonly ModuleReference Module;

        public ObjectReference(ModuleReference module, IntPtr thisPtr)
        {
            Module = (ModuleReference)module.Clone();
            ThisPtr = thisPtr;
            var vftblPtr = Marshal.PtrToStructure<VftblPtr>(ThisPtr);
            _vftblIUnknown = Marshal.PtrToStructure<IUnknownVftbl>(vftblPtr.Vftbl);
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
            Module.Dispose();
        }

        public object Clone()
        {
            _vftblIUnknown.AddRef(ThisPtr);
            return new ObjectReference<T>(Module, ThisPtr);
        }
    }

    internal abstract class IModule : IDisposable
    {
        uint _refs = 0;
        protected static object _cacheLock = new object();
        public void AddRef() { ++_refs; }
        public void Release()
        {
            lock (_cacheLock)
            {
                if (_refs == 0)
                {
                    throw new IndexOutOfRangeException();
                }

                --_refs;
                if (_refs == 0)
                {
                    Dispose();
                }
            }
        }
        public abstract ObjectReference<IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId);
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

        public ObjectReference<IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId) { return _module.GetActivationFactory(runtimeClassId); }

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

        public override ObjectReference<IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(_GetActivationFactory(runtimeClassId.Handle, &instancePtr)); }
            using (var module = new ModuleReference(this))
            {
                return new ObjectReference<IActivationFactoryVftbl>(module, instancePtr);
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

        public override ObjectReference<IActivationFactoryVftbl> GetActivationFactory(HString runtimeClassId)
        {
            Guid iid = typeof(IActivationFactoryVftbl).GUID;
            IntPtr instancePtr;
            unsafe { Marshal.ThrowExceptionForHR(Platform.RoGetActivationFactory(runtimeClassId.Handle, ref iid, &instancePtr)); }
            using (var module = new ModuleReference(this))
            {
                return new ObjectReference<IActivationFactoryVftbl>(module, instancePtr);
            }
        }

        public override void Dispose()
        {
            Marshal.ThrowExceptionForHR(Platform.CoDecrementMTAUsage(_mtaCookie));
        }
    }


    internal class ActivationFactory<T> : IDisposable
    {
        ObjectReference<IActivationFactoryVftbl> _IActivationFactory;

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
            using (var instance = new ObjectReference<IInspectableVftbl>(_IActivationFactory.Module, instancePtr))
            {
                return instance.As<T>();
            }
        }

        public void Dispose()
        {
            _IActivationFactory.Dispose();
        }
    }
}
