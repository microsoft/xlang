#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace std::literals;

namespace
{
    constexpr uint32_t to_uint(char const value) noexcept
    {
        if (value >= '0' && value <= '9')
        {
            return value - '0';
        }

        if (value >= 'A' && value <= 'F')
        {
            return 10 + value - 'A';
        }

        if (value >= 'a' && value <= 'f')
        {
            return 10 + value - 'a';
        }

        std::terminate();
    }

    constexpr guid make_guid(std::string_view const& value) noexcept
    {
        if (value.size() != 36 || value[8] != '-' || value[13] != '-' || value[18] != '-' || value[23] != '-')
        {
            std::terminate();
        }

        return
        {
            ((to_uint(value[0]) * 16 + to_uint(value[1])) << 24) +
            ((to_uint(value[2]) * 16 + to_uint(value[3])) << 16) +
            ((to_uint(value[4]) * 16 + to_uint(value[5])) << 8) +
             (to_uint(value[6]) * 16 + to_uint(value[7])),

            static_cast<uint16_t>(((to_uint(value[9]) * 16 + to_uint(value[10])) << 8) +
                (to_uint(value[11]) * 16 + to_uint(value[12]))),

            static_cast<uint16_t>(((to_uint(value[14]) * 16 + to_uint(value[15])) << 8) +
                (to_uint(value[16]) * 16 + to_uint(value[17]))),

            {
                static_cast<uint8_t>(to_uint(value[19]) * 16 + to_uint(value[20])),
                static_cast<uint8_t>(to_uint(value[21]) * 16 + to_uint(value[22])),

                static_cast<uint8_t>(to_uint(value[24]) * 16 + to_uint(value[25])),
                static_cast<uint8_t>(to_uint(value[26]) * 16 + to_uint(value[27])),
                static_cast<uint8_t>(to_uint(value[28]) * 16 + to_uint(value[29])),
                static_cast<uint8_t>(to_uint(value[30]) * 16 + to_uint(value[31])),
                static_cast<uint8_t>(to_uint(value[32]) * 16 + to_uint(value[33])),
                static_cast<uint8_t>(to_uint(value[34]) * 16 + to_uint(value[35])),
            }
        };
    }

    constexpr bool equal(guid const& left, guid const& right) noexcept
    {
        return left.Data1 == right.Data1 &&
            left.Data2 == right.Data2 &&
            left.Data3 == right.Data3 &&
            left.Data4[0] == right.Data4[0] &&
            left.Data4[1] == right.Data4[1] &&
            left.Data4[2] == right.Data4[2] &&
            left.Data4[3] == right.Data4[3] &&
            left.Data4[4] == right.Data4[4] &&
            left.Data4[5] == right.Data4[5] &&
            left.Data4[6] == right.Data4[6] &&
            left.Data4[7] == right.Data4[7];
    }
}

#define REQUIRE_EQUAL_GUID(left, ...) STATIC_REQUIRE(equal(make_guid(left), guid_of<__VA_ARGS__>()));
#define REQUIRE_EQUAL_NAME(left, ...) STATIC_REQUIRE(left == name_of<__VA_ARGS__>());

