using System;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using Xunit;
using WinRT;

using WF = Windows.Foundation;
using WFC = Windows.Foundation.Collections;
using WFM = Windows.Foundation.Metadata;
using WFN = Windows.Foundation.Numerics;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Foundation.Metadata;
using Windows.Foundation.Numerics;

using TestComp;
//using TestComp.Manual;

#if false
using Windows.Foundation;
using Windows.Foundation.Collections;
//using Windows.Foundation.Metadata;

// piids needed for activation and casting (As<I>)
// delegates and interfaces (generic and non) have base guids
// pdelegates and pinterfaces calculate and cache piids from base guids
// iinspectable is special (cinterface)
// runtime classes are special (default_interface)
// delegates are special (delegate)
// for each of these, need a way to identify

namespace Windows.Foundation
{
    [Guid("30d5a829-7fa4-4026-83bb-d75bae4ea99e")]
    public class IClosable { }

    [Guid("96369f54-8eb6-48f0-abce-c1b211e627c3")]
    public class IStringable { }

    // does this project as 'object'?  generic constraint
    [Guid("af86e2e0-b12d-4c6a-9c5a-d7aa65101e90")]
    public class IInspectable { }

    [Guid("61c17706-2d65-11e0-9ae8-d48564015472")]
    public class IReference<T> { }

    [Guid("9de1c535-6ae1-11e0-84e1-18a905bcc53f")]
    public class EventHandler<T>
    { }

    [Guid("9de1c534-6ae1-11e0-84e1-18a905bcc53f")]
    public class TypedEventHandler<TSender, TResult>
    { }

    [Guid("b5d036d7-e297-498f-ba60-0289e76e23dd")]
    public class IAsyncOperationWithProgress<TResult, TProgress>
    { }

    // delegates
    public class Delegate
    { }

    [Guid("a4ed5c81-76c9-40bd-8be6-b1d90fb20ae7")]
    public class AsyncActionCompletedHandler : Delegate
    {
        public static Guid PIID = GuidGenerator.GetGuid(typeof(AsyncActionCompletedHandler));
    }

    // runtime classes
    public class RuntimeClass
    {
    }

    // structs
    struct Rect
    {
        public float X;
        public float Y;
        public float Width;
        public float Height;
    }

    struct Point
    {
        public float X;
        public float Y;
    }

    struct Size
    {
        public float Width;
        public float Height;
    }

    struct TimeSpan
    {
        public Int64 Duration;
    }
}

namespace Windows.Foundation.Collections
{
    public class StringMap : RuntimeClass
    {
        public static Type default_interface = typeof(IMap<string, string>);
    }

    [Guid("faa585ea-6214-4217-afda-7f46de5869b3")]
    public class IIterable<T>
    {
    }

    [Guid("02b51929-c1c4-4a7e-8940-0312b5c18500")]
    public class IKeyValuePair<K, V>
    {
    }

    [Guid("3c2925fe-8519-45c1-aa79-197b6718c1c1")]
    public class IMap<K, V>
    {
        public static Guid PIID = GuidGenerator.GetGuid(typeof(IMap<K, V>));
    }
}

namespace Windows.Foundation.Metadata
{
    [Flags]
    enum AttributeTargets : UInt32
    {
        All = 4294967295,
        Delegate = 1,
        Enum = 2,
        Event = 4,
        Field = 8,
        Interface = 16,
        Method = 64,
        Parameter = 128,
        Property = 256,
        RuntimeClass = 512,
        Struct = 1024,
        InterfaceImpl = 2048,
        ApiContract = 8192
    }

    enum ThreadingModel : Int32
    {
        STA = 1,
        MTA = 2,
        Both = 3,
        InvalidThreading = 0
    }
}
#endif

// todo ...
namespace Windows.Foundation.Numerics
{
    struct float2
    {
        float x, y;
    }

    struct float3
    {
        float x, y, z;
    }

    struct float4
    {
        float x, y, z, w;
    }

    struct float3x2
    {
        float m11, m12;
        float m21, m22;
        float m31, m32;
    }

    struct float4x4
    {
        float m11, m12, m13, m14;
        float m21, m22, m23, m24;
        float m31, m32, m33, m34;
        float m41, m42, m43, m44;
    }

    struct plane
    {
        float3 normal;
        float d;
    }

    struct quaternion
    {
        float x, y, z, w;
    }
}

namespace UnitTest
{
    using A = IIterable<IStringable>;
    using B = IKeyValuePair<string, IAsyncOperationWithProgress</*A*/IIterable<IStringable>, float>>;
    using IInspectable = ObjectReference<WinRT.Interop.IInspectableVftbl>; // todo

