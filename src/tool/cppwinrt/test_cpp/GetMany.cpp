#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

TEST_CASE("GetMany")
{
    // All
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 3> buffer{ 0xCC, 0xCC, 0xCC };
        REQUIRE(3 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
    }
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 3> buffer{ 0xCC, 0xCC, 0xCC };
        REQUIRE(3 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
    }

    // Less
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 2> buffer{ 0xCC, 0xCC };
        REQUIRE(2 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
    }
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 2> buffer{ 0xCC, 0xCC };
        REQUIRE(2 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
    }

    // More
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 4> buffer{ 0xCC, 0xCC, 0xCC, 0xCC };
        REQUIRE(3 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
        REQUIRE(buffer[3] == 0xCC);
    }
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 4> buffer{ 0xCC, 0xCC, 0xCC, 0xCC };
        REQUIRE(3 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
        REQUIRE(buffer[3] == 0xCC);
    }

    // Offset
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 4> buffer{ 0xCC, 0xCC, 0xCC, 0xCC };
        REQUIRE(2 == v.GetMany(1, buffer));
        REQUIRE(buffer[0] == 2);
        REQUIRE(buffer[1] == 3);
        REQUIRE(buffer[2] == 0xCC);
        REQUIRE(buffer[3] == 0xCC);
    }

    // The same tests but with a non-trivially destructible type...

    // All
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 3> buffer{ L"old", L"old", L"old" };
        REQUIRE(3 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
        REQUIRE(buffer[2] == L"3");
    }
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 3> buffer{ L"old", L"old", L"old" };
        REQUIRE(3 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
        REQUIRE(buffer[2] == L"3");
    }

    // Less
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 2> buffer{ L"old", L"old" };
        REQUIRE(2 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
    }
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 2> buffer{ L"old", L"old" };
        REQUIRE(2 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
    }

    // More
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 4> buffer{ L"old", L"old", L"old", L"old" };
        REQUIRE(3 == v.GetMany(0, buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
        REQUIRE(buffer[2] == L"3");
        REQUIRE(buffer[3] == L"");
    }
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 4> buffer{ L"old", L"old", L"old", L"old" };
        REQUIRE(3 == v.First().GetMany(buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
        REQUIRE(buffer[2] == L"3");
        REQUIRE(buffer[3] == L"");
    }

    // Offset
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 4> buffer{ L"old", L"old", L"old", L"old" };
        REQUIRE(2 == v.GetMany(1, buffer));
        REQUIRE(buffer[0] == L"2");
        REQUIRE(buffer[1] == L"3");
        REQUIRE(buffer[2] == L"");
        REQUIRE(buffer[3] == L"");
    }
}
