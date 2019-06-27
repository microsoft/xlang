using System;
using System.Runtime.InteropServices;
using System.Numerics;

namespace WinRT.Perception.Spatial
{
    internal class ISpatialCoordinateSystem
    {
        [Guid("69EBCA4B-60A3-3586-A653-59A7BD676D07")]
        public struct Vftbl
        {
            public unsafe delegate int _TryGetTransformTo([In] IntPtr thisPtr, [In] IntPtr targetPtr, [Out] IntPtr* value);

            public WinRT.Interop.IInspectableVftbl IInspectableVftbl;
            public _TryGetTransformTo TryGetTransformTo;
        }

        public readonly WinRT.ObjectReference<Vftbl> _obj;

        public static implicit operator ISpatialCoordinateSystem(WinRT.IObjectReference obj) => obj.As<Vftbl>();
        public static implicit operator ISpatialCoordinateSystem(WinRT.ObjectReference<Vftbl> obj) => new ISpatialCoordinateSystem(obj);
        public ISpatialCoordinateSystem(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

        public unsafe Matrix4x4? TryGetTransformTo(ISpatialCoordinateSystem other)
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

    public class SpatialCoordinateSystem
    {
        ISpatialCoordinateSystem _spatialCoordinateSystem;

        internal SpatialCoordinateSystem(WinRT.ObjectReference<ISpatialCoordinateSystem.Vftbl> obj)
        {
            _spatialCoordinateSystem = obj;
        }

        public static SpatialCoordinateSystem FromNativePtr(IntPtr thisPtr)
        {
            return new SpatialCoordinateSystem(
                WinRT.ObjectReference<WinRT.Interop.IInspectableVftbl>.FromNativePtr(WinRT.WinrtModule.Instance, thisPtr)
                .As<ISpatialCoordinateSystem.Vftbl>());
        }

        public Matrix4x4? TryGetTransformTo(SpatialCoordinateSystem other) => _spatialCoordinateSystem.TryGetTransformTo(other._spatialCoordinateSystem);
    }

    namespace Preview
    {
        internal class ISpatialGraphInteropPreviewStatics
        {
            [Guid("c042644c-20d8-4ed0-aef7-6805b8e53f55")]
            public struct Vftbl
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

            public readonly WinRT.ObjectReference<Vftbl> _obj;

            public static implicit operator ISpatialGraphInteropPreviewStatics(WinRT.IObjectReference obj) => obj.As<Vftbl>();
            public static implicit operator ISpatialGraphInteropPreviewStatics(WinRT.ObjectReference<Vftbl> obj) => new ISpatialGraphInteropPreviewStatics(obj);
            public ISpatialGraphInteropPreviewStatics(WinRT.ObjectReference<Vftbl> obj) { _obj = obj; }

            public unsafe SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId)
            {
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.CreateCoordinateSystemForNode(_obj.ThisPtr, nodeId, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<ISpatialCoordinateSystem.Vftbl>.Attach(_obj.Module, ref instancePtr));
            }

            public unsafe SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position)
            {
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.CreateCoordinateSystemForNodeWithPosition(_obj.ThisPtr, nodeId, position, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<ISpatialCoordinateSystem.Vftbl>.Attach(_obj.Module, ref instancePtr));
            }

            public unsafe SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position, Quaternion orientation)
            {
                IntPtr instancePtr = IntPtr.Zero;
                Marshal.ThrowExceptionForHR(_obj.Vftbl.CreateCoordinateSystemForNodeWithPositionAndOrientation(_obj.ThisPtr, nodeId, position, orientation, &instancePtr));
                return new SpatialCoordinateSystem(WinRT.ObjectReference<ISpatialCoordinateSystem.Vftbl>.Attach(_obj.Module, ref instancePtr));
            }
        }

        public class SpatialGraphInteropPreview
        {
            internal class Statics : ISpatialGraphInteropPreviewStatics
            {
                public Statics() : base(WinRT.ActivationFactory<SpatialGraphInteropPreview>._As<ISpatialGraphInteropPreviewStatics.Vftbl>()) { }
            }

            static WinRT.WeakLazy<Statics> _statics = new WinRT.WeakLazy<Statics>();

            public static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId) => _statics.Value.CreateCoordinateSystemForNode(nodeId);

            public static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position) => _statics.Value.CreateCoordinateSystemForNode(nodeId, position);

            public static SpatialCoordinateSystem CreateCoordinateSystemForNode(Guid nodeId, Vector3 position, Quaternion orientation) => _statics.Value.CreateCoordinateSystemForNode(nodeId, position, orientation);
        }
    }
}