using System;
using System.Runtime.InteropServices;
using System.Numerics;

namespace WinRT.Perception.Spatial
{
    namespace Interop
    {
        [Guid("69EBCA4B-60A3-3586-A653-59A7BD676D07")]
        public struct ISpatialCoordinateSystemVftbl
        {
            public unsafe delegate int _TryGetTransformTo([In] IntPtr thisPtr, [In] IntPtr targetPtr, [Out] IntPtr* value);

            public WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _TryGetTransformTo TryGetTransformTo;
        }

        [Guid("c042644c-20d8-4ed0-aef7-6805b8e53f55")]
        public struct ISpatialGraphInteropPreviewStaticsVftbl
        {
            public unsafe delegate int _CreateCoordinateSystemForNode
                ([In] IntPtr thisPtr, [In] Guid nodeId, [Out] IntPtr* result);
            public unsafe delegate int _CreateCoordinateSystemForNodeWithPosition
                ([In] IntPtr thisPtr, [In] Guid nodeId, [In] Vector3 position, [Out] IntPtr* result);
            public unsafe delegate int _CreateCoordinateSystemForNodeWithPositionAndOrientation
                ([In] IntPtr thisPtr, [In] Guid nodeId, [In] Vector3 position, [In] Quaternion orientation, [Out] IntPtr* result);
            public unsafe delegate int _CreateLocatorForNode
                ([In] IntPtr thisPtr, [In] Guid nodeId, [Out] IntPtr* result);

            WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _CreateCoordinateSystemForNode CreateCoordinateSystemForNode;
            public _CreateCoordinateSystemForNodeWithPosition CreateCoordinateSystemForNodeWithPosition;
            public _CreateCoordinateSystemForNodeWithPositionAndOrientation CreateCoordinateSystemForNodeWithPositionAndOrientation;
            public _CreateLocatorForNode CreateLocatorForNode;
        }
    }

    public class SpatialCoordinateSystem : WinRT.RuntimeClass<SpatialCoordinateSystem, Interop.ISpatialCoordinateSystemVftbl>
    {
        internal SpatialCoordinateSystem(WinRT.ObjectReference<Interop.ISpatialCoordinateSystemVftbl> obj) :
            base(obj)
        { }

        public static SpatialCoordinateSystem FromNativePtr(IntPtr thisPtr)
        {
            return new SpatialCoordinateSystem(
                WinRT.ObjectReference<WinRT.Interop.IInspectableVftbl>.FromNativePtr(WinRT.WinrtModule.Instance, thisPtr)
                .As<Interop.ISpatialCoordinateSystemVftbl>());
        }

        public unsafe Matrix4x4? TryGetTransformTo(SpatialCoordinateSystem other)
        {
            IntPtr instancePtr = IntPtr.Zero;
            Marshal.ThrowExceptionForHR(_obj.Vftbl.TryGetTransformTo(_obj.ThisPtr, other._obj.ThisPtr, &instancePtr));
            if (instancePtr == IntPtr.Zero)
            {
                return null;
            }

            var reference = WinRT.ObjectReference<WinRT.Interop.IReference_Matrix4x4>.Attach(_obj.Module, ref instancePtr);
            Matrix4x4 result = Matrix4x4.Identity;
            Marshal.ThrowExceptionForHR(reference.Vftbl.get_Value(reference.ThisPtr, &result));
            return result;
        }
    }

    namespace Preview
    {
        public class SpatialGraphInteropPreview
        {
            static WinRT.WeakLazy<WinRT.ActivationFactory<SpatialGraphInteropPreview>> _factory = new WinRT.WeakLazy<WinRT.ActivationFactory<SpatialGraphInteropPreview>>();

            public unsafe static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId)
            {
                var statics = _factory.Value.As<Interop.ISpatialGraphInteropPreviewStaticsVftbl>();
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(statics.Vftbl.CreateCoordinateSystemForNode(statics.ThisPtr, nodeId, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<Interop.ISpatialCoordinateSystemVftbl>.Attach(statics.Module, ref instancePtr));
            }

            public unsafe static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position)
            {
                var statics = _factory.Value.As<Interop.ISpatialGraphInteropPreviewStaticsVftbl>();
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(statics.Vftbl.CreateCoordinateSystemForNodeWithPosition(statics.ThisPtr, nodeId, position, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<Interop.ISpatialCoordinateSystemVftbl>.Attach(statics.Module, ref instancePtr));
            }

            public unsafe static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position, Quaternion orientation)
            {
                var statics = _factory.Value.As<Interop.ISpatialGraphInteropPreviewStaticsVftbl>();
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(statics.Vftbl.CreateCoordinateSystemForNodeWithPositionAndOrientation(statics.ThisPtr, nodeId, position, orientation, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<Interop.ISpatialCoordinateSystemVftbl>.Attach(statics.Module, ref instancePtr));
            }
        }
    }
}