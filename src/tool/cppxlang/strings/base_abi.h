
namespace winrt::impl
{
    inline constexpr hresult error_ok{ 0 }; // S_OK
    inline constexpr hresult error_fail{ static_cast<hresult>(0x80004005) }; // E_FAIL
    inline constexpr hresult error_access_denied{ static_cast<hresult>(0x80070005) }; // E_ACCESSDENIED
    inline constexpr hresult error_wrong_thread{ static_cast<hresult>(0x8001010E) }; // RPC_E_WRONG_THREAD
    inline constexpr hresult error_not_implemented{ static_cast<hresult>(0x80004001) }; // E_NOTIMPL
    inline constexpr hresult error_invalid_argument{ static_cast<hresult>(0x80070057) }; // E_INVALIDARG
    inline constexpr hresult error_out_of_bounds{ static_cast<hresult>(0x8000000B) }; // E_BOUNDS
    inline constexpr hresult error_no_interface{ static_cast<hresult>(0x80004002) }; // E_NOINTERFACE
    inline constexpr hresult error_class_not_available{ static_cast<hresult>(0x80040111) }; // CLASS_E_CLASSNOTAVAILABLE
    inline constexpr hresult error_changed_state{ static_cast<hresult>(0x8000000C) }; // E_CHANGED_STATE
    inline constexpr hresult error_illegal_method_call{ static_cast<hresult>(0x8000000E) }; // E_ILLEGAL_METHOD_CALL
    inline constexpr hresult error_illegal_state_change{ static_cast<hresult>(0x8000000D) }; // E_ILLEGAL_STATE_CHANGE
    inline constexpr hresult error_illegal_delegate_assignment{ static_cast<hresult>(0x80000018) }; // E_ILLEGAL_DELEGATE_ASSIGNMENT
    inline constexpr hresult error_canceled{ static_cast<hresult>(0x800704C7) }; // HRESULT_FROM_WIN32(ERROR_CANCELLED)
    inline constexpr hresult error_bad_alloc{ static_cast<hresult>(0x8007000E) }; // E_OUTOFMEMORY
    inline constexpr hresult error_not_initialized{ static_cast<hresult>(0x800401F0) }; // CO_E_NOTINITIALIZED

