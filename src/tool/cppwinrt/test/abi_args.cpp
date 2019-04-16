#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;

TEST_CASE("abi_args")
{
    // in: array
    {
        auto a = single_threaded_vector<int>();
        a.ReplaceAll({ 1,2 });
        REQUIRE(a.Size() == 2);
        REQUIRE(a.GetAt(0) == 1);
        REQUIRE(a.GetAt(1) == 2);
    }
    {
        auto a = single_threaded_vector<int>();
        a.ReplaceAll({ 1,2 });
        REQUIRE(a.Size() == 2);
        REQUIRE(a.GetAt(0) == 1);
        REQUIRE(a.GetAt(1) == 2);
    }

}
