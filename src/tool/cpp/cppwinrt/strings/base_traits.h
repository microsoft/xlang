
namespace winrt::impl
{
    template <> struct guid_storage<Windows::Foundation::IUnknown>
    {
        static constexpr guid value{ 0x00000000,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<Windows::Foundation::IInspectable>
    {
        static constexpr guid value{ 0xAF86E2E0,0xB12D,0x4C6A,{ 0x9C,0x5A,0xD7,0xAA,0x65,0x10,0x1E,0x90 } };
    };

    template <> struct guid_storage<IAgileObject>
    {
        static constexpr guid value{ 0x94EA2B94,0xE9CC,0x49E0,{ 0xC0,0xFF,0xEE,0x64,0xCA,0x8F,0x5B,0x90 } };
    };

    template <> struct guid_storage<IMarshal>
    {
        static constexpr guid value{ 0x00000003,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IStaticLifetime>
    {
        static constexpr guid value{ 0x17b0e613,0x942a,0x422d,{ 0x90,0x4c,0xf9,0x0d,0xc7,0x1a,0x7d,0xae } };
    };

    template <> struct guid_storage<IWeakReference>
    {
        static constexpr guid value{ 0x00000037,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IWeakReferenceSource>
    {
        static constexpr guid value{ 0x00000038,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<ILanguageExceptionErrorInfo2>
    {
        static constexpr guid value{ 0x5746E5C4,0x5B97,0x424C,{ 0xB6,0x20,0x28,0x22,0x91,0x57,0x34,0xDD } };
    };

    template <> struct guid_storage<IContextCallback>
    {
        static constexpr guid value{ 0x000001da,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IServerSecurity>
    {
        static constexpr guid value{ 0x0000013E,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<ICallbackWithNoReentrancyToApplicationSTA>
    {
        static constexpr guid value{ 0x0A299774,0x3E4E,0xFC42,{ 0x1D,0x9D,0x72,0xCE,0xE1,0x05,0xCA,0x57 } };
    };

    template <> struct guid_storage<IBufferByteAccess>
    {
        static constexpr guid value{ 0x905a0fef,0xbc53,0x11df,{ 0x8c,0x49,0x00,0x1e,0x4f,0xc6,0x86,0xda } };
    };

    template <typename T> struct guid_storage<Windows::Foundation::IReference<T>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IReference<T>>::value };
    };

    template <typename T> struct guid_storage<Windows::Foundation::IReferenceArray<T>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IReferenceArray<T>>::value };
    };

    template <typename T> struct consume<Windows::Foundation::IReference<T>>
    {
        template <typename D> using type = consume_IReference<D, T>;
    };

    template <typename T> struct consume<Windows::Foundation::IReferenceArray<T>>
    {
        template <typename D> using type = consume_IReferenceArray<D, T>;
    };

    template <> struct name<Windows::Foundation::IInspectable>
    {
        static constexpr auto & value{ L"Object" };
        static constexpr auto & data{ "cinterface(IInspectable)" };
    };

    template <> struct name<IAgileObject>
    {
        static constexpr auto & value{ L"IAgileObject" };
    };

    template <> struct name<IWeakReferenceSource>
    {
        static constexpr auto & value{ L"IWeakReferenceSource" };
    };

    template <typename T> struct name<Windows::Foundation::IReference<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IReference`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Windows::Foundation::IReferenceArray<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IReferenceArray`1<", name_v<T>, L">") };
    };



    template <> struct category<Windows::Foundation::IUnknown>
    {
        using type = interface_category;
    };

    template <> struct category<Windows::Foundation::IInspectable>
    {
        using type = basic_category;
    };

    template <typename T> struct category<Windows::Foundation::IReference<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17706, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };

    template <typename T> struct category<Windows::Foundation::IReferenceArray<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17707, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };
}
