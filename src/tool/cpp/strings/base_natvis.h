
#ifdef _DEBUG
#define WINRT_NATVIS
#endif

#ifdef WINRT_NATVIS

namespace winrt::impl
{
    struct natvis
    {
        static auto WINRT_CALL abi_val(void* object, wchar_t const * iid_str, int method)
        {
            union variant
            {
                bool b;
                wchar_t c;
                int8_t i1;
                int16_t i2;
                int32_t i4;
                int64_t i8;
                uint8_t u1;
                uint16_t u2;
                uint32_t u4;
                uint64_t u8;
                float r4;
                double r8;
                guid g;
                void* s;
                uint8_t v[1024];
            }
            value{};
            guid iid{};
            if (WINRT_IIDFromString(iid_str, &iid) == error_ok)
            {
                IInspectable* pinsp;
                typedef int32_t(WINRT_CALL IInspectable::* PropertyAccessor)(void*);
                if (((IUnknown*)object)->QueryInterface(iid, reinterpret_cast<void**>(&pinsp)) == error_ok)
                {
                    auto vtbl = *(PropertyAccessor**)pinsp;
                    static const int IInspectable_vtbl_size = 6;
                    auto get_Property = vtbl[method + IInspectable_vtbl_size];
                    (pinsp->*get_Property)(&value);
                    pinsp->Release();
                }
            }
            return value;
        }

        static auto WINRT_CALL get_val(winrt::Windows::Foundation::IInspectable* object, wchar_t const * iid_str, int method)
        {
            return abi_val(static_cast<IUnknown*>(get_abi(*object)), iid_str, method);
        }
    };
}

extern "C"
__declspec(selectany)
decltype(winrt::impl::natvis::abi_val) & WINRT_abi_val = winrt::impl::natvis::abi_val;

extern "C"
__declspec(selectany)
decltype(winrt::impl::natvis::get_val) & WINRT_get_val = winrt::impl::natvis::get_val;

#ifdef _M_IX86
#pragma comment(linker, "/include:_WINRT_abi_val")
#pragma comment(linker, "/include:_WINRT_get_val")
#else
#pragma comment(linker, "/include:WINRT_abi_val")
#pragma comment(linker, "/include:WINRT_get_val")
#endif

#endif
