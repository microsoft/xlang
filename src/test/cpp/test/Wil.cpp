#include "pch.h"
#include "winrt/Windows.Foundation.h"

using namespace winrt;
using namespace Windows::Foundation;

namespace
{
    struct WilException
    {
    };

    struct Sample : implements<Sample, IStringable>
    {
        hstring ToString()
        {
            throw WilException();
        }
    };

    int32_t WINRT_CALL handler(void* address) noexcept
    {
        REQUIRE(address);

        try
        {
            throw;
        }
        catch (WilException)
        {
            return 0x80000018; // E_ILLEGAL_DELEGATE_ASSIGNMENT
        }

        REQUIRE(false);
        return 0;
    }
}

TEST_CASE("Wil")
{
    REQUIRE(!winrt_to_hresult_handler);
    winrt_to_hresult_handler = handler;

    REQUIRE_THROWS_AS(make<Sample>().ToString(), hresult_illegal_delegate_assignment);

    winrt_to_hresult_handler = nullptr;
}