    public class TestGuids
    {
        private static void AssertGuid<T>(string expected)
        {
            var actual = GuidGenerator.CreateIID(typeof(T));
            Assert.Equal(actual, new Guid(expected));
        }

        [Fact]
        public void TestGenerics()
        {
            // Ensure every generic instance has a unique PIID
            Assert.NotEqual(IMap<bool, string>.Vftbl.PIID, IMap<string, bool>.Vftbl.PIID);

            AssertGuid<IStringable>("96369f54-8eb6-48f0-abce-c1b211e627c3"); 

            // Generated Windows.Foundation GUIDs
            AssertGuid<IAsyncActionWithProgress<A>>("dd725452-2da3-5103-9c7d-22ee9bb14ad3");
            AssertGuid<IAsyncOperationWithProgress<A, B>>("94645425-b9e5-5b91-b509-8da4df6a8916");
            AssertGuid<IAsyncOperation<A>>("2bd35ee6-72d9-5c5d-9827-05ebb81487ab");
            AssertGuid<IReferenceArray<A>>("4a33fe03-e8b9-5346-a124-5449913eca57");
            AssertGuid<IReference<A>>("f9e4006c-6e8c-56df-811c-61f9990ebfb0");
            AssertGuid<AsyncActionProgressHandler<A>>("c261d8d0-71ba-5f38-a239-872342253a18");
            AssertGuid<AsyncActionWithProgressCompletedHandler<A>>("9a0d211c-0374-5d23-9e15-eaa3570fae63");
            AssertGuid<AsyncOperationCompletedHandler<A>>("9d534225-231f-55e7-a6d0-6c938e2d9160");
            AssertGuid<AsyncOperationProgressHandler<A, B>>("264f1e0c-abe4-590b-9d37-e1cc118ecc75");
            AssertGuid<AsyncOperationWithProgressCompletedHandler<A, B>>("c2d078d8-ac47-55ab-83e8-123b2be5bc5a");
            AssertGuid<WF.EventHandler<A>>("fa0b7d80-7efa-52df-9b69-0574ce57ada4");
            AssertGuid<TypedEventHandler<A, B>>("edb31843-b4cf-56eb-925a-d4d0ce97a08d");

            // Generated Windows.Foundation.Collections GUIDs
            AssertGuid<IIterable<A>>("96565eb9-a692-59c8-bcb5-647cde4e6c4d");
            AssertGuid<IIterator<A>>("3c9b1e27-8357-590b-8828-6e917f172390");
            AssertGuid<IKeyValuePair<A, B>>("89336cd9-8b66-50a7-9759-eb88ccb2e1fe");
            AssertGuid<IMapChangedEventArgs<A>>("e1aa5138-12bd-51a1-8558-698dfd070abe");
            AssertGuid<IMapView<A, B>>("b78f0653-fa89-59cf-ba95-726938aae666");
            AssertGuid<IMap<A, B>>("9962cd50-09d5-5c46-b1e1-3c679c1c8fae");
            AssertGuid<IObservableMap<A, B>>("75f99e2a-137e-537e-a5b1-0b5a6245fc02");
            AssertGuid<IObservableVector<A>>("d24c289f-2341-5128-aaa1-292dd0dc1950");
            AssertGuid<IVectorView<A>>("5f07498b-8e14-556e-9d2e-2e98d5615da9");
            AssertGuid<IVector<A>>("0e3f106f-a266-50a1-8043-c90fcf3844f6");
            AssertGuid<MapChangedEventHandler<A, B>>("19046f0b-cf81-5dec-bbb2-7cc250da8b8b");
            AssertGuid<VectorChangedEventHandler<A>>("a1e9acd7-e4df-5a79-aefa-de07934ab0fb");

            // Generated primitive GUIDs
            AssertGuid<IReference<bool>>("3c00fd60-2950-5939-a21a-2d12c5a01b8a");
            AssertGuid<IReference<sbyte>>("95500129-fbf6-5afc-89df-70642d741990");
            AssertGuid<IReference<Int16>>("6ec9e41b-6709-5647-9918-a1270110fc4e");
            AssertGuid<IReference<Int32>>("548cefbd-bc8a-5fa0-8df2-957440fc8bf4");
            AssertGuid<IReference<Int64>>("4dda9e24-e69f-5c6a-a0a6-93427365af2a");
            AssertGuid<IReference<byte>>("e5198cc8-2873-55f5-b0a1-84ff9e4aad62");
            AssertGuid<IReference<UInt16>>("5ab7d2c3-6b62-5e71-a4b6-2d49c4f238fd");
            AssertGuid<IReference<UInt32>>("513ef3af-e784-5325-a91e-97c2b8111cf3");
            AssertGuid<IReference<UInt64>>("6755e376-53bb-568b-a11d-17239868309e");
            AssertGuid<IReference<float>>("719cc2ba-3e76-5def-9f1a-38d85a145ea8");
            AssertGuid<IReference<double>>("2f2d6c29-5473-5f3e-92e7-96572bb990e2");
            AssertGuid<IReference<char>>("fb393ef3-bbac-5bd5-9144-84f23576f415");
            AssertGuid<IReference<Guid>>("7d50f649-632c-51f9-849a-ee49428933ea");
            AssertGuid<IReference<HResult>>("6ff27a1e-4b6a-59b7-b2c3-d1f2ee474593");
            AssertGuid<IReference<string>>("fd416dfb-2a07-52eb-aae3-dfce14116c05");
            //AssertGuid<IReference<event_token>>("a9b18291-ce2a-5dae-8a23-b7f7388416db");
            AssertGuid<IReference<WF.TimeSpan>>("604d0c4c-91de-5c2a-935f-362f13eaf800");
            AssertGuid<IReference<WF.DateTime>>("5541d8a7-497c-5aa4-86fc-7713adbf2a2c");
            AssertGuid<IReference<Point>>("84f14c22-a00a-5272-8d3d-82112e66df00");
            AssertGuid<IReference<Rect>>("80423f11-054f-5eac-afd3-63b6ce15e77b");
            AssertGuid<IReference<Size>>("61723086-8e53-5276-9f36-2a4bb93e2b75");
            //AssertGuid<IReference<float2>>("48f6a69e-8465-57ae-9400-9764087f65ad");
            //AssertGuid<IReference<float3>>("1ee770ff-c954-59ca-a754-6199a9be282c");
            //AssertGuid<IReference<float4>>("a5e843c9-ed20-5339-8f8d-9fe404cf3654");
            //AssertGuid<IReference<float3x2>>("76358cfd-2cbd-525b-a49e-90ee18247b71");
            //AssertGuid<IReference<float4x4>>("dacbffdc-68ef-5fd0-b657-782d0ac9807e");
            //AssertGuid<IReference<quaternion>>("b27004bb-c014-5dce-9a21-799c5a3c1461");
            //AssertGuid<IReference<plane>>("46d542a1-52f7-58e7-acfc-9a6d364da022");

            // Enums, structs, IInspectable, classes, and delegates
            AssertGuid<IReference<PropertyType>>("ecebde54-fac0-5aeb-9ba9-9e1fe17e31d5");
            AssertGuid<IReference<Point>>("84f14c22-a00a-5272-8d3d-82112e66df00");
            AssertGuid<IVector<IInspectable>>("b32bdca4-5e52-5b27-bc5d-d66a1a268c2a");
            AssertGuid<IVector<WF.Uri>>("0d82bd8d-fe62-5d67-a7b9-7886dd75bc4e");
            AssertGuid<IVector<AsyncActionCompletedHandler>>("5dafe591-86dc-59aa-bfda-07f5d59fc708");
        }
    }

