#include "pch.h"

using namespace xlang;

TEST_CASE("IXlangObject")
{
    struct Object : implements<Object, Windows::Foundation::IXlangObject>
    {
        
    };

    auto obj = make<Object>();
}
