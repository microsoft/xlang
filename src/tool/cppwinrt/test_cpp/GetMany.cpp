#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

//
// Now that all of the generics are generated (rather than hand-written), it's far less likely
// that something like GetMany is incorrect. And the "FillArray" pattern used by GetMany is
// tested elsewhere in the out_params and return_params tests. However since C++/WinRT provides
// an implementation of GetMany over and above the projection, these tests validate the this
// implementation is correct. Other tests exist for collections under 'old_tests' but new
// optimizations are coming for GetMany and I want to make sure that GetMany is completely
// covered.
//

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

    // None
    {
        auto v = single_threaded_vector<int>({ 1,2,3 });
        std::array<int, 3> buffer{ 0xCC, 0xCC, 0xCC };
        REQUIRE(0 == v.GetMany(3, buffer));
        REQUIRE(buffer[0] == 0xCC);
        REQUIRE(buffer[1] == 0xCC);
        REQUIRE(buffer[2] == 0xCC);
    }
    {
        auto v = single_threaded_vector<int>({ 1,2,3,4 });
        auto pos = v.First();
        std::array<int, 3> buffer{ 0xCC, 0xCC, 0xCC };
        REQUIRE(3 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
        buffer = { 0xCC, 0xCC, 0xCC };
        REQUIRE(1 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == 4);
        REQUIRE(buffer[1] == 0xCC);
        REQUIRE(buffer[2] == 0xCC);
        buffer = { 0xCC, 0xCC, 0xCC };
        REQUIRE(0 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == 0xCC);
        REQUIRE(buffer[1] == 0xCC);
        REQUIRE(buffer[2] == 0xCC);
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

    // None
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3" });
        std::array<hstring, 3> buffer{ L"old", L"old", L"old" };
        REQUIRE(0 == v.GetMany(3, buffer));
        REQUIRE(buffer[0] == L"");
        REQUIRE(buffer[1] == L"");
        REQUIRE(buffer[2] == L"");
    }
    {
        auto v = single_threaded_vector<hstring>({ L"1",L"2",L"3",L"4" });
        auto pos = v.First();
        std::array<hstring, 3> buffer{ L"old", L"old", L"old" };
        REQUIRE(3 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == L"1");
        REQUIRE(buffer[1] == L"2");
        REQUIRE(buffer[2] == L"3");
        buffer = { L"old", L"old", L"old" };
        REQUIRE(1 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == L"4");
        REQUIRE(buffer[1] == L"");
        REQUIRE(buffer[2] == L"");
        buffer = { L"old", L"old", L"old" };
        REQUIRE(0 == pos.GetMany(buffer));
        REQUIRE(buffer[0] == L"");
        REQUIRE(buffer[1] == L"");
        REQUIRE(buffer[2] == L"");
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
