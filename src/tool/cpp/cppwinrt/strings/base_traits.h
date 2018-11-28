
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

    template <> struct category<Windows::Foundation::IUnknown>
    {
        using type = interface_category;
    };

    template <> struct category<Windows::Foundation::IInspectable>
    {
        using type = basic_category;
    };
}
