#include "pch.h"

// TODO: enable xplat activation test
#if XLANG_PLATFORM_WINDOWS
#include "winrt_helpers.h"

TEST_CASE("Simple activation")
{
    xlang_error_info* result{};

    // TODO: Update cpp projection to emit char16_t names
    std::u16string_view class_name{ u"Component.Edge.ZeroClass" };
    xlang_string_header str_header{};
    xlang_string str{};

    result = xlang_create_string_reference_utf16(class_name.data(), static_cast<uint32_t>(class_name.size()), &str_header, &str);
    REQUIRE(result == nullptr);

    winrt::Windows::Foundation::IUnknown factory{};
    xlang_guid iid = to_guid(winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>());
    result = xlang_get_activation_factory(str, iid, winrt::put_abi(factory));
    REQUIRE(result == nullptr);

    // TODO: Actually call an activation method once we've converged our type system.
    // For now, simply check that we got a valid object back.
    REQUIRE(factory != nullptr);
}

#endif