    template <> struct abi<Windows::Foundation::IUnknown>
    {
        struct WINRT_NOVTABLE type
        {
            virtual int32_t WINRT_CALL QueryInterface(guid const& id, void** object) noexcept = 0;
            virtual uint32_t WINRT_CALL AddRef() noexcept = 0;
            virtual uint32_t WINRT_CALL Release() noexcept = 0;
        };
    };
    template <> struct guid_storage<Windows::Foundation::IUnknown>
    {
        static constexpr guid value{ 0x00000000,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };
    using unknown_abi = abi_t<Windows::Foundation::IUnknown>;

    template <> struct abi<Windows::Foundation::IInspectable>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL GetIids(uint32_t* count, guid** ids) noexcept = 0;
            virtual int32_t WINRT_CALL GetRuntimeClassName(xlang_string* name) noexcept = 0;
            virtual int32_t WINRT_CALL GetTrustLevel(Windows::Foundation::TrustLevel* level) noexcept = 0;
        };
    };
    template <> struct guid_storage<Windows::Foundation::IInspectable>
    {
        static constexpr guid value{ 0xAF86E2E0,0xB12D,0x4C6A,{ 0x9C,0x5A,0xD7,0xAA,0x65,0x10,0x1E,0x90 } };
    };
    template <> struct name<Windows::Foundation::IInspectable>
    {
        static constexpr auto & value{ L"Object" };
        static constexpr auto & data{ "cinterface(IInspectable)" };
    };
    template <> struct category<Windows::Foundation::IInspectable>
    {
        using type = basic_category;
    };
    using inspectable_abi = abi_t<Windows::Foundation::IInspectable>;

    struct WINRT_NOVTABLE IAgileObject : unknown_abi
    {
    };
    template <> struct guid_storage<IAgileObject>
    {
        static constexpr guid value{ 0x94EA2B94,0xE9CC,0x49E0,{ 0xC0,0xFF,0xEE,0x64,0xCA,0x8F,0x5B,0x90 } };
    };

    struct WINRT_NOVTABLE IAgileReference : unknown_abi
    {
        virtual int32_t WINRT_CALL Resolve(guid const& id, void** object) noexcept = 0;
    };

    struct WINRT_NOVTABLE IStaticLifetime : inspectable_abi
    {
        virtual int32_t WINRT_CALL unused() noexcept = 0;
        virtual int32_t WINRT_CALL GetCollection(void** value) noexcept = 0;
    };
    template <> struct guid_storage<IStaticLifetime>
    {
        static constexpr guid value{ 0x17b0e613,0x942a,0x422d,{ 0x90,0x4c,0xf9,0x0d,0xc7,0x1a,0x7d,0xae } };
    };

    struct WINRT_NOVTABLE IStaticLifetimeCollection : inspectable_abi
    {
        virtual int32_t WINRT_CALL Lookup(void*, void**) noexcept = 0;
        virtual int32_t WINRT_CALL unused() noexcept = 0;
        virtual int32_t WINRT_CALL unused2() noexcept = 0;
        virtual int32_t WINRT_CALL unused3() noexcept = 0;
        virtual int32_t WINRT_CALL Insert(void*, void*, bool*) noexcept = 0;
        virtual int32_t WINRT_CALL unused4() noexcept = 0;
        virtual int32_t WINRT_CALL unused5() noexcept = 0;
    };
    template <> struct guid_storage<IStaticLifetimeCollection>
    {
        static constexpr guid value{ 0x1b0d3570,0x0877,0x5ec2,{ 0x8a,0x2c,0x3b,0x95,0x39,0x50,0x6a,0xca } };
    };

    struct WINRT_NOVTABLE IWeakReference : unknown_abi
    {
        virtual int32_t WINRT_CALL Resolve(guid const& iid, void** objectReference) noexcept = 0;
    };
    template <> struct guid_storage<IWeakReference>
    {
        static constexpr guid value{ 0x00000037,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    struct WINRT_NOVTABLE IWeakReferenceSource : unknown_abi
    {
        virtual int32_t WINRT_CALL GetWeakReference(IWeakReference** weakReference) noexcept = 0;
    };
    template <> struct guid_storage<IWeakReferenceSource>
    {
        static constexpr guid value{ 0x00000038,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    struct WINRT_NOVTABLE IRestrictedErrorInfo : unknown_abi
    {
        virtual int32_t WINRT_CALL GetErrorDetails(bstr* description, int32_t* error, bstr* restrictedDescription, bstr* capabilitySid) noexcept = 0;
        virtual int32_t WINRT_CALL GetReference(bstr* reference) noexcept = 0;
    };

    struct WINRT_NOVTABLE ILanguageExceptionErrorInfo2 : unknown_abi
    {
        virtual int32_t WINRT_CALL GetLanguageException(void** exception) noexcept = 0;
        virtual int32_t WINRT_CALL GetPreviousLanguageExceptionErrorInfo(ILanguageExceptionErrorInfo2** previous) noexcept = 0;
        virtual int32_t WINRT_CALL CapturePropagationContext(void* exception) noexcept = 0;
        virtual int32_t WINRT_CALL GetPropagationContextHead(ILanguageExceptionErrorInfo2** head) noexcept = 0;
    };
    template <> struct guid_storage<ILanguageExceptionErrorInfo2>
    {
        static constexpr guid value{ 0x5746E5C4,0x5B97,0x424C,{ 0xB6,0x20,0x28,0x22,0x91,0x57,0x34,0xDD } };
    };

    struct com_callback_args
    {
        uint32_t reserved1;
        uint32_t reserved2;
        void* data;
    };

    struct ICallbackWithNoReentrancyToApplicationSTA;
    template <> struct guid_storage<ICallbackWithNoReentrancyToApplicationSTA>
    {
        static constexpr guid value{ 0x0A299774,0x3E4E,0xFC42,{ 0x1D,0x9D,0x72,0xCE,0xE1,0x05,0xCA,0x57 } };
    };

    struct WINRT_NOVTABLE IContextCallback : unknown_abi
    {
        virtual int32_t WINRT_CALL ContextCallback(int32_t(WINRT_CALL *callback)(com_callback_args*), com_callback_args* args, guid const& iid, int method, void* reserved) noexcept = 0;
    };
    template <> struct guid_storage<IContextCallback>
    {
        static constexpr guid value{ 0x000001da,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    struct WINRT_NOVTABLE IServerSecurity : unknown_abi
    {
        virtual int32_t WINRT_CALL QueryBlanket(uint32_t*, uint32_t*, wchar_t**, uint32_t*, uint32_t*, void**, uint32_t*) noexcept = 0;
        virtual int32_t WINRT_CALL ImpersonateClient() noexcept = 0;
        virtual int32_t WINRT_CALL RevertToSelf() noexcept = 0;
        virtual int32_t WINRT_CALL IsImpersonating() noexcept = 0;
    };
    template <> struct guid_storage<IServerSecurity>
    {
        static constexpr guid value{ 0x0000013E,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    struct WINRT_NOVTABLE IBufferByteAccess : unknown_abi
    {
        virtual int32_t WINRT_CALL Buffer(uint8_t** value) noexcept = 0;
    };
    template <> struct guid_storage<IBufferByteAccess>
    {
        static constexpr guid value{ 0x905a0fef,0xbc53,0x11df,{ 0x8c,0x49,0x00,0x1e,0x4f,0xc6,0x86,0xda } };
    };
}
