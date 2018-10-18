#include <winrt/Component.h>
#include <pal.h>
#include "Class.h"

using namespace std::string_view_literals;

extern "C"
{
    xlang_result XLANG_CALL lib_get_activation_factory(xlang_string class_name, xlang_guid const& iid, void **factory) noexcept
    try
    {
        *factory = nullptr;

        char16_t const* buffer{};
        uint32_t length{};
        winrt::check_hresult(xlang_get_string_raw_buffer_utf16(class_name, &buffer, &length));
        std::u16string_view name{ buffer, length };
        
        if (name == u"Component.Class"sv)
        {
            // TODO: Unify xlang guid and winrt guid
            if (*reinterpret_cast<winrt::guid const*>(&iid) == winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
            {
                auto result = winrt::make_self<winrt::Component::factory_implementation::Class>();
                *factory = winrt::to_abi<winrt::Windows::Foundation::IActivationFactory>(result.detach());
                return xlang_error_ok;
            }
            else
            {
                return winrt::hresult_no_interface{}.to_abi();
            }
        }

        return winrt::hresult_class_not_available{}.to_abi();
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
}