    public class TestCompTests
    {
        public Class TestObject { get; private set; }

        public TestCompTests()
        {
            TestObject = new Class();
        }

        [Fact]
        public void TestPrimitives()
        {
            var test_int = 21;
            TestObject.IntPropertyChanged += (Object sender, Int32 value) =>
            {
                var c = (Class)sender;
                Assert.Equal(value, test_int);
            };
            TestObject.IntProperty = test_int;
        }

        [Fact]
        public void TestStrings()
        {
            string test_string = "x";
            string test_string2 = "y";

            var href = new WinRT.HStringReference(test_string);

            // In hstring from managed->native implicitly creates hstring reference 
            TestObject.StringProperty = test_string;

            // Out hstring from native->managed only creates System.String on demand
            var sp = TestObject.StringProperty;
            Assert.Equal(sp, test_string);

            // Out hstring from managed->native always creates HString from System.String
            TestObject.SetString(() => test_string2);
            Assert.Equal(TestObject.StringProperty, test_string2);

            // In hstring from native->managed only creates System.String on demand
            TestObject.StringPropertyChanged += (Class sender, WinRT.HString value) => sender.StringProperty2 = value;
            TestObject.GetString();
            Assert.Equal(TestObject.StringProperty2, test_string2);
        }
    }
}
