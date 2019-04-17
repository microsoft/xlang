#pragma once
#include "Class.g.h"

namespace winrt::test_component::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;
        Class(hstring const&) {}

        void Fail(bool fail)
        {
            m_fail = fail;
        }

        void abi_enter()
        {
            if (m_fail)
            {
                throw hresult_invalid_argument(L"value");
            }
        }

        hstring InInt32(int32_t value);
        hstring InString(hstring const& value);
        hstring InObject(Windows::Foundation::IInspectable const& value);
        hstring InStringable(Windows::Foundation::IStringable const& value);
        hstring InStruct(Struct const& value);
        hstring InStructRef(Struct const& value);
        hstring InEnum(Signed const& value);

        void OutInt32(int32_t& value);
        void OutString(hstring& value);
        void OutObject(Windows::Foundation::IInspectable& value);
        void OutStringable(Windows::Foundation::IStringable& value);
        void OutStruct(Struct& value);
        void OutEnum(Signed& value);

        int32_t ReturnInt32();
        hstring ReturnString();
        Windows::Foundation::IInspectable ReturnObject();
        Windows::Foundation::IStringable ReturnStringable();
        Struct ReturnStruct();
        Signed ReturnEnum();

        hstring InInt32Array(array_view<int32_t const> value);
        hstring InStringArray(array_view<hstring const> value);
        hstring InObjectArray(array_view<Windows::Foundation::IInspectable const> value);
        hstring InStringableArray(array_view<Windows::Foundation::IStringable const> value);
        hstring InStructArray(array_view<Struct const> value);
        hstring InEnumArray(array_view<Signed const> value);

        void OutInt32Array(com_array<int32_t>& value);
        void OutStringArray(com_array<hstring>& value);
        void OutObjectArray(com_array<Windows::Foundation::IInspectable>& value);
        void OutStringableArray(com_array<Windows::Foundation::IStringable>& value);
        void OutStructArray(com_array<Struct>& value);
        void OutEnumArray(com_array<Signed>& value);

        void RefInt32Array(array_view<int32_t> value);
        void RefStringArray(array_view<hstring> value);
        void RefObjectArray(array_view<Windows::Foundation::IInspectable> value);
        void RefStringableArray(array_view<Windows::Foundation::IStringable> value);
        void RefStructArray(array_view<Struct> value);
        void RefEnumArray(array_view<Signed> value);

        com_array<int32_t> ReturnInt32Array();
        com_array<hstring> ReturnStringArray();
        com_array<Windows::Foundation::IInspectable> ReturnObjectArray();
        com_array<Windows::Foundation::IStringable> ReturnStringableArray();
        com_array<Struct> ReturnStructArray();
        com_array<Signed> ReturnEnumArray();

        void NoexceptVoid() noexcept;
        int32_t NoexceptInt32() noexcept;
        hstring NoexceptString() noexcept;

    private:

        bool m_fail{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
