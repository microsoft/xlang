#include "pch.h"
#include <winrt/Component.h>
#include "winrt_helpers.h"

TEST_CASE("Simple activation")
{
    xlang_result result{};
    // TODO: Update cpp projection to emit char16_t names
    std::u16string_view class_name{ u"Component.Class" };
    xlang_string_header str_header{};
    xlang_string str{};

    result = xlang_create_string_reference_utf16(class_name.data(), static_cast<uint32_t>(class_name.size()), &str_header, &str);
    REQUIRE(result == xlang_error_ok);

    winrt::Windows::Foundation::IUnknown factory{};
    xlang_guid iid = to_guid(winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>());
    result = xlang_get_activation_factory(str, iid, winrt::put_abi(factory));
    REQUIRE(result == xlang_error_ok);
    REQUIRE(factory != nullptr);
}
