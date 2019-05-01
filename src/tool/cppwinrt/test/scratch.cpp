#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;

namespace
{
    struct Stringable : implements<Stringable, IStringable, IClosable>
    {
        hstring ToString() { Test(); return L""; }
        void Close() {}

        void Call(IInspectable const&)
        {
            puts("IInspectable");
        }

        //void Call(IStringable const&)
        //{
        //    puts("IStringable");
        //}

        //void Call(IClosable const&)
        //{
        //    puts("IClosable");
        //}

        void Test()
        {
            Call(*this);
        }
    };
}

TEST_CASE("scratch")
{
    auto s = make<Stringable>();
    s.ToString();
}
