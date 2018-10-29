#include "pch.h"
#include "winrt/Component.h"

using namespace winrt;
using namespace Component;

TEST_CASE("Enum")
{
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<Signed>, int32_t>);
    STATIC_REQUIRE(std::is_same_v<std::underlying_type_t<Unsigned>, uint32_t>);

    STATIC_REQUIRE(static_cast<int32_t>(Signed::First) == -1);
    STATIC_REQUIRE(static_cast<int32_t>(Signed::Second) == 0);
    STATIC_REQUIRE(static_cast<int32_t>(Signed::Third) == 1);

    STATIC_REQUIRE(static_cast<uint32_t>(Unsigned::First) == 0);
    STATIC_REQUIRE(static_cast<uint32_t>(Unsigned::Second) == 1);
    STATIC_REQUIRE(static_cast<uint32_t>(Unsigned::Third) == 2);
}