TEST_CASE("generic_types")
{
    using A = IIterable<IStringable>;
    using B = IKeyValuePair<hstring, IAsyncOperationWithProgress<A, float>>;

    REQUIRE_EQUAL_GUID("96369F54-8EB6-48F0-ABCE-C1B211E627C3"sv, IStringable);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IStringable", IStringable);

    //
    // Generated Windows.Foundation GUIDs
    //

    REQUIRE_EQUAL_GUID("DD725452-2DA3-5103-9C7D-22EE9BB14AD3", IAsyncActionWithProgress<A>);
    REQUIRE_EQUAL_GUID("94645425-B9E5-5B91-B509-8DA4DF6A8916", IAsyncOperationWithProgress<A, B>);
    REQUIRE_EQUAL_GUID("2BD35EE6-72D9-5C5D-9827-05EBB81487AB", IAsyncOperation<A>);
    REQUIRE_EQUAL_GUID("4A33FE03-E8B9-5346-A124-5449913ECA57", IReferenceArray<A>);
    REQUIRE_EQUAL_GUID("F9E4006C-6E8C-56DF-811C-61F9990EBFB0", IReference<A>);
    REQUIRE_EQUAL_GUID("C261D8D0-71BA-5F38-A239-872342253A18", AsyncActionProgressHandler<A>);
    REQUIRE_EQUAL_GUID("9A0D211C-0374-5D23-9E15-EAA3570FAE63", AsyncActionWithProgressCompletedHandler<A>);
    REQUIRE_EQUAL_GUID("9D534225-231F-55E7-A6D0-6C938E2D9160", AsyncOperationCompletedHandler<A>);
    REQUIRE_EQUAL_GUID("264F1E0C-ABE4-590B-9D37-E1CC118ECC75", AsyncOperationProgressHandler<A, B>);
    REQUIRE_EQUAL_GUID("C2D078D8-AC47-55AB-83E8-123B2BE5BC5A", AsyncOperationWithProgressCompletedHandler<A, B>);
    REQUIRE_EQUAL_GUID("FA0B7D80-7EFA-52DF-9B69-0574CE57ADA4", EventHandler<A>);
    REQUIRE_EQUAL_GUID("EDB31843-B4CF-56EB-925A-D4D0CE97A08D", TypedEventHandler<A, B>);

    //
    // Generated Windows.Foundation.Collections GUIDs
    //

    REQUIRE_EQUAL_GUID("96565EB9-A692-59C8-BCB5-647CDE4E6C4D", IIterable<A>);
    REQUIRE_EQUAL_GUID("3C9B1E27-8357-590B-8828-6E917F172390", IIterator<A>);
    REQUIRE_EQUAL_GUID("89336CD9-8B66-50A7-9759-EB88CCB2E1FE", IKeyValuePair<A, B>);
    REQUIRE_EQUAL_GUID("E1AA5138-12BD-51A1-8558-698DFD070ABE", IMapChangedEventArgs<A>);
    REQUIRE_EQUAL_GUID("B78F0653-FA89-59CF-BA95-726938AAE666", IMapView<A, B>);
    REQUIRE_EQUAL_GUID("9962CD50-09D5-5C46-B1E1-3C679C1C8FAE", IMap<A, B>);
    REQUIRE_EQUAL_GUID("75F99E2A-137E-537E-A5B1-0B5A6245FC02", IObservableMap<A, B>);
    REQUIRE_EQUAL_GUID("D24C289F-2341-5128-AAA1-292DD0DC1950", IObservableVector<A>);
    REQUIRE_EQUAL_GUID("5F07498B-8E14-556E-9D2E-2E98D5615DA9", IVectorView<A>);
    REQUIRE_EQUAL_GUID("0E3F106F-A266-50A1-8043-C90FCF3844F6", IVector<A>);
    REQUIRE_EQUAL_GUID("19046F0B-CF81-5DEC-BBB2-7CC250DA8B8B", MapChangedEventHandler<A, B>);
    REQUIRE_EQUAL_GUID("A1E9ACD7-E4DF-5A79-AEFA-DE07934AB0FB", VectorChangedEventHandler<A>);

    //
    // Generated primitive GUIDs
    //

    REQUIRE_EQUAL_GUID("3C00FD60-2950-5939-A21A-2D12C5A01B8A", IReference<bool>);
    REQUIRE_EQUAL_GUID("95500129-FBF6-5AFC-89DF-70642D741990", IReference<int8_t>);
    REQUIRE_EQUAL_GUID("6EC9E41B-6709-5647-9918-A1270110FC4E", IReference<int16_t>);
    REQUIRE_EQUAL_GUID("548CEFBD-BC8A-5FA0-8DF2-957440FC8BF4", IReference<int32_t>);
    REQUIRE_EQUAL_GUID("4DDA9E24-E69F-5C6A-A0A6-93427365AF2A", IReference<int64_t>);
    REQUIRE_EQUAL_GUID("e5198cc8-2873-55f5-b0a1-84ff9e4aad62", IReference<uint8_t>);
    REQUIRE_EQUAL_GUID("5AB7D2C3-6B62-5E71-A4B6-2D49C4F238FD", IReference<uint16_t>);
    REQUIRE_EQUAL_GUID("513ef3af-e784-5325-a91e-97c2b8111cf3", IReference<uint32_t>);
    REQUIRE_EQUAL_GUID("6755e376-53bb-568b-a11d-17239868309e", IReference<uint64_t>);
    REQUIRE_EQUAL_GUID("719CC2BA-3E76-5DEF-9F1A-38D85A145EA8", IReference<float>);
    REQUIRE_EQUAL_GUID("2F2D6C29-5473-5F3E-92E7-96572BB990E2", IReference<double>);
    REQUIRE_EQUAL_GUID("FB393EF3-BBAC-5BD5-9144-84F23576F415", IReference<char16_t>);
    REQUIRE_EQUAL_GUID("7D50F649-632C-51F9-849A-EE49428933EA", IReference<guid>);
    REQUIRE_EQUAL_GUID("6FF27A1E-4B6A-59B7-B2C3-D1F2EE474593", IReference<hresult>);
    REQUIRE_EQUAL_GUID("FD416DFB-2A07-52EB-AAE3-DFCE14116C05", IReference<hstring>);
    REQUIRE_EQUAL_GUID("A9B18291-CE2A-5DAE-8A23-B7F7388416DB", IReference<event_token>);

    // Enums, structs, IInspectable, classes, and delegates

    REQUIRE_EQUAL_GUID("ECEBDE54-FAC0-5AEB-9BA9-9E1FE17E31D5", IReference<PropertyType>);
    REQUIRE_EQUAL_GUID("84F14C22-A00A-5272-8D3D-82112E66DF00", IReference<Point>);
    REQUIRE_EQUAL_GUID("B32BDCA4-5E52-5B27-BC5D-D66A1A268C2A", IVector<IInspectable>);
    REQUIRE_EQUAL_GUID("0D82BD8D-FE62-5D67-A7B9-7886DD75BC4E", IVector<Uri>);
    REQUIRE_EQUAL_GUID("5DAFE591-86DC-59AA-BFDA-07F5D59FC708", IVector<AsyncActionCompletedHandler>);

    // TODO: add numerics, chrono, rect, size, point, and anything else that is hardcoded

    //
    // Generated Windows.Foundation names
    //

    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IAsyncActionWithProgress`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IAsyncActionWithProgress<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        IAsyncOperationWithProgress<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IAsyncOperation`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IAsyncOperation<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReferenceArray`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IReferenceArray<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IReference<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.AsyncActionProgressHandler`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        AsyncActionProgressHandler<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.AsyncActionWithProgressCompletedHandler`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        AsyncActionWithProgressCompletedHandler<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.AsyncOperationCompletedHandler`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        AsyncOperationCompletedHandler<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.AsyncOperationProgressHandler`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        AsyncOperationProgressHandler<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.AsyncOperationWithProgressCompletedHandler`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        AsyncOperationWithProgressCompletedHandler<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.EventHandler`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        EventHandler<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.TypedEventHandler`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        TypedEventHandler<A, B>);

    //
    // Generated Windows.Foundation.Collections names
    //

    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IIterable<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IIterator`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IIterator<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IKeyValuePair`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        IKeyValuePair<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IMapChangedEventArgs`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IMapChangedEventArgs<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IMapView`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        IMapView<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IMap`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        IMap<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IObservableMap`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        IObservableMap<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IObservableVector`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IObservableVector<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IVectorView`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IVectorView<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IVector`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        IVector<A>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.MapChangedEventHandler`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Windows.Foundation.Collections.IKeyValuePair`2<String, Windows.Foundation.IAsyncOperationWithProgress`2<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>, Single>>>",
        MapChangedEventHandler<A, B>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.VectorChangedEventHandler`1<Windows.Foundation.Collections.IIterable`1<Windows.Foundation.IStringable>>",
        VectorChangedEventHandler<A>);

    //
    // Generated primitive names
    //

    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Boolean>",
        IReference<bool>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Int8>",
        IReference<int8_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Int16>",
        IReference<int16_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Int32>",
        IReference<int32_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Int64>",
        IReference<int64_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<UInt8>",
        IReference<uint8_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<UInt16>",
        IReference<uint16_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<UInt32>",
        IReference<uint32_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<UInt64>",
        IReference<uint64_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Single>",
        IReference<float>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Double>",
        IReference<double>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Char16>",
        IReference<char16_t>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Guid>",
        IReference<guid>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Windows.Foundation.HResult>",
        IReference<hresult>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<String>",
        IReference<hstring>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Windows.Foundation.EventRegistrationToken>",
        IReference<event_token>);

    // Enums, structs, IInspectable, classes, and delegates

    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Windows.Foundation.PropertyType>",
        IReference<PropertyType>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.IReference`1<Windows.Foundation.Point>",
        IReference<Point>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IVector`1<Object>",
        IVector<IInspectable>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IVector`1<Windows.Foundation.Uri>",
        IVector<Uri>);
    REQUIRE_EQUAL_NAME(L"Windows.Foundation.Collections.IVector`1<Windows.Foundation.AsyncActionCompletedHandler>",
        IVector<AsyncActionCompletedHandler>);
}